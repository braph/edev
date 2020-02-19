#include "player.hpp"

#include "common.hpp"

#include <iostream> //XXX
#include "unistd.h"

Mpg123Player :: Mpg123Player()
: audio_system("jack,pulse,alsa,oss")
, flags(FLAG_NEED_FORMAT)
, state(STATE_STOPPED)
, channels(0)
, sample_rate(0)
, seconds_total(0)
, seconds_played(0)
, seconds_remaining(0)
, mpgProc(NULL)
{
}

Mpg123Player :: ~Mpg123Player() {
#if 0
  proc.terminate();
  in.close();
  out.close();
  if (thr.joinable())
    thr.join();
#endif
}

void Mpg123Player :: play() {
  if (file.size())
    play(file);
}

void Mpg123Player :: play(const std::string &_file) {
  file = _file;

  if (mpgProc && (!mpgProc->proc.valid() || !mpgProc->proc.running())) {
    delete mpgProc;
    mpgProc = NULL;
  }

  if (!mpgProc) {
    mpgProc = new Mpg123Process(*this);
  }

  sample_rate = 0;
  flags = FLAG_NEED_FORMAT;
  *mpgProc << "L " << file << ENDL;
  *mpgProc << "SILENCE"    << ENDL;
}
//
void Mpg123Player :: stop() {
  if (mpgProc)
  *mpgProc << 'Q' << ENDL;
//    std::lock_guard<std::mutex> lock(in_mutex);
//    try { if (in) in->close(); in=NULL; } catch (...) {}
}

void Mpg123Player :: work() {
  try {
  if (mpgProc) {
    if (flags & FLAG_NEED_FORMAT) {
      flags &= ~FLAG_NEED_FORMAT;
      *mpgProc << "FORMAT" << ENDL;
    }
    *mpgProc << "SAMPLE" << ENDL;
  }
  } catch (...) {
    std::cerr << "sdfsfsfd!!!!!!!!!" << std::endl;
  }
}

static inline bool check_and_advance(const char **s, const char *prefix, size_t len) {
  if (!std::strncmp(*s, prefix, len)) {
    *s += len;
    return true;
  }
  return false;
}

void Mpg123Player :: read_data(boost::process::ipstream&o) {
  try {
    std::string buf;
    const char* line;
    signal(SIGPIPE, SIG_IGN);
    signal(SIGABRT, SIG_IGN);

    try {
      while (mpgProc && mpgProc->proc.running() && o && std::getline(o, buf)) {
        line = buf.c_str();

        if (0){}
#define on(S) else if (check_and_advance(&line, S, STRLEN(S)))
        on("@SAMPLE ") {
          if (! sample_rate)
            flags |= FLAG_NEED_FORMAT;
          else {
            unsigned int samples_played, samples_total;
            std::sscanf(line, "%u %u", &samples_played, &samples_total);
            seconds_played = samples_played / sample_rate;
            seconds_total  = samples_total  / sample_rate;
          }
        }
        on("@FORMAT ") {
          std::sscanf(line, "%u %hhu", &sample_rate, &channels);
        }
        on("@P ") {
          state = std::atoi(line);
        }
        on("@F ") {
          int _;
          float played = 0, remaining = 0;
          std::sscanf(line, "%d %d %f %f", &_, &_, &played, &remaining);
          seconds_played    = played;
          seconds_remaining = remaining;
          seconds_total     = seconds_played + seconds_remaining;
        }
        on("@E ") {
        }
        on("@I ") {
          //std::cerr << line << ENDL;
        }
        on("@R ") {
        }
        on("@S ") {
        }
        else {
          std::cerr << "Mpg123Player: ?line:" << buf << std::endl;
        }
      }
    }
    catch (const std::exception &e) {
      std::cerr << "read_data(): Error occured: " << e.what() << std::endl;
    }
    catch (...) {
      std::cerr << "read_data(): Caught unknown exception" << std::endl;
    }
  }
  catch (const std::exception&e) {
    std::cerr << "ERROR DO HORENSOH:" << e.what() << std::endl;
  }
  catch (...) {
    std::cerr << "UNIOWN ERER" << std::endl;
  }
}

void Mpg123Player :: set_position(unsigned seconds) {
  if (mpgProc)
  *mpgProc << "J " << seconds << 's' << ENDL;
}

void Mpg123Player :: seek_forward(unsigned seconds) {
  if (mpgProc)
  *mpgProc << "J +" << seconds << 's' << ENDL;
}

void Mpg123Player :: seek_backward(unsigned seconds) {
  if (mpgProc)
  *mpgProc << "J -" << seconds << 's' << ENDL;
}

void Mpg123Player :: pause() {
  if (mpgProc)
    if (state == STATE_PLAYING)
      *mpgProc << 'P' << ENDL;
}

void Mpg123Player :: toggle() {
  if (mpgProc)
  *mpgProc << 'P' << ENDL;
}


//in << "VOLUME 0.1" << ENDL; // XXX

#if TEST_PLAYER
#include <unistd.h>
#include <iostream>
#include <cassert>

int main() {
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
}
#endif
