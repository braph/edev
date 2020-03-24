#ifndef MPG123_PLAYER_HPP
#define MPG123_PLAYER_HPP

#include <string>
#include <cstdint>

#include "lib/process.hpp"

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

  void work() noexcept;

  void play() noexcept;
  void play(const std::string&) noexcept;
  void stop() noexcept;
  void pause() noexcept;
  void toggle() noexcept;
  void position(int) noexcept;
  void seekForward(int) noexcept;
  void seekBackward(int) noexcept;

  State state()            const noexcept { return _state;            }
  bool  isStopped()        const noexcept { return _state == STOPPED; }
  bool  isPlaying()        const noexcept { return _state == PLAYING; }
  bool  isPaused()         const noexcept { return _state == PAUSED;  }
  bool  isLoading()        const noexcept { return _state == LOADING; }
  bool  isTrackCompleted() const noexcept { return _track_completed;  }
  int   position()         const noexcept { return _seconds_played;   }
  int   length()           const noexcept { return _seconds_total;    }
  float percent()          const noexcept { return (length() ? float(position()) / length() : 0); }
  void  percent(float p)         noexcept { position(length() * p);   }

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

  void reset() noexcept;
  void read_stderr() noexcept;
  void read_stdout() noexcept;
  void parse_stdout_line(const char*) noexcept;
};

#endif
