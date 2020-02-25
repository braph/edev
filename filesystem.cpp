#include "filesystem.hpp"

#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>

namespace Filesystem {

std::string home() {
  static std::string _home;

  if (_home.empty()) {
    const char *s = getenv("HOME");

    if (s && *s)
      _home = s;
    else {
      struct passwd *pwd = getpwuid(getuid());
      if (pwd)
        _home = pwd->pw_dir;
    }
  }

  return _home;
}

std::string expand(const std::string &path) {
  std::string new_path = path;

  if (path.length() >= 1 && path[0] == '~')
    new_path.replace(0, 1, home());

  return new_path;
}

//filesystem_error;
size_t dir_size(const boost::filesystem::path& path) {
  boost::system::error_code e;
  size_t s;

  if (boost::filesystem::is_regular_file(path)) {
    s = boost::filesystem::file_size(path, e);
    return (e ? 0 : s);
  }
  else if (boost::filesystem::is_directory(path)) {
    size_t s = 0;
    for (boost::filesystem::directory_entry& f : boost::filesystem::directory_iterator(path))
      s += file_size(f.path());
    return s;
  }

  return 0;
}

} // namespace Filesystem

#if TEST_FILESYSTEM
#include <cassert>
#include <iostream>
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



