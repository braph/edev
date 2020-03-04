#include "filesystem.hpp"

#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>

namespace Filesystem {

const char* home() {
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

std::string expand(std::string path) {
  if (!path.empty() && path[0] == '~')
    path.replace(0, 1, home());
  return path;
}

size_t dir_size(const boost::filesystem::path& path) {
  boost::system::error_code e;
  size_t s = 0;

  if (boost::filesystem::is_regular_file(path)) {
    s = boost::filesystem::file_size(path, e);
    if (e)
      s = 0;
  }
  else if (boost::filesystem::is_directory(path)) {
    for (auto& f : boost::filesystem::directory_iterator(path, e))
      s += dir_size(f.path());
  }

  return s;
}

} // namespace Filesystem

#ifdef TEST_FILESYSTEM
#include "test.hpp"
#define NOT_REACHED assert(!"Not reached")
int main() {
  assert( Filesystem::exists("/"));
  assert(!Filesystem::exists("/non-existant"));
  assert( Filesystem::exists(Filesystem::home()));
  assert( Filesystem::expand("~")     == Filesystem::home());
  assert( Filesystem::expand("~/foo") == Filesystem::home() + "/foo");

  try { Filesystem::mkdir("/this/does/not/exist"); NOT_REACHED; }
  catch (std::exception _) { /* OK */ }

  try { Filesystem::mkdir("/"); NOT_REACHED; }
  catch (std::exception _) { /* OK */ }

  rmdir               ("__a_directory_to_be_created");
  Filesystem::mkdir   ("__a_directory_to_be_created");
  Filesystem::mkdir_p ("__a_directory_to_be_created");
  rmdir               ("__a_directory_to_be_created");

  Filesystem::mkdir_p ("/this/is/a/test");
  Filesystem::mkdir_p ("/this/is/a/test/");
}
#endif

