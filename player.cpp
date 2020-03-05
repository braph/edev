#include "player.hpp"

#include "common.hpp"
#include "process.cpp"

#include <unistd.h>

#include <iostream> //XXX

Mpg123Player :: Mpg123Player()
: audio_system("jack,pulse,alsa,oss")
, failed(0)
, state(STOPPED)
, track_completed(false)
, channels(0)
, sample_rate(0)
, seconds_total(0)
, seconds_played(0)
, seconds_remaining(0)
, process(nullptr)
{
}

Mpg123Player :: ~Mpg123Player() {
  stop();
}

void Mpg123Player :: play(const std::string &file) {
  if (this->file != file) {
    this->file = file;
    sample_rate = 0;
    channels = 0;
  }
  play();
}

void Mpg123Player :: play() {
  state = LOADING;
  seconds_total = 0;
  seconds_played = 0;
  seconds_remaining = 0;
  work();
}

void Mpg123Player :: stop() {
  if (process) {
    process->stdin_pipe.close();
    process->get_exit_status();
    process = nullptr;
  }

  state = STOPPED;
  seconds_total = 0;
  seconds_played = 0;
  seconds_remaining = 0;
}

void Mpg123Player :: work() {
  if (state == STOPPED)
    return;

  // Start process if he died (or wasn't even started yet)
  if (!process || !process->running()) {
    process = std::unique_ptr<Process>(
        new Process([&](){
          execlp("mpg123", "mp123", "-o", audio_system.c_str(), "--fuzzy", "-R", NULL);
        },
        true));
    *process << "SILENCE\n";
  }

  read_output();

  if (state == LOADING)
    *process << "L " << file << "\n";

  if (!sample_rate)
    *process << "FORMAT\n";

  *process << "SAMPLE\n";

  char buffer[4192];
  ssize_t n = process->stderr_pipe.read(buffer, 4192);
  if (n > 0) ++failed;

  read_output();
}

void Mpg123Player :: read_output() {
  char buffer[4192];
  ssize_t len = process->stdout_pipe.read(buffer, sizeof(buffer));
  if (len <= 0)
    return;

  for (ssize_t i = 0; i < len; ++i)
    if (buffer[i] != '\n')
      stdout_buffer.push_back(buffer[i]);
    else {
      parse_line(stdout_buffer.c_str());
      stdout_buffer.clear();
    }
}

/* Parse Mpg123's output:
 *   @FORMAT 44100 2
 *   @E Unknown command or no arguments: foo
 */
void Mpg123Player :: parse_line(const char* line) {
  if (*line++ != '@')
    return;

  // Single char command
  if (line[0] && line[1] == ' ') {
    line += 2;
    switch (line[-2]) {
      case 'P': /* Playing State */
        state = static_cast<State>(std::atoi(line));
        if (state == STOPPED) {
          if (failed) // Try again if playback stopped because of failure
            state = LOADING;
          else
            track_completed = true;
        }
        else {
          track_completed = false;
          failed = 0;
        }
        break;
      case 'E': /* Error message */ ++failed; break;
      case 'I': /* Info */ break;
      case 'J': /* Jump */ break;
      case 'R': /* ???? */ break;
      case 'S': /* ???? */ break;
      case 'F': {
        float played, remaining;
        std::sscanf(line, "%*d %*d %f %f", /* &d, &d, */ &played, &remaining);
        seconds_played    = played;
        seconds_remaining = remaining;
        seconds_total     = seconds_played + seconds_remaining;
        break;
      }
      default:
        std::cerr << "parse_output(): " << &line[-2] << std::endl;
    }
  }
  // String command
  else {
    if (cstr_seek(&line, "SAMPLE ")) {
      if (sample_rate) {
        int samples_played, samples_total;
        std::sscanf(line, "%d %d", &samples_played, &samples_total);
        seconds_played = samples_played / sample_rate;
        seconds_total  = samples_total  / sample_rate;
      }
    }
    else if (cstr_seek(&line, "FORMAT ")) {
      std::sscanf(line, "%d %d", &sample_rate, &channels);
    }
    else if (cstr_seek(&line, "silence ")) {
    }
    else
      std::cerr << "parse_output(): " << line << std::endl;
  }
}

void Mpg123Player :: set_position(int seconds) {
  if (process && process->running())
    *process << "J " << seconds << "s\n";
}

void Mpg123Player :: seek_forward(int seconds) {
  if (process && process->running())
    *process << "J +" << seconds << "s\n";
}

void Mpg123Player :: seek_backward(int seconds) {
  if (process && process->running())
    *process << "J -" << seconds << "s\n";
}

void Mpg123Player :: pause() {
  if (process && process->running())
    if (state == PLAYING)
      *process << "P\n";
}

void Mpg123Player :: toggle() {
  if (process && process->running())
    *process << "P\n";
}

#ifdef USE_VOLUME
void Mpg123Player :: volume(int volume) {
  if (process && process->running())
    *process << "VOLUME " << volume << '\n';
}
#endif

#ifdef TEST_PLAYER
#include "test.hpp"

int main() {
  TEST_BEGIN();

  Mpg123Player player;
  assert (! player.isPlaying());
  assert (! player.isPaused());
  assert (  player.isStopped());
  assert (! player.position());
  assert (! player.length());
  assert (! player.percent());
  player.play("/home/braph/.cache/ektoplayer/aerodromme-crop-circle.mp3");
  sleep(3);
  player.work();
  assert (  player.isPlaying());
  assert (! player.isPaused());
  assert (! player.isStopped());
  assert (  player.position());
  assert (  player.length());
  assert (  player.percent());

  pause();

  TEST_END();
}
#endif
