#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <boost/filesystem.hpp>

#include <string>

namespace Filesystem {
  using namespace boost::filesystem;
  using boost::system::error_code;

  const char* home() noexcept;
  std::string expand(std::string) noexcept;
  size_t      dir_size(const path&) noexcept;
}

#endif
