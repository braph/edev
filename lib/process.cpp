#include "process.hpp"

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <stdexcept>

Process :: Process(std::function<void()> function, bool pipe_stdin, bool pipe_stdout, bool pipe_stderr) noexcept
  : _pid(-1)
  , _closed(true)
{
  open(function, pipe_stdin, pipe_stdout, pipe_stderr);
}

Process :: ~Process() {
  close_fds();
}

pid_t Process :: open(std::function<void()> function, bool pipe_stdin, bool pipe_stdout, bool pipe_stderr) noexcept {
  assert(function && "No function passed");

  int stdin_p[2], stdout_p[2], stderr_p[2];
  
  if (pipe_stdin && pipe(stdin_p) != 0)
    return -1;
  if (pipe_stdout && pipe(stdout_p) != 0) {
    if (pipe_stdin) {close(stdin_p[0]); close(stdin_p[1]);}
    return -1;
  }
  if (pipe_stderr && pipe(stderr_p) != 0) {
    if (pipe_stdin)  {close(stdin_p[0]);  close(stdin_p[1]);}
    if (pipe_stdout) {close(stdout_p[0]); close(stdout_p[1]);}
    return -1;
  }
  
  pid_t pid = fork();
  
  if (pid < 0) {
    if (pipe_stdin)  {close(stdin_p[0]);  close(stdin_p[1]);}
    if (pipe_stdout) {close(stdout_p[0]); close(stdout_p[1]);}
    if (pipe_stderr) {close(stderr_p[0]); close(stderr_p[1]);}
    return pid;
  }
  else if (pid == 0) {
    if (pipe_stdin)  dup2(stdin_p[0], 0);
    if (pipe_stdout) dup2(stdout_p[1], 1);
    if (pipe_stderr) dup2(stderr_p[1], 2);
    if (pipe_stdin)  {close(stdin_p[0]);  close(stdin_p[1]);}
    if (pipe_stdout) {close(stdout_p[0]); close(stdout_p[1]);}
    if (pipe_stderr) {close(stderr_p[0]); close(stderr_p[1]);}
  
    // Based on http://stackoverflow.com/a/899533/3808293
    int fd_max = static_cast<int>(sysconf(_SC_OPEN_MAX)); // truncation is safe
    for (int fd = 3; fd < fd_max; fd++)
      close(fd);
  
    setpgid(0, 0);
    
    function();
    
    _exit(EXIT_FAILURE);
  }
  
  if (pipe_stdin)  close(stdin_p[0]);
  if (pipe_stdout) close(stdout_p[1]);
  if (pipe_stderr) close(stderr_p[1]);
  
  if (pipe_stdin)  stdin_pipe.open(stdin_p[1]);
  if (pipe_stdout) stdout_pipe.open(stdout_p[0]);
  if (pipe_stderr) stderr_pipe.open(stderr_p[0]);
  
  _closed = false;
  this->_pid = pid;
  return pid;
}

int Process :: get_exit_status() noexcept {
  if (_pid <= 0)
    return -1;

  int exit_status;
  waitpid(_pid, &exit_status, 0);
  _closed = true;
  close_fds();

  if (exit_status >= 256)
    exit_status = exit_status >> 8;
  return exit_status;
}

bool Process :: try_get_exit_status(int &exit_status) noexcept {
  if (_pid <= 0)
    return false;

  pid_t p = waitpid(_pid, &exit_status, WNOHANG);
  if (p == 0)
    return false;

  _closed = true;
  close_fds();

  if (exit_status >= 256)
    exit_status = exit_status >> 8;

  return true;
}

bool Process :: running() noexcept {
  int retcode;
  return !try_get_exit_status(retcode);
}

void Process :: close_fds() noexcept {
  stdin_pipe.close();
  stdout_pipe.close();
  stderr_pipe.close();
  // if (_pid > 0) ?!
}

void Process :: kill(bool force) noexcept {
  if (_pid > 0 && !_closed) {
    if (force)
      ::kill(-_pid, SIGTERM);
    else
      ::kill(-_pid, SIGINT);
  }
}

pid_t Process :: get_id() const noexcept {
  return _pid;
}

