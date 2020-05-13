#ifndef TODO
#define TODO



/// FILE*
struct CFileIO {
  inline size_t fread(void *ptr, size_t size, size_t nmemb) { return ::fread(ptr, size, nmemb); }
  inline size_t fwrite(const void *ptr, size_t size, size_t nmemb) { return ::fwrite(ptr, size, nmemb); }

#ifdef __GNUC__
  __attribute__((format(printf, 2, 3)))
#endif

#ifdef __clang__
  __attribute__((__format__(__printf__, 2, 3)))
#endif


  inline int fprintf(const char *format, ...) {
    va_list ap;
    ::va_start(ap, format);
    int r = ::vfprintf(stderr, format, ap);
    ::va_end(ap);
    return r;
  }

  inline int vfprintf(const char *format, va_list ap) {
    return ::vfprintf(_fh, format, ap);
  }

  inline int clearerr() { return ::clearerr(_fh); }
  inline int fclose()   { return ::fclose(_fh); }
  inline int feof()     { return ::feof(_fh); }
  inline int ferror()   { return ::ferror(_fh); }
  inline int fflush() { return ::fflush(_fh); }
  inline int fgetc() { return ::fgetc(_fh); }
  inline int fgetpos(fpos_t *pos) { return ::fgetpos(_fh, pos); }
  inline char *fgets(char *s, int size) { return ::fgets(s, size, _fh); }

#ifdef _POSIX_C_SOURCE
  inline int fileno() { return ::fileno(_fh); }
#endif


#if /* Since glibc 2.24: */ _POSIX_C_SOURCE >= 199309L \
|| /* Glibc versions <= 2.23: */ _POSIX_C_SOURCE \
|| /* Glibc versions <= 2.19: */ _BSD_SOURCE || _SVID_SOURCE

  inline void flockfile() { ::flockfile(_fh); }
  inline int ftrylockfile() { ::ftrylockfile(_fh); }
  inline void funlockfile() { ::funlockfile(_fh); }

  int      fputc(int, FILE *);
  int      fputs(const char *restrict, FILE *restrict);
  size_t   fread(void *restrict, size_t, size_t, FILE *restrict);

  int      fscanf(FILE *restrict, const char *restrict, ...);
  int      fseek(FILE *, long, int);
  int      fseeko(FILE *, off_t, int);
  int      fsetpos(FILE *, const fpos_t *);
  long     ftell(FILE *);
  off_t    ftello(FILE *);
  int      ftrylockfile(FILE *);
  void     funlockfile(FILE *);
  size_t   fwrite(const void *restrict, size_t, size_t, FILE *restrict);
  int      fgetc(FILE *) { return getc(_fh); }





private:
  FILE* _fh;
};

/// FILE*
struct CFile : public CFileIO {
  FILE *fopen(const char *pathname, const char *mode);
  FILE *fdopen(int fd, const char *mode);
  FILE *freopen(const char *pathname, const char *mode, FILE *stream);
};

#endif


fh.close();
fclose(fh);

