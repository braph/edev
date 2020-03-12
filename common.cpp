#include "common.hpp"

static union {
  wchar_t wide[512];
  char  narrow[sizeof(wide)];
} buffer;

char* toNarrowChar(wchar_t ch) {
  wctomb(buffer.narrow, ch);
  return buffer.narrow;
}

wchar_t* toWideString(CString s, size_t* len) {
  size_t _len = mbstowcs(buffer.wide, s, ARRAY_SIZE(buffer.wide));
  if (len)
    *len = _len;
  return buffer.wide;
}

char* toNarrowString(CWString s, size_t* len) {
  size_t _len = wcstombs(buffer.narrow, s, ARRAY_SIZE(buffer.narrow));
  if (len)
    *len = _len;
  return buffer.narrow;
}

char* time_format(time_t t, CString fmt) {
  struct tm* st = localtime(&t);
  strftime(buffer.narrow, ARRAY_SIZE(buffer.narrow), fmt, st);
  return buffer.narrow;
}

#ifdef TEST_COMMON
#endif
