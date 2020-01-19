#ifndef _MPG123_PLAYER_HPP
#define _MPG123_PLAYER_HPP

#include <string>
#include <iostream>//XXX
#include <pthread.h>

#include <boost/process.hpp>
#include <boost/algorithm/string.hpp>

/* XXX: in/out locks? */

/* XXX: We dont do events */

namespace bp = boost::process;
namespace bs = boost::algorithm;

enum {
  STATE_STOPPED = 0,
  STATE_PAUSED  = 1,
  STATE_PLAYING = 2
};

class Mpg123Player {
  private:
    bp::child proc;
    bp::opstream in;
    bp::ipstream out;
    bp::ipstream err;
    std::string file;
    std::string audio_system;
    unsigned char state;
    unsigned char channels;
    unsigned int  sample_rate;
    unsigned int  seconds_played;
    unsigned int  seconds_total;
    unsigned int  seconds_remaining;

    void start_mpg123_thread();

  public:
    Mpg123Player();

    void poll();

    void play();
    void play(const std::string&);
    void pause();
    void toggle();
    void set_position(unsigned);
    void seek_forward(unsigned);
    void seek_backward(unsigned);

    inline bool is_paused()        { return state == STATE_PAUSED;  }
    inline bool is_stopped()       { return state == STATE_STOPPED; }
    inline bool is_playing()       { return state == STATE_PLAYING; }
    inline unsigned int position() { return seconds_played; }
    inline unsigned int length()   { return seconds_total;  }
    inline unsigned int percent()  { return (seconds_total ? position() / length() : 0); }
};

#endif

   def stop
      stop_polling_thread
      @track_completed = nil
      @seconds_played = @seconds_total = 0
      @events.trigger(:position_change)
      @events.trigger(:stop)
      write(?Q) if @state != STATE_STOPPED
   end


Mpg123Player :: Mpg123Player()
  :
    audio_system("jack,pulse,alsa,oss"),
    state(STATE_STOPPED),
    channels(0),
    sample_rate(0),
    seconds_total(0),
    seconds_remaining(0),
    seconds_played(0)
{
}

void Mpg123Player :: play() {
  play(file);
}

void Mpg123Player :: play(const std::string &_file) {
  start_mpg123_thread();
  //   @track_completed = :track_completed
  file = _file;
  in << "L " << file << std::endl;
  // Thread.new { sleep 3; write(CMD_FORMAT) }
}

void Mpg123Player :: set_position(unsigned seconds) {
  in << "J " << seconds << 's' << std::endl;
}

void Mpg123Player :: seek_forward(unsigned seconds) {
  in << "J -" << seconds << 's' << std::endl;
}

void Mpg123Player :: seek_backward(unsigned seconds) {
  in << "J +" << seconds << 's' << std::endl;
}

void Mpg123Player :: pause() {
  if (state == STATE_PLAYING)
    in << 'P' << std::endl;
}

void Mpg123Player :: toggle() {
  in << 'P' << std::endl;
}

void* poll_thread(void *data) {
  Mpg123Player* player = (Mpg123Player*) data;
  player->poll();
}

void Mpg123Player :: start_mpg123_thread() {
  // With lock...
  if (proc.valid() && proc.running())
    return;

  proc = bp::child(
    bp::search_path("mpg123"), "-o", audio_system, "--fuzzy", "-R",
    bp::std_in < in, bp::std_out > out, bp::std_err > err
  );

  in << "SILENCE" << std::endl;
  in << "FORMAT"  << std::endl; // TODO: pause?

  pthread_t thr;
  pthread_create(&thr, NULL, poll_thread, this);
}

//while (proc.running() && std::getline(out, line)) {

void Mpg123Player :: poll() {
  in << "SAMPLE" << std::endl;
  in << "VOLUME 0.1" << std::endl; // XXX

  std::string line;
  std::string command;

  while (std::getline(out, line) ) {
    //std::cout << "Parsing: " << line << std::endl;
    auto idx = line.find(' ');
    if (idx != std::string::npos) {
      command  = line.substr(0, idx);
      line     = line.substr(idx + 1);
    } else {
      command  = line;
      line     = "";
    }

#define scase(S) else if (command == S)
    if (0){}
    scase("@FORMAT") {
      std::sscanf(line.c_str(), "%d %d", &sample_rate, &channels);
    }
    scase("@F") {
      int _;
      std::sscanf(line.c_str(), "%d %d %f %f", &_, &_, &seconds_played, &seconds_remaining);
      seconds_total = seconds_played + seconds_remaining;
      // @events.trigger(:position_change)
    }
    scase("@SAMPLE") {
      if (! sample_rate) // @FORMAT not called
        sample_rate = 44100;

      int samples_played, samples_total;
      std::scanf(line.c_str(), "%d %d", &samples_played, &samples_total);
      /*
      @seconds_played = samples_played / @sample_rate rescue 0.0
      @seconds_total = samples_total / @sample_rate rescue 0.0
      */
    }
    scase("@P") {
      state = std::stoi(line);
      /*
      if (@state) == STATE_STOPPED    @events.trigger(:stop, @track_completed)
      elsif @state == STATE_PAUSED    @events.trigger(:pause)
      elsif @state == STATE_PLAYING   @events.trigger(:play)
      end
      */
    }
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
  }
}

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


