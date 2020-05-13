#ifndef LIB_RAII_FILE_HPP
#define LIB_RAII_FILE_HPP

#include <cstdio>
#include <memory>

namespace RAII {

struct FileDeleter {
  void operator()(std::FILE* file) {
    fclose(file);
  }
};

struct FILE : public std::unique_ptr<std::FILE, FileDeleter> {
  using std::unique_ptr<std::FILE, FileDeleter>::unique_ptr;

  operator std::FILE*() const noexcept {
    return get();
  }
};

} // namespace RAII


#endif
