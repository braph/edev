#ifndef _FILESYSTEM_HPP
#define _FILESYSTEM_HPP

#include <boost/filesystem.hpp>

#include <string>

#define PATH_SEP "/"

namespace Filesystem {
  const char* home();
  std::string expand(std::string);
  size_t      dir_size(const boost::filesystem::path&);
}

#endif
