#ifndef _MPG123_PLAYER_HPP
#define _MPG123_PLAYER_HPP

#include <string>
#include <mutex>
#include <memory>
#include <thread>
#include <cstdint>
#include <iostream>
#include <functional>

#include <boost/process.hpp>
#include "process.hpp"

class Mpg123Player {
public:
  enum State : uint8_t {
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
  void set_position(int);
  void seek_forward(int);
  void seek_backward(int);

  inline bool  isTrackCompleted() const { return track_completed;  }
  inline bool  isPaused()         const { return state == PAUSED;  }
  inline bool  isStopped()        const { return state == STOPPED; }
  inline bool  isPlaying()        const { return state == PLAYING; }
  inline int   getState()         const { return state;            }
  inline int   position()         const { return seconds_played;   }
  inline int   length()           const { return seconds_total;    }
  inline float percent() const { return (length() ? static_cast<float>(position()) / length() : 0); }
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
  uint8_t failed; // Automatically gives up trying on overflow :3
  State   state;
  bool    track_completed;
  int     channels;
  int     sample_rate;
  int     seconds_total;
  int     seconds_played;
  int     seconds_remaining;
  std::unique_ptr<Mpg123Process> process;

  void parse_line(const char*);
  void read_output(const char*, size_t);
};

#endif
