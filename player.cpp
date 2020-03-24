#include "player.hpp"

#include "lib/sscan.hpp"
#include "lib/process.hpp"

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
  _stdout_buffer.reserve(256);
}

Mpg123Player :: ~Mpg123Player() {
  stop();
}

void Mpg123Player :: reset() noexcept {
  _state = STOPPED;
  _channels = 0;
  _sample_rate = 0;
  _seconds_total = 0;
  _seconds_played = 0;
  _seconds_remaining = 0;
  _track_completed = false;
}

void Mpg123Player :: play(const std::string &file) noexcept {
  if (_file != file) {
    _file = file;
    _sample_rate = 0;
    _channels = 0;
  }
  play();
}

void Mpg123Player :: play() noexcept {
  reset();
  _state = LOADING;
  work();
}

void Mpg123Player :: stop() noexcept {
  if (_process) {
    _process->stdin_pipe.close();
    _process->get_exit_status();
    _process = nullptr;
  }
  reset();
}

void Mpg123Player :: work() noexcept {
  if (_state == STOPPED)
    return;

  // Start process if it died (or wasn't even started yet)
  if (!_process || !_process->running()) {
    _process = std::unique_ptr<Process>(
      new Process([&](){ execlp("mpg123", "mp123", "--fuzzy", "-R", NULL); }, true));
    *_process << "SILENCE\n";
  }

  read_stdout();

  if (_state == LOADING)
    *_process << "L " << _file << '\n';

  if (! _sample_rate)
    *_process << "FORMAT\n";

  *_process << "SAMPLE\n";

  read_stdout();
  read_stderr();
}

void Mpg123Player :: read_stderr() noexcept {
  char buffer[128];
  ssize_t n = _process->stderr_pipe.read(buffer, sizeof(buffer));
  if (n > 0) ++_failed;
}

void Mpg123Player :: read_stdout() noexcept {
  char buffer[1024];
  ssize_t len = _process->stdout_pipe.read(buffer, sizeof(buffer));

  for (ssize_t i = 0; i < len; ++i)
    if (buffer[i] != '\n')
      _stdout_buffer.push_back(buffer[i]);
    else {
      parse_stdout_line(_stdout_buffer.c_str());
      _stdout_buffer.clear();
    }
}

/* Parse Mpg123's output:
 *   @FORMAT 44100 2
 *   @E Unknown command or no arguments: foo
 */
void Mpg123Player :: parse_stdout_line(const char* line) noexcept {
  SScan scan(line);
  if (! scan.read('@'))
    return;

  int i = 0; // XXX this silences warning

  // Single char command
  if (scan[0] && scan[1] == ' ') {
    scan.seek(2, false);
    switch (scan[-2]) {
      case 'P': /* Playing State: 0|1|2  */
        scan.strtoi(i);
        _state = static_cast<State>(i);
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
        // @F 77 17466 2.01 456.25
        float played, remaining;
        if (scan.strtoi(i).strtoi(i).strtof(played).strtof(remaining)) {
          _seconds_played    = played;
          _seconds_remaining = remaining;
          _seconds_total     = _seconds_played + _seconds_remaining;
        }
        break;
      }
      default:
        //std::cerr << "parse_output(): " << &scan[-2] << std::endl;
        break;
    }
  }
  // String command
  else {
    if (scan.read("SAMPLE ")) {
      if (_sample_rate) {
        int samples_played, samples_total;
        if (scan.strtoi(samples_played).strtoi(samples_total)) {
          _seconds_played = samples_played / _sample_rate;
          _seconds_total  = samples_total  / _sample_rate;
        }
      }
    }
    else if (scan.clearError().read("FORMAT ")) {
      scan.strtoi(_sample_rate).strtoi(_channels);
    }
    else {
      //std::cerr << "parse_output(): " << scan << std::endl;
    }
  }
}

void Mpg123Player :: position(int seconds) noexcept {
  if (_process && _process->running())
    *_process << "J " << seconds << "s\n";
}

void Mpg123Player :: seekForward(int seconds) noexcept {
  if (_process && _process->running())
    *_process << "J +" << seconds << "s\n";
}

void Mpg123Player :: seekBackward(int seconds) noexcept {
  if (_process && _process->running())
    *_process << "J -" << seconds << "s\n";
}

void Mpg123Player :: pause() noexcept {
  if (_process && _process->running())
    if (_state == PLAYING)
      *_process << "P\n";
}

void Mpg123Player :: toggle() noexcept {
  if (_process && _process->running())
    *_process << "P\n";
}

#ifdef USE_VOLUME
void Mpg123Player :: volume(int volume) noexcept {
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
