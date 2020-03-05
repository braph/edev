#ifndef _PROCESS_HPP
#define _PROCESS_HPP

#include "pipestream.hpp"

#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <memory>
#include <functional>
#include <sys/wait.h>

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
  
  /// Get the process id of the started process.
  id_type get_id() const noexcept;
  /// Wait until process is finished, and return exit status.
  int get_exit_status() noexcept;
  /// If process is finished, returns true and sets the exit status. Returns false otherwise.
  bool try_get_exit_status(int &exit_status) noexcept;
  /// Write to stdin.
  bool write(const char *bytes, size_t n);
  /// Write to stdin. Convenience function using write(const char *, size_t).
  bool write(const std::string &data);
  /// Close stdin. If the process takes parameters from stdin, use this to notify that all parameters have been sent.
  void close_stdin() noexcept;
  
  /// Kill the process. force=true is only supported on Unix-like systems.
  void kill(bool force=false) noexcept;
  /// Kill a given process id. Use kill(bool force) instead if possible. force=true is only supported on Unix-like systems.
  static void kill(id_type id, bool force=false) noexcept;

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
  std::thread stdout_thread, stderr_thread;
  id_type open(std::function<void()>, bool, bool, bool) noexcept;

  void close_fds() noexcept;
};

#endif
