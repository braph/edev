#ifndef LAUNCHERS_HPP
#define LAUNCHERS_HPP

#include <lib/stringpack.hpp>
#include <lib/process.hpp>

#include <cstring>

#include <unistd.h>

namespace Lauchers {

static inline Process unpack_archive(const char* archive, const char* destination_dir) {
  char archive_abs_path[PATH_MAX];
  if (!::realpath(archive, archive_abs_path))
    return {[=](){ ::exit(1); }};

  Filesystem::create_directory(destination_dir);
  if (::chdir(destination_dir) < 0)
    return {[=](){ ::exit(1); }};

  return {[=](){
    ::close(STDIN_FILENO);
    ::dup2(STDERR_FILENO, STDOUT_FILENO);
    ::execlp("/bin/7z", "7z", "x", archive_abs_path, NULL);
    ::execlp("/bin/unzip", "unzip", archive_abs_path, NULL);
  }, false, false, false};
}

static inline Process open_image(const char* url) {
  return {[=](){
    ::execlp("/bin/feh",      "feh",      url, NULL);
    ::execlp("/bin/display",  "display",  url, NULL);
    ::execlp("/bin/xdg-open", "xdg-open", url, NULL);
  }, true, false, false};
}

static inline Process open_url(const char* url) {
  using pack = StringPack::AlphaNoCase;
  const char* dot = std::strrchr(url, '.');
  switch (pack::pack_runtime(dot ? dot : "")) {
  case pack(".png"):
  case pack(".jpg"):
  case pack(".jpeg"):
    return open_image(url);
  default:
    return {[=](){
      ::execlp("xdg-open", "xdg-open", url, NULL);
    }, true, false, false};
  }
}

} // namespace Lauchers

#endif
