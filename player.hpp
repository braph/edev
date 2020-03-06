#ifndef _MPG123_PLAYER_HPP
#define _MPG123_PLAYER_HPP

#include <string>
#include <cstdint>
#include <functional>

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
  void position(int);
  void seekForward(int);
  void seekBackward(int);

  inline bool  isTrackCompleted() const { return _track_completed;  }
  inline bool  isPaused()         const { return _state == PAUSED;  }
  inline bool  isStopped()        const { return _state == STOPPED; }
  inline bool  isPlaying()        const { return _state == PLAYING; }
  inline bool  isLoading()        const { return _state == LOADING; }
  inline int   state()            const { return _state;            }
  inline int   position()         const { return _seconds_played;   }
  inline int   length()           const { return _seconds_total;    }
  inline float percent()          const { return (length() ? float(position()) / length() : 0); }
  inline void  percent(float p)         { position(length() * p); }

private:
  std::string _file;
  uint8_t _failed; // Automatically gives up trying on overflow :3
  State   _state;
  bool    _track_completed;
  int     _channels;
  int     _sample_rate;
  int     _seconds_total;
  int     _seconds_played;
  int     _seconds_remaining;
  std::unique_ptr<Process> _process;
  std::string _stdout_buffer;

  void reset();
  void parse_line(const char*);
  void read_output();
};

#endif
