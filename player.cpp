#include "player.hpp"

#include "common.hpp"
#include "process.cpp"
#include "process_unix.cpp"

#include "unistd.h"

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
    process->close_stdin();
    process->get_exit_status();
    process = nullptr;
  }

  state = STOPPED;
}

void Mpg123Player :: work() {
  if (state == STOPPED)
    return;

  // Start process if he died (or wasn't even started yet)
  if (!process || !process->running()) {
    process = std::unique_ptr<Mpg123Process>(new Mpg123Process(
        "/bin/mpg123 -o pulse -R", "", // TODO: -o audio_system --fuzzy
        [&](const char* buffer, size_t n) { read_output(buffer, n); },
        [&](const char* buffer, size_t n) { failed++;               },
        true, 4096 /* Buffer size */));
    *process << "SILENCE\n";
  }

  if (state == LOADING)
    *process << "L " << file << "\n";

  if (!sample_rate)
    *process << "FORMAT\n";

  *process << "SAMPLE\n";
}

static std::string _buffer; // TODO
void Mpg123Player :: read_output(const char* __buffer, size_t len) {
  char* buffer = const_cast<char*>(__buffer); // We know we can change the buffer
  char* end;

  while ((end = static_cast<char*>(std::memchr(buffer, '\n', len)))) {
    *end = '\0';
    if (_buffer.size()) {
      _buffer.append(buffer);
      parse_line(_buffer.c_str());
      _buffer.clear();
    } else {
      parse_line(buffer);
      len -= (end-buffer)+1;
      buffer = end + 1;
    }
  }

  if (len)
    _buffer.append(buffer, len);

#if 0
  if (!_buffer.size())

  while (std::string::npos != (len = _buffer.find('\n'))) {
    _buffer[len] = '\0';
    parse_line(_buffer.c_str());
    _buffer.erase(0, len+1);
  }
#endif
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
    float played, remaining;
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
      case 'F': 
        std::sscanf(line, "%*d %*d %f %f", /* &d, &d, */ &played, &remaining);
        seconds_played    = played;
        seconds_remaining = remaining;
        seconds_total     = seconds_played + seconds_remaining;
        break;
      default:
        std::cerr << "parse_output(): " << &line[-2] << std::endl;
    }
  }
  // String command
  else {
    if (cstr_seek(&line, "SAMPLE ")) {
      if (sample_rate) {
        unsigned int samples_played, samples_total;
        std::sscanf(line, "%u %u", &samples_played, &samples_total);
        seconds_played = samples_played / sample_rate;
        seconds_total  = samples_total  / sample_rate;
      }
    }
    else if (cstr_seek(&line, "FORMAT ")) {
      std::sscanf(line, "%u %hhu", &sample_rate, &channels);
    }
    else if (cstr_seek(&line, "silence ")) {
    }
    else
      std::cerr << "parse_output(): " << line << std::endl;
  }
}

void Mpg123Player :: set_position(unsigned seconds) {
  if (process && process->running())
    *process << "J " << seconds << "s\n";
}

void Mpg123Player :: seek_forward(unsigned seconds) {
  if (process && process->running())
    *process << "J +" << seconds << "s\n";
}

void Mpg123Player :: seek_backward(unsigned seconds) {
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

#if USE_VOLUME
void Mpg123Player :: volume(int volume) {
  if (process && process->running())
    *process << "VOLUME " << volume << '\n';
}
#endif

#if TEST_PLAYER
#include "test.hpp"

int main() {
  TEST_BEGIN();

  Mpg123Player player;
  assert (! player.is_playing());
  assert (! player.is_paused());
  assert (  player.is_stopped());
  assert (! player.position());
  assert (! player.length());
  assert (! player.percent());
  player.play("/home/braph/.cache/ektoplayer/aerodromme-crop-circle.mp3");
  sleep(3);
  player.poll();
  assert (  player.is_playing());
  assert (! player.is_paused());
  assert (! player.is_stopped());
  assert (  player.position());
  assert (  player.length());
  assert (  player.percent());

  pause();

  TEST_END();
}
#endif
