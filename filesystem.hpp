#ifndef _FILESYSTEM_HPP
#define _FILESYSTEM_HPP

#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <stdexcept>

#define PATH_SEP      "/"
#define DEFAULT_PERMS (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)

#include <iostream> //XXX:rm

namespace Filesystem {
  std::string home();
  std::string expand(const std::string&);

  static inline void mkdir(const std::string& dir, mode_t mode = DEFAULT_PERMS) {
    if (::mkdir(dir.c_str(), mode)) {
      const char *msg = std::strerror(errno);
      throw std::invalid_argument(msg);
    }
  }

  static inline void mkdir_p(const std::string& dir, mode_t mode = DEFAULT_PERMS) {
    size_t idx = 0;
    std::string part;

    for (;;) {
      idx = dir.find(PATH_SEP, idx);
      if (idx == std::string::npos)
        break;

      part = dir.substr(0, idx);
      if (part != "")
        std::cout << part << std::endl;
      ++idx;
    }

    if (part != dir)
      std::cout << dir << std::endl;
  }

  static inline bool exists(const std::string& file) {
    return 0 == access(file.c_str(), F_OK);
  }
}

#endif
