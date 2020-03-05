#include "process.hpp"

#include <cstdlib>
#include <unistd.h>
#include <signal.h>
#include <stdexcept>

Process::Process(std::function<void()> function, bool pipe_stdin, bool pipe_stdout, bool pipe_stderr) noexcept
: closed(true)
, pid(-1)
{
  assert(function);
  open(function, pipe_stdin, pipe_stdout, pipe_stderr);
}

Process::id_type Process::open(std::function<void()> function, bool pipe_stdin, bool pipe_stdout, bool pipe_stderr) noexcept {
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
  
    //Based on http://stackoverflow.com/a/899533/3808293
    int fd_max=static_cast<int>(sysconf(_SC_OPEN_MAX)); // truncation is safe
    for(int fd=3;fd<fd_max;fd++)
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
  
  fcntl(stdout_pipe.fd, F_SETFL, fcntl(stdout_pipe.fd, F_GETFL) | O_NONBLOCK);
  fcntl(stderr_pipe.fd, F_SETFL, fcntl(stderr_pipe.fd, F_GETFL) | O_NONBLOCK);

  closed=false;
  this->pid = pid;
  return pid;
}

int Process::get_exit_status() noexcept {
  if (pid<= 0)
    return -1;

  int exit_status;
  waitpid(pid, &exit_status, 0);
  {
    std::lock_guard<std::mutex> lock(close_mutex);
    closed=true;
  }
  close_fds();

  if (exit_status >= 256)
    exit_status = exit_status >> 8;
  return exit_status;
}

bool Process::try_get_exit_status(int &exit_status) noexcept {
  if (pid<= 0)
    return false;

  id_type p = waitpid(pid, &exit_status, WNOHANG);
  if (p == 0)
    return false;

  {
    std::lock_guard<std::mutex> lock(close_mutex);
    closed=true;
  }
  close_fds();

  if (exit_status>=256)
    exit_status=exit_status>>8;

  return true;
}

bool Process :: running() noexcept {
  int retcode;
  return !try_get_exit_status(retcode);
}

void Process::close_fds() noexcept {
  stdin_pipe.close();
  stdout_pipe.close();
  stderr_pipe.close();
#if 0
  if (stdout_fd) {
    if (pid > 0)
      close(*stdout_fd);
    stdout_fd.reset();
  }
  if (stderr_fd) {
    if (pid > 0)
      close(*stderr_fd);
    stderr_fd.reset();
  }
#endif
}

void Process::kill(bool force) noexcept {
  std::lock_guard<std::mutex> lock(close_mutex);
  if (pid > 0 && !closed) {
    if (force)
      ::kill(-pid, SIGTERM);
    else
      ::kill(-pid, SIGINT);
  }
}

void Process::kill(id_type id, bool force) noexcept {
  if (id<= 0)
    return;

  if (force)
    ::kill(-id, SIGTERM);
  else
    ::kill(-id, SIGINT);
}

Process::~Process() noexcept {
  close_fds();
}

Process::id_type Process::get_id() const noexcept {
  return pid;
}

