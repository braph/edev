#ifndef _MPG123_PLAYER_HPP
#define _MPG123_PLAYER_HPP

#include <string>
#include <thread>
#include <iostream>
#include <mutex>
#include <functional>

#include <boost/process.hpp>

class Mpg123Player {
public:
  enum {
    STATE_STOPPED = 0, // -.
    STATE_PAUSED  = 1, //   } Mpg123 
    STATE_PLAYING = 2, // -'
    STATE_LOADING = 3,
  };

  Mpg123Player();
 ~Mpg123Player();

  void work();

  void play();
  void play(const std::string&);
  void stop();
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
  inline float        percent()  { return (seconds_total ? static_cast<float>(position()) / length() : 0); }

void read_data(boost::process::ipstream&o);

  enum bar {
    ENDL
  };

  struct Mpg123Process {
    boost::process::opstream in;
    boost::process::ipstream out;
    std::mutex in_mutex;
    boost::process::child proc;
    std::thread thr;

    Mpg123Process(Mpg123Player& player) : in(), out() {
      proc = boost::process::child (
        boost::process::search_path("mpg123"),
        "-o", "jack,pulse,alsa,oss", "--fuzzy", "-R",
        boost::process::std_in  < in,
        boost::process::std_out > out,
        boost::process::std_err > boost::process::null
      );

      thr = std::thread( [&] { player.read_data(out); } );
    }

  ~Mpg123Process() {
    thr.join();
    //proc.join();
    //in.close();
    //out.close();
  }

    template<typename T>
    Mpg123Process& operator<<(T v) {
      std::lock_guard<std::mutex> lock(in_mutex);
      try { in << v; } catch (...) { }
      return *this;
    }

    Mpg123Process& operator<<(bar e) {
      std::lock_guard<std::mutex> lock(in_mutex);
      try { in << std::endl; } catch (...) { }
      return *this;
    }
  };

private:
  enum {
    FLAG_NEED_FORMAT = 1
  };

  std::string file;
  std::string audio_system;
  unsigned char flags;
  unsigned char state;
  unsigned char channels;
  unsigned int  sample_rate;
  unsigned int  seconds_total;
  unsigned int  seconds_played;
  unsigned int  seconds_remaining;
  Mpg123Process* mpgProc;

  //void read_data();
  void start_mpg123();
};

#endif
