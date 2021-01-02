#ifndef URL_HANDLER_HPP
#define URL_HANDLER_HPP

#include <lib/stringpack.hpp>

#include <unistd.h>

#include <cstdlib>

static inline void unpack_archive(const char* archive, const char* destination_dir) {
  if (::fork() != 0)
    return;

  char archive_abs_path[PATH_MAX];
  realpath(archive, archive_abs_path);

  Filesystem::create_directory(destination_dir);
  if (chdir(destination_dir) < 0)
    return;

  dup2(STDERR_FILENO, STDOUT_FILENO);
  ::execlp("/bin/7z", "7z", "x", archive_abs_path, NULL);
  ::execlp("/bin/unzip", "unzip", archive_abs_path, NULL);
  ::exit(0);
}

static inline void open_image(const char* url) {
  if (::fork() != 0)
    return;

  ::execlp("/bin/sh", "sh", "-c",
    "URL=$1; shift;"
    "for CMD; do :|$CMD \"$URL\" && break;"
    "done >/dev/null 2>/dev/null",

    // argv[0]
    "open_image",

    // $URL
    url,

    // $CMD's
    "feh",
    "display",
    "xdg-open",

    NULL
  );

  std::exit(0);
}

static inline void open_url(const char* url) {
  using pack = StringPack::AlphaNoCase;
  const char* dot = std::strrchr(url, '.');
  switch (pack::pack_runtime(dot ? dot : "")) {
  case pack(".png"):
  case pack(".jpg"):
  case pack(".jpeg"):
    open_image(url);
    break;
  default:
    if (::fork() == 0) {
      ::execlp("xdg-open", "xdg-open", url, NULL);
      std::exit(0);
    }
  }
}

#endif
