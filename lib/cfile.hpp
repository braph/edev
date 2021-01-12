#ifndef LIB_CFILE_HPP
#define LIB_CFILE_HPP

#include <cstdio>

struct CFile {
  CFile(FILE* fh) noexcept : _fh(fh) {}

  ~CFile() {
    close();
  }

  static CFile fopen(const char *pathname, const char *mode) noexcept
  { return CFile(fopen(pathname, mode)); }

#if _POSIX_C_SOURCE
  static CFile fdopen(int fd, const char *mode) noexcept
  { return CFile(fdopen(fd, mode)); }
#endif

  void reopen(const char *pathname, const char *mode, FILE *stream) noexcept
  { _fh = freopen(pathname, mode, _fh); }

  inline operator FILE*() const noexcept { return _fh; }
  inline operator bool()  const noexcept { return _fh; }

  inline size_t fread(void *ptr, size_t size, size_t nmemb)        { return std::fread(ptr, size, nmemb, _fh); }
  inline size_t fwrite(const void *ptr, size_t size, size_t nmemb) { return std::fwrite(ptr, size, nmemb, _fh); }

#if !defined(NDEBUG) && (defined(__GNUC__) || defined(__clang__))
  __attribute__((__format__(__printf__, 2, 3)))
  int printf(const char* fmt, ...) noexcept {
    va_list ap;
    va_start(ap, fmt);
    int r = std::fprintf(_fh, fmt, ap);
    va_end(ap);
    return r;
  }
#else
  template<typename ... T>
  int printf(const char* fmt, T ... args) noexcept {
    return std::fprintf(_fh, fmt, args ...);
  }
#endif

#if !defined(NDEBUG) && (defined(__GNUC__) || defined(__clang__))
  __attribute__((__format__(__scanf__, 2, 3)))
  int scanf(const char* fmt, ...) noexcept {
    va_list ap;
    va_start(ap, fmt);
    int r = std::fscanf(_fh, fmt, ap);
    va_end(ap);
    return r;
  }
#else
  template<typename ... T>
  int scanf(const char* fmt, T ... args) noexcept {
    return std::fscanf(_fh, fmt, args ...);
  }
#endif

  inline int vfprintf(const char *format, va_list ap) {
    return std::vfprintf(_fh, format, ap);
  }

  inline int clearerr()           noexcept { return std::clearerr(_fh); }
  inline int close()              noexcept { return std::fclose(_fh);   }
  inline int eof()                noexcept { return std::feof(_fh);     }
  inline int error()              noexcept { return std::ferror(_fh);   }
  inline int flush()              noexcept { return std::fflush(_fh);   }
  inline int getc()               noexcept { return std::fgetc(_fh);    }
  inline char *gets(char *s, int size)     { return std::fgets(s, size, _fh); }
  inline int fputc(int c)         noexcept { return std::fputc(c, _fh); }
  inline int fputs(const char *s) noexcept { return std::fputs(s, _fh); }

  inline int getpos(fpos_t *pos)       noexcept { return std::fgetpos(_fh, pos); }
  inline int setpos(const fpos_t *pos) noexcept { return std::fsetpos(_fh, pos); }

  inline long tell()                  noexcept { return std::ftell(_fh);  }
  inline void rewind()                 noexcept { return std::rewind(_fh); }

  inline int  seek(long offset, int whence) noexcept { return std::fseek(_fh, offset, whence); }

#if 0
  int      fseeko(FILE *, off_t, int);
  off_t    ftello(FILE *);
#endif

#ifdef _POSIX_C_SOURCE
  inline int fileno() { return ::fileno(_fh); }
#endif

#if /* Since glibc 2.24: */       _POSIX_C_SOURCE >= 199309L \
||  /* Glibc versions <= 2.23: */ _POSIX_C_SOURCE \
||  /* Glibc versions <= 2.19: */ _BSD_SOURCE || _SVID_SOURCE
  inline void lockfile()    { return ::flockfile(_fh);    }
  inline int  trylockfile() { return ::ftrylockfile(_fh); }
  inline void unlockfile()  { return ::funlockfile(_fh);  }
#endif

private:
  FILE* _fh;
};

#endif
