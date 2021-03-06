#ifndef LIB_CFILE_HPP
#define LIB_CFILE_HPP

#include <cstdio>
#include <system_error>

// wide-char functions missing...

struct CFile {
  struct String {
    const char *s;
    template<class T>
    inline String(const T& s_)          noexcept : s(s_.c_str()) {}
    inline String(const char* s_)       noexcept : s(s_)         {}
    inline operator const char*() const noexcept { return s; }
  };

  CFile(FILE* fh) noexcept
    : _fh(fh)
  {}

  ~CFile() {
    close();
  }

  static CFile open(String pathname, String mode) {
    auto fh = std::fopen(pathname, mode);
    if (! fh)
      throw std::system_error(errno, std::generic_category());
    return CFile(fh);
  }

#ifdef _POSIX_C_SOURCE
  static CFile fdopen(int fd, String mode) {
    auto fh = ::fdopen(fd, mode);
    if (! fh)
      throw std::system_error(errno, std::generic_category());
    return CFile(fh);
  }
#endif

  void reopen(String pathname, String mode) {
    auto fh = std::freopen(pathname, mode, _fh);
    if (! fh)
      throw std::system_error(errno, std::generic_category());
  }

  inline operator FILE*()     const noexcept { return _fh; }
  inline operator bool()      const noexcept { return _fh; }

  inline size_t read(void *ptr, size_t size, size_t nmemb)        { return std::fread(ptr, size, nmemb, _fh);  }
  inline size_t write(const void *ptr, size_t size, size_t nmemb) { return std::fwrite(ptr, size, nmemb, _fh); }

  inline int  (getc)()              noexcept { return std::getc(_fh);           }
  inline char* gets(char *s, int size)       { return std::fgets(s, size, _fh); }
  inline int   ungetc(char c)       noexcept { return std::ungetc(c, _fh);      }

  inline int  (putc)(int c)         noexcept { return std::fputc(c, _fh);       }
  inline int   puts(String s)       noexcept { return std::fputs(s, _fh);       }

  inline int   flush()              noexcept { return std::fflush(_fh);         }
  inline int   close()              noexcept { int r = std::fclose(_fh); _fh = NULL; return r; }

#if (defined(_FILE_OFFSET_BITS) && _FILE_OFFSET_BITS == 64) || _POSIX_C_SOURCE >= 200112L
  inline int   seeko(off_t offset, int whence) noexcept { return ::fseeko(_fh, offset, whence); }
  inline off_t tello()                   const noexcept { return ::ftello(_fh);                 }
#endif

  inline int   seek(long offset, int whence) noexcept { return std::fseek(_fh, offset, whence); }
  inline long  tell()                  const noexcept { return std::ftell(_fh);                 }
  inline void  rewind()                      noexcept { return std::rewind(_fh);                }
  inline int   getpos(fpos_t *pos)     const noexcept { return std::fgetpos(_fh, pos);          }
  inline int   setpos(const fpos_t *pos)     noexcept { return std::fsetpos(_fh, pos);          }

  inline void  clearerr()           noexcept { return std::clearerr(_fh); }
  inline int   eof()          const noexcept { return std::feof(_fh);     }
  inline int   error()        const noexcept { return std::ferror(_fh);   }
#ifdef _POSIX_C_SOURCE
  inline int   fileno()       const noexcept { return ::fileno(_fh); }
#endif

#if /* Since glibc 2.24: */       _POSIX_C_SOURCE >= 199309L \
||  /* Glibc versions <= 2.23: */ _POSIX_C_SOURCE \
||  /* Glibc versions <= 2.19: */ _BSD_SOURCE || _SVID_SOURCE
  inline void lockfile()    noexcept { return ::flockfile(_fh);    }
  inline int  trylockfile() noexcept { return ::ftrylockfile(_fh); }
  inline void unlockfile()  noexcept { return ::funlockfile(_fh);  }
#endif


  inline int setvbuf(char *buf, int mode, size_t size) noexcept
  { return std::setvbuf(_fh, buf, mode, size); }

  inline int setbuf(char *buf) noexcept
  { return std::setvbuf(_fh, buf, buf ? _IOFBF : _IONBF, BUFSIZ); };

  inline int setbuffer(char *buf, size_t size) noexcept
  { return std::setvbuf(_fh, buf, buf ? _IOFBF : _IONBF, BUFSIZ); };

  inline int setlinebuf() noexcept
  { return std::setvbuf(_fh, NULL, _IOLBF, 0); }


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

  inline int vfprintf(const char *format, va_list ap) noexcept {
    return std::vfprintf(_fh, format, ap);
  }

  static CFile& stdin()  { return reinterpret_cast<CFile&>(::stdin);  };
  static CFile& stdout() { return reinterpret_cast<CFile&>(::stdout); };
  static CFile& stderr() { return reinterpret_cast<CFile&>(::stderr); };

private:
  FILE* _fh;
};
#endif
