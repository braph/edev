#ifndef _MPG123_PLAYER_HPP
#define _MPG123_PLAYER_HPP

#include <string>
#include <thread>

#include <boost/process.hpp>

/* XXX: in/out locks? */

class Mpg123Player {
public:
  enum {
    STATE_STOPPED = 0,
    STATE_PAUSED  = 1,
    STATE_PLAYING = 2
  };

  Mpg123Player();
 ~Mpg123Player();

  void poll();

  void play();
  void play(const std::string&);
  void pause();
  void toggle();
  void set_position(unsigned);
  void seek_forward(unsigned);
  void seek_backward(unsigned);

  inline bool isPaused()         { return state == STATE_PAUSED;  }
  inline bool isStopped()        { return state == STATE_STOPPED; }
  inline bool isPlaying()        { return state == STATE_PLAYING; }
  inline int  getState()         { return state;                  }
  inline unsigned int position() { return seconds_played;         }
  inline unsigned int length()   { return seconds_total;          }
  inline float        percent()  { return (seconds_total ? (float) position() / length() : 0); }

private:
  enum {
    FLAG_NEED_FORMAT = 1
  };

  std::thread thr;
  boost::process::child proc;
  boost::process::opstream in;
  boost::process::ipstream out;
  boost::process::ipstream err;
  std::string file;
  std::string audio_system;
  unsigned char flags;
  unsigned char state;
  unsigned char channels;
  unsigned int  sample_rate;
  unsigned int  seconds_total;
  unsigned int  seconds_played;
  unsigned int  seconds_remaining;

  void read_data();
  void start_mpg123();
};

#endif
