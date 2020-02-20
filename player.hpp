#ifndef _MPG123_PLAYER_HPP
#define _MPG123_PLAYER_HPP

#include <string>
#include <thread>
#include <iostream>
#include <mutex>
#include <memory>
#include <functional>

#include <boost/process.hpp>
#include "process.hpp"

class Mpg123Player {
public:
  enum State : char {
    STOPPED = 0, // <-.
    PAUSED  = 1, //    } Mpg123 
    PLAYING = 2, // <-'
    LOADING = 3, // <--- New introduced
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

  inline bool isTrackCompleted() { return track_completed;  }
  inline bool isPaused()         { return state == PAUSED;  }
  inline bool isStopped()        { return state == STOPPED; }
  inline bool isPlaying()        { return state == PLAYING; }
  inline int  getState()         { return state;            }
  inline unsigned int position() { return seconds_played;   }
  inline unsigned int length()   { return seconds_total;    }
  inline float        percent()  { return (length() ? static_cast<float>(position()) / length() : 0); }
  inline void  setPostionByPercent(float p) { set_position(length() * p); }

  struct Mpg123Process : public TinyProcessLib::Process {
    using TinyProcessLib::Process::Process;

    template<size_t LEN>
    Mpg123Process& operator<<(const char (&s)[LEN]) {
      write(s, LEN-1);
      return *this;
    }

    Mpg123Process& operator<<(const std::string& s) {
      write(s);
      return *this;
    }

    template<typename T>
    Mpg123Process& operator<<(T v) {
      write(std::to_string(v));
      return *this;
    }

    bool running() {
      int retcode;
      return !try_get_exit_status(retcode);
    }
  };

#if 0
  struct Mpg123Process {
      proc = boost::process::child (
        boost::process::search_path("mpg123"),
        "-o", "jack,pulse,alsa,oss", "--fuzzy", "-R",
        boost::process::std_in  < in,
        boost::process::std_out > out,
        boost::process::std_err > boost::process::null
      );
    }

    Mpg123Process& operator<<(bar e) {
      std::lock_guard<std::mutex> lock(in_mutex);
      try { in << std::endl; } catch (...) { }
      return *this;
    }
  };
#endif

private:
  std::string file;
  std::string audio_system;
  unsigned char failed; // Automatically gives up trying on overflow :3
  State         state;
  bool          track_completed;
  unsigned char channels;
  unsigned int  sample_rate;
  unsigned int  seconds_total;
  unsigned int  seconds_played;
  unsigned int  seconds_remaining;
  std::unique_ptr<Mpg123Process> process;

  void parse_line(const char*);
  void read_output(const char*, size_t);
};

#endif
