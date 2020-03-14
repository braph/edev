#include "filesystem.hpp"

#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>

namespace Filesystem {

const char* home() noexcept {
  static const char* _home = NULL;

  if (!_home) {
    _home = getenv("HOME");

    if (!_home || !*_home) {
      struct passwd *pwd = getpwuid(getuid());
      if (pwd)
        _home = pwd->pw_dir;
    }
  }

  if (!_home)
    _home = "";

  return _home;
}

std::string expand(std::string path) noexcept {
  if (!path.empty() && path[0] == '~')
    path.replace(0, 1, home());
  return path;
}

size_t dir_size(const path& path) noexcept {
  boost::system::error_code e;
  size_t s = 0;

  if (is_regular_file(path)) {
    s = file_size(path, e);
    if (e)
      s = 0;
  }
  else if (is_directory(path)) {
    for (auto& f : directory_iterator(path, e))
      s += dir_size(f.path());
  }

  return s;
}

} // namespace Filesystem

#ifdef TEST_FILESYSTEM
#include "test.hpp"
int main() {
  assert( Filesystem::exists("/"));
  assert(!Filesystem::exists("/non-existant"));
  assert( Filesystem::exists(Filesystem::home()));
  assert( Filesystem::expand("~")     == Filesystem::home());
  assert( Filesystem::expand("~/foo") == std::string(Filesystem::home()) + "/foo");
}
#endif

