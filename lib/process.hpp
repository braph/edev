#ifndef LIB_PROCESS_HPP
#define LIB_PROCESS_HPP

#include "pipestream.hpp"

#include <sys/types.h> // pid_t

#include <functional>

class Process {
public:
  PipeStream stdin_pipe;
  PipeStream stdout_pipe;
  PipeStream stderr_pipe;

  Process(
      std::function<void()> function,
      bool pipe_stdin=true,
      bool pipe_stdout=true,
      bool pipe_stderr=true) noexcept;
 ~Process();

  pid_t  get_id()                         const noexcept { return _pid; }
  int    get_exit_status()                      noexcept;
  bool   try_get_exit_status(int &exit_status)  noexcept;
  void   close_stdin()                          noexcept;
  void   kill(bool force=false)                 noexcept;
  bool   running()                              noexcept;
  void   detach()                               noexcept;

  template<typename T>
  Process& operator<<(const T& v) noexcept {
    stdin_pipe << v;
    return *this;
  }

private:
  pid_t _pid;
  bool _closed;

  pid_t open(std::function<void()>, bool, bool, bool) noexcept;
  void close_fds() noexcept;
};

#endif
