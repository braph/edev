#include "player.hpp"

#include "common.hpp"
#include "process.cpp"

#include <memory>
#include <cstdio>

Mpg123Player :: Mpg123Player()
: _failed(0)
, _state(STOPPED)
, _track_completed(false)
, _channels(0)
, _sample_rate(0)
, _seconds_total(0)
, _seconds_played(0)
, _seconds_remaining(0)
, _process(nullptr)
{
}

Mpg123Player :: ~Mpg123Player() {
  stop();
}

void Mpg123Player :: reset() {
  _state = STOPPED;
  _channels = 0;
  _sample_rate = 0;
  _seconds_total = 0;
  _seconds_played = 0;
  _seconds_remaining = 0;
  _track_completed = false;
}

void Mpg123Player :: play(const std::string &file) {
  if (_file != file) {
    _file = file;
    _sample_rate = 0;
    _channels = 0;
  }
  play();
}

void Mpg123Player :: play() {
  reset();
  _state = LOADING;
  work();
}

void Mpg123Player :: stop() {
  if (_process) {
    _process->stdin_pipe.close();
    _process->get_exit_status();
    _process = nullptr;
  }
  reset();
}

void Mpg123Player :: work() {
  if (_state == STOPPED)
    return;

  // Start process if it died (or wasn't even started yet)
  if (!_process || !_process->running()) {
    _process = std::unique_ptr<Process>(
      new Process([&](){ execlp("mpg123", "mp123", "--fuzzy", "-R", NULL); },
      true));
    *_process << "SILENCE\n";
  }

  read_output();

  if (_state == LOADING)
    *_process << "L " << _file << "\n";

  if (!_sample_rate)
    *_process << "FORMAT\n";

  *_process << "SAMPLE\n";

  char buffer[1024];
  ssize_t n = _process->stderr_pipe.read(buffer, sizeof(buffer));
  if (n > 0) ++_failed;

  read_output();
}

void Mpg123Player :: read_output() {
  char buffer[4096];
  ssize_t len = _process->stdout_pipe.read(buffer, sizeof(buffer));

  for (ssize_t i = 0; i < len; ++i)
    if (buffer[i] != '\n')
      _stdout_buffer.push_back(buffer[i]);
    else {
      parse_line(_stdout_buffer.c_str());
      _stdout_buffer.clear();
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
        _state = static_cast<State>(std::atoi(line));
        if (_state == STOPPED) {
          if (_failed) // Try again if playback stopped because of failure
            _state = LOADING;
          else
            _track_completed = true;
        }
        else {
          _track_completed = false;
          _failed = 0;
        }
        break;
      case 'E': /* Error message */ ++_failed; break;
      case 'I': /* Info */ break;
      case 'J': /* Jump */ break;
      case 'R': /* ???? */ break;
      case 'S': /* ???? */ break;
      case 'F': {
        float played, remaining;
        std::sscanf(line, "%*d %*d %f %f", /* &d, &d, */ &played, &remaining);
        _seconds_played    = played;
        _seconds_remaining = remaining;
        _seconds_total     = _seconds_played + _seconds_remaining;
        break;
      }
      default:
        //std::cerr << "parse_output(): " << &line[-2] << std::endl;
        break;
    }
  }
  // String command
  else {
    if (cstr_seek(&line, "SAMPLE ")) {
      if (_sample_rate) {
        int samples_played, samples_total;
        std::sscanf(line, "%d %d", &samples_played, &samples_total);
        _seconds_played = samples_played / _sample_rate;
        _seconds_total  = samples_total  / _sample_rate;
      }
    }
    else if (cstr_seek(&line, "FORMAT ")) {
      std::sscanf(line, "%d %d", &_sample_rate, &_channels);
    }
    else if (cstr_seek(&line, "silence ")) {
    }
    else {
      //std::cerr << "parse_output(): " << line << std::endl;
    }
  }
}

void Mpg123Player :: position(int seconds) {
  if (_process && _process->running())
    *_process << "J " << seconds << "s\n";
}

void Mpg123Player :: seekForward(int seconds) {
  if (_process && _process->running())
    *_process << "J +" << seconds << "s\n";
}

void Mpg123Player :: seekBackward(int seconds) {
  if (_process && _process->running())
    *_process << "J -" << seconds << "s\n";
}

void Mpg123Player :: pause() {
  if (_process && _process->running())
    if (_state == PLAYING)
      *_process << "P\n";
}

void Mpg123Player :: toggle() {
  if (_process && _process->running())
    *_process << "P\n";
}

#ifdef USE_VOLUME
void Mpg123Player :: volume(int volume) {
  if (_process && _process->running())
    *_process << "VOLUME " << volume << '\n';
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
