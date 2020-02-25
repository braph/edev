#ifndef _FILESYSTEM_HPP
#define _FILESYSTEM_HPP

#include <boost/filesystem.hpp>

#include <string>

#define PATH_SEP "/"

namespace Filesystem {
  std::string home();
  std::string expand(const std::string&);
  size_t      dir_size(const boost::filesystem::path&);
}

#endif
