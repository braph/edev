#ifndef LIB_PIPESTREAM_HPP
#define LIB_PIPESTREAM_HPP

#include <unistd.h>

#include <string>
#include <cstring>
#include <cstdio>

struct PipeStream {
  PipeStream() noexcept
    : _fd(-1)
  {}

  PipeStream(int filedes) noexcept
    : _fd(filedes)
  {}

 ~PipeStream() {
   close();
  }

  inline operator bool()  const noexcept { return _fd >= 0; }
  inline bool operator!() const noexcept { return _fd < 0;  }
  inline int fd()         const noexcept { return _fd;      }

  inline void open(int filedes) noexcept {
    close();
    _fd = filedes;
  }

  inline void close() noexcept {
    if (*this)
      ::close(_fd);
    _fd = -1;
  }

  // Read =====================================================================
  inline ssize_t read(void* buf, size_t nbyte) noexcept {
    return ::read(_fd, buf, nbyte);
  }

  template<size_t N>
  inline ssize_t read(char (&buf)[N]) noexcept {
    return ::read(_fd, buf, N);
  }

  inline ssize_t read(char* buf, size_t nbyte) noexcept {
    return ::read(_fd, buf, nbyte);
  }

  // Write ====================================================================
  inline ssize_t write(const void* buf, size_t nbyte) noexcept {
    return ::write(_fd, buf, nbyte);
  }

  inline ssize_t write(const char* buf, size_t nbyte) noexcept {
    return ::write(_fd, buf, nbyte);
  }

  inline ssize_t write(const std::string& buf) noexcept {
    return ::write(_fd, &buf[0], buf.size());
  }

  inline ssize_t write(char c) noexcept {
    return ::write(_fd, &c, 1);
  }

  inline ssize_t write(const char* s) noexcept {
    return ::write(_fd, s, std::strlen(s));
  }

#if 0
  template<typename T>
  ssize_t write(T value) {
    char buf[32];
    size_t len = std::sprintf(buf, format_specifiers::print::specifier<T>::value, value);
    return ::write(_fd, buf, len);
  }
#endif

  // Operator << ==============================================================
  template<typename T>
  inline PipeStream& operator<<(const T& v) noexcept {
    write(v);
    return *this;
  }

private:
  int _fd;
};

#endif
