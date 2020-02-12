#ifndef _FILESYSTEM_HPP
#define _FILESYSTEM_HPP

#include <string>

#define PATH_SEP "/"

namespace Filesystem {
  std::string home();
  std::string expand(const std::string&);
}

#endif
