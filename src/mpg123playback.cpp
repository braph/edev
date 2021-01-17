#include "mpg123playback.hpp"

#include "ektoplayer.hpp"

#include <lib/process.hpp>
#include <lib/string.hpp> // temp_sprintf
#include <lib/stringpack.hpp>

#include <fcntl.h>

#include <memory>
#include <cstdio>
#include <cinttypes>

Mpg123Playback :: Mpg123Playback()
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

void Mpg123Playback :: reset() noexcept {
  _state = STOPPED;
  _failed = 0;
  _channels = 0;
  _sample_rate = 0;
  _seconds_total = 0;
  _seconds_played = 0;
  _seconds_remaining = 0;
  _track_completed = false;
}

void Mpg123Playback :: play(std::string file) noexcept {
  _file = std::move(file);
  play();
}

void Mpg123Playback :: play() noexcept {
  reset();
  _state = LOADING;
  work();
}

void Mpg123Playback :: stop() noexcept {
  _process = nullptr;
  reset();
}

void Mpg123Playback :: work() noexcept {
  if (_state == STOPPED)
    return;

  // Start process if it died (or wasn't even started yet)
  if (!_process || !_process->running()) {
    _process = std::unique_ptr<Process>(
      new Process([](){ ::execlp("mpg123", "mpg123", "--fuzzy", "-R", NULL); }, true));

    int fd = _process->stdout_pipe.fd();
    ::fcntl(fd, F_SETFL, ::fcntl(fd, F_GETFL) | O_NONBLOCK);
    fd = _process->stderr_pipe.fd();
    ::fcntl(fd, F_SETFL, ::fcntl(fd, F_GETFL) | O_NONBLOCK);
  }

  read_stdout();

  if (_state == LOADING)
    *_process << "SILENCE\nL " << _file << '\n';

  if (! _sample_rate)
    *_process << "FORMAT\n";

  *_process << "SAMPLE\n";

  read_stdout();
  read_stderr();
}

void Mpg123Playback :: read_stderr() noexcept {
  char buffer[128];
  const ssize_t n = _process->stderr_pipe.read(buffer);
  if (n > 0)
    ++_failed;
}

void Mpg123Playback :: read_stdout() noexcept {
  char buffer[512];
  const ssize_t len = _process->stdout_pipe.read(buffer);

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
void Mpg123Playback :: parse_stdout_line(const char* line) noexcept {
  //log_write("PARSE: %s\n", line);
  char* rest = const_cast<char*>(std::strchr(line, ' '));
  if (! rest)
    return;

  using pack = StringPack::Raw;
  switch (pack(line, size_t(rest-line))) {
    case pack("@SAMPLE"): // 16063 21920052
      if (_sample_rate) {
        _seconds_played = std::strtoimax(rest, &rest, 10) / _sample_rate;
        _seconds_total  = std::strtoimax(rest, &rest, 10) / _sample_rate;
      }
      break;
    case pack("@FORMAT"): // 44100 2
      _sample_rate = std::strtoimax(rest, &rest, 10);
      _channels    = std::strtoimax(rest, &rest, 10);
      break;
    case pack("@F"): // 77 17466 2.01 456.25
      std::strtoimax(rest, &rest, 10);
      std::strtoimax(rest, &rest, 10);
      _seconds_played    = int(std::strtof(rest, &rest));
      _seconds_remaining = int(std::strtof(rest, &rest));
      _seconds_total     = _seconds_played + _seconds_remaining;
      break;
    case pack("@P"): // 0|1|2
      _state = static_cast<State>(std::atoi(rest));
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
    case pack("@E"): /* Error */ ++_failed;
    case pack("@I"): /* Info  */
    case pack("@J"): /* Jump  */
    case pack("@R"): /* ????  */
    case pack("@S"): /* ????  */
      break;
#ifndef NDEBUG
    default:
      log_write("mpg123: %s\n", line);
      break;
#endif
  }
}

void Mpg123Playback :: position(int seconds) noexcept {
  if (_process && _process->running())
    *_process << temp_sprintf<20>("J %ds\n", seconds);
}

void Mpg123Playback :: seek(int seconds) noexcept {
  if (_process && _process->running())
    *_process << temp_sprintf<20>("J %+ds\n", seconds);
}

void Mpg123Playback :: seek_forward(int seconds) noexcept {
  if (_process && _process->running())
    *_process << temp_sprintf<20>("J +%ds\n", seconds);
}

void Mpg123Playback :: seek_backward(int seconds) noexcept {
  if (_process && _process->running())
    *_process << temp_sprintf<20>("J -%ds\n", seconds);
}

void Mpg123Playback :: pause() noexcept {
  if (_process && _process->running())
    if (_state == PLAYING)
      *_process << "P\n";
}

void Mpg123Playback :: toggle() noexcept {
  if (_process && _process->running())
    *_process << "P\n";
}

void Mpg123Playback :: volume(int volume) noexcept {
  if (_process && _process->running())
    *_process << temp_sprintf<20>("VOLUME %d\n", volume);
}

#ifdef TEST_PLAYER
#include "test.hpp"

int main() {
  TEST_BEGIN();

  Mpg123Playback player;
  assert (! player.is_playing());
  assert (! player.is_paused());
  assert (  player.is_stopped());
  assert (! player.position());
  assert (! player.length());
  assert (! player.percent());
  player.play("/home/braph/.cache/ektoplayer/aerodromme-crop-circle.mp3");
  sleep(3);
  player.work();
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
