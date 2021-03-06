#ifndef MPG123_PLAYBACK_HPP
#define MPG123_PLAYBACK_HPP

#include <string>
#include <memory>
#include <cstdint>

#include <lib/process.hpp>

class Mpg123Playback {
public:
  enum State : uint8_t {
    STOPPED = 0, // <-.
    PAUSED  = 1, //    } Mpg123
    PLAYING = 2, // <-'
    LOADING = 3, // <--- New introduced
  };

  Mpg123Playback();

  void work()             noexcept;
  void play()             noexcept;
  void play(std::string)  noexcept;
  void stop()             noexcept;
  void pause()            noexcept;
  void toggle()           noexcept;
  void position(int)      noexcept;
  void seek(int)          noexcept;
  void seek_forward(int)  noexcept;
  void seek_backward(int) noexcept;
  void volume(int)        noexcept;

  State state()              const noexcept { return _state;            }
  bool  is_stopped()         const noexcept { return _state == STOPPED; }
  bool  is_playing()         const noexcept { return _state == PLAYING; }
  bool  is_paused()          const noexcept { return _state == PAUSED;  }
  bool  is_loading()         const noexcept { return _state == LOADING; }
  bool  is_track_completed() const noexcept { return _track_completed;  }
  int   position()           const noexcept { return _seconds_played;   }
  int   length()             const noexcept { return _seconds_total;    }
  float percent()            const noexcept { return (length() ? float(position()) / length() : 0); }
  void  percent(float p)           noexcept { position(length() * p);   }

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

  void reset()                        noexcept;
  void read_stderr()                  noexcept;
  void read_stdout()                  noexcept;
  void parse_stdout_line(const char*) noexcept;
};

#endif
