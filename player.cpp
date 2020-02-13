#include "player.hpp"

#include "common.hpp"

#include <iostream> //XXX

Mpg123Player :: Mpg123Player()
: audio_system("jack,pulse,alsa,oss")
, flags(FLAG_NEED_FORMAT)
, state(STATE_STOPPED)
, channels(0)
, sample_rate(0)
, seconds_total(0)
, seconds_played(0)
, seconds_remaining(0)
{
}

Mpg123Player :: ~Mpg123Player() {
  proc.terminate();
  in.close();
  out.close();
  if (thr.joinable())
    thr.join();
}

void Mpg123Player :: play() {
  if (file.size())
    play(file);
}

void Mpg123Player :: play(const std::string &_file) {
  file = _file;

  if (proc.valid() && proc.running())
    return;

  flags = FLAG_NEED_FORMAT;
  sample_rate = 0;

  proc = boost::process::child(
    boost::process::search_path("mpg123"),
    "-o", audio_system, "--fuzzy", "-R",
    boost::process::std_in  < in,
    boost::process::std_out > out,
    boost::process::std_err > err
  );

  in << "L " << file << std::endl;
  in << "SILENCE"    << std::endl;

  thr = std::thread(&Mpg123Player::read_data, this);
}

void Mpg123Player :: set_position(unsigned seconds) {
  in << "J " << seconds << 's' << std::endl;
}

void Mpg123Player :: seek_forward(unsigned seconds) {
  in << "J +" << seconds << 's' << std::endl;
}

void Mpg123Player :: seek_backward(unsigned seconds) {
  in << "J -" << seconds << 's' << std::endl;
}

void Mpg123Player :: pause() {
  if (state == STATE_PLAYING)
    in << 'P' << std::endl;
}

void Mpg123Player :: toggle() {
  in << 'P' << std::endl;
}

void Mpg123Player :: stop() {
  in << 'S' << std::endl;
}

void Mpg123Player :: poll() {
  if (flags & FLAG_NEED_FORMAT) {
    flags &= ~FLAG_NEED_FORMAT;
    in << "FORMAT" << std::endl;
  }
  in << "SAMPLE" << std::endl;
}

static inline bool check_and_advance(const char **s, const char *prefix, size_t len) {
  if (!std::strncmp(*s, prefix, len)) {
    *s += len;
    return true;
  }
  return false;
}

void Mpg123Player :: read_data() {
  std::string buf;
  const char* line;

  try {
    while (proc.valid() && proc.running() && std::getline(out, buf) ) {
      std::cerr << "Parsing: " << buf << std::endl;
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
    }
  }
  catch (const std::exception &e) {
    std::cerr << "Error occured: " << e.what() << std::endl;
  }

  std::cerr << "Player: Reading thread died" << std::endl;
}

//in << "VOLUME 0.1" << std::endl; // XXX

#if 0
   def stop
      stop_polling_thread
      @track_completed = nil
      @seconds_played = @seconds_total = 0
      @events.trigger(:position_change)
      @events.trigger(:stop)
      write(?Q) if @state != STATE_STOPPED
   end

    // rescue Ektoplayer::Application.log()...
    // send(cmd, line) rescue nil
    // ensure....
    // begin msg = mpg123_err.read
    // rescue msg = '' end
    /*
        Ektoplayer::Application.log(self, 'player closed:', msg)
        @mpg123_thread.kill if @mpg123_thread
        (@mpg123_in.close rescue nil)  if @mpg123_in
        (@mpg123_out.close rescue nil) if @mpg123_out
        @mpg123_thread = nil
        stop_polling_thread
    start_polling_thread if @polling_interval > 0
    */

#endif

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
