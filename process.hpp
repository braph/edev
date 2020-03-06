#ifndef _PROCESS_HPP
#define _PROCESS_HPP

#include "pipestream.hpp"

#include <sys/wait.h>

#include <string>
#include <mutex>
#include <memory>
#include <functional>

class Process {
public:
  typedef pid_t id_type;
  typedef int fd_type;
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
  
  id_type get_id() const noexcept;
  int get_exit_status() noexcept;
  bool try_get_exit_status(int &exit_status) noexcept;
  void close_stdin() noexcept;
  void kill(bool force=false) noexcept;
  bool running() noexcept;

  template<size_t LEN>
  Process& operator<<(const char (&s)[LEN]) {
    stdin_pipe.write(s, LEN-1);
    return *this;
  }

  Process& operator<<(const std::string& s) {
    stdin_pipe.write(s);
    return *this;
  }

  template<typename T>
  Process& operator<<(T v) {
    stdin_pipe << v;
    return *this;
  }
  
private:
  pid_t pid;
  bool closed;
  std::mutex close_mutex;
  std::mutex stdin_mutex;
  id_type open(std::function<void()>, bool, bool, bool) noexcept;

  void close_fds() noexcept;
};

#endif
