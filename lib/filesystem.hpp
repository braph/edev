#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#if __cplusplus < 201703L
#include <boost/filesystem.hpp>
#else
#include <filesystem>
#endif

#include <string>

namespace Filesystem {
#if __cplusplus < 201703L
  using namespace boost::filesystem;
  using boost::system::error_code;
#else
  using namespace std::filesystem;
  using std::error_code;
#endif

  const char* home()                noexcept;
  std::string expand(std::string)   noexcept;
  size_t      dir_size(const path&) noexcept;
}

#endif
