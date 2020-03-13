#ifndef PROCESS_HPP
#define PROCESS_HPP

#include "pipestream.hpp"

#include <sys/wait.h>

#include <string>
#include <mutex>
#include <memory>
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
 ~Process() noexcept;
  
  pid_t get_id() const noexcept;
  int get_exit_status() noexcept;
  bool try_get_exit_status(int &exit_status) noexcept;
  void close_stdin() noexcept;
  void kill(bool force=false) noexcept;
  bool running() noexcept;

  template<typename T>
  Process& operator<<(T v) noexcept {
    stdin_pipe << v;
    return *this;
  }

  template<size_t LEN>
  Process& operator<<(const char (&s)[LEN]) noexcept {
    stdin_pipe.write(s, LEN-1);
    return *this;
  }

private:
  pid_t pid;
  bool closed;
  std::mutex close_mutex;
  std::mutex stdin_mutex;
  pid_t open(std::function<void()>, bool, bool, bool) noexcept;

  void close_fds() noexcept;
};

#endif
