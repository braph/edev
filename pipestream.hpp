#ifndef _PIPESTREAM_HPP
#define _PIPESTREAM_HPP

#include <unistd.h>

#include <string>

struct PipeStream {
  int fd;

  PipeStream() noexcept
    : fd(-1) {}

  PipeStream(int filedes) noexcept
    : fd(filedes) {}

 ~PipeStream()
  { close(); }

  operator bool()  const noexcept { return fd >= 0; }
  bool operator!() const noexcept { return fd <  0; }

  void open(int filedes) noexcept {
    close();
    fd = filedes;
  }

  void close() noexcept {
    if (*this)
      ::close(fd);
    fd = -1;
  }

  // Read =====================================================================
  ssize_t read(void* buf, size_t nbyte) noexcept {
    return ::read(fd, buf, nbyte);
  }

  ssize_t read(char* buf, size_t nbyte) noexcept {
    return ::read(fd, buf, nbyte);
  }

  // Write ====================================================================
  ssize_t write(const void* buf, size_t nbyte) noexcept {
    return ::write(fd, buf, nbyte);
  }

  ssize_t write(const char* buf, size_t nbyte) noexcept {
    return ::write(fd, buf, nbyte);
  }

  ssize_t write(const std::string& buf) noexcept {
    return ::write(fd, buf.c_str(), buf.size());
  }

  template<size_t LEN>
  PipeStream& operator<<(const char (&s)[LEN]) noexcept {
    ::write(fd, s, LEN-1);
    return *this;
  }

  PipeStream& operator<<(const std::string& s) noexcept {
    ::write(fd, s.c_str(), s.size());
    return *this;
  }

  template<typename T>
  PipeStream& operator<<(T v) noexcept {
    write(std::to_string(v));
    return *this;
  }
};

#endif
