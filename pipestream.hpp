#ifndef _PIPESTREAM_HPP
#define _PIPESTREAM_HPP

#include <unistd.h>

struct PipeStream {
  int fd;

  PipeStream() : fd(-1) {}
  PipeStream(int filedes) : fd(filedes) {}
 ~PipeStream() { close(); }

  inline operator bool()  const { return fd >= 0; }
  inline bool operator!() const { return fd <  0; }

  inline void open(int filedes) {
    close();
    fd = filedes;
  }

  inline void close() {
    if (*this)
      ::close(fd);
    fd = -1;
  }

  // Read =====================================================================
  inline ssize_t read(void* buf, size_t nbyte) {
    return ::read(fd, buf, nbyte);
  }

  inline ssize_t read(char* buf, size_t nbyte) {
    return ::read(fd, buf, nbyte);
  }

  // Write ====================================================================
  inline ssize_t write(const void* buf, size_t nbyte) {
    return ::write(fd, buf, nbyte);
  }

  inline ssize_t write(const char* buf, size_t nbyte) {
    return ::write(fd, buf, nbyte);
  }

  inline ssize_t write(const std::string& buf) {
    return write(buf.c_str(), buf.size());
  }

  template<size_t LEN>
  inline PipeStream& operator<<(const char (&s)[LEN]) {
    write(s, LEN-1);
    return *this;
  }

  inline PipeStream& operator<<(const std::string& s) {
    write(s);
    return *this;
  }

  template<typename T>
  inline PipeStream& operator<<(T v) {
    write(std::to_string(v));
    return *this;
  }
};

#endif
