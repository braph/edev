#ifndef PROGRAMS_HPP
#define PROGRAMS_HPP

#include <lib/process.hpp>
#include <lib/stringpack.hpp>

#include <cstring>

#include <unistd.h>

namespace Programs {

struct String {
  const char *s;
  template<class T>
  inline String(const T& s_)          noexcept : s(s_.c_str()) {}
  inline String(const char* s_)       noexcept : s(s_)         {}
  inline operator const char*() const noexcept { return s; }
};

static inline Process file_archiver(String archive, String dest_dir) {
  return {[=](){
    ::close(STDIN_FILENO);
    ::dup2(STDERR_FILENO, STDOUT_FILENO);
    ::execlp("/bin/7z", "7z", "x", archive, NULL);
    ::execlp("/bin/unzip", "unzip", archive, NULL);
  }, false, false, false, dest_dir};
}

static inline Process image_viewer(String url) {
  return {[=](){
    ::execlp("/bin/feh",      "feh",      url, NULL);
    ::execlp("/bin/display",  "display",  url, NULL);
    ::execlp("/bin/xdg-open", "xdg-open", url, NULL);
  }, true, false, false};
}

static inline Process browser(String url) {
  const char* dot = std::strrchr(url, '.');
  using pack = StringPack::AlphaNoCase;
  switch (pack::pack_runtime(dot ? dot : "")) {
  case pack(".png"):
  case pack(".jpg"):
  case pack(".jpeg"):
    return image_viewer(url);
  default:
    return {[=](){
      ::execlp("xdg-open", "xdg-open", url, NULL);
    }, true, false, false};
  }
}

} // namespace Programs

#endif
