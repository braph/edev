#include "cstring.hpp"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(A) (sizeof(A)/sizeof(*A))
#endif

// Maybe use std::string as buffer...
static union {
  wchar_t wide[512];
  char  narrow[sizeof(wide)];
} buffer;

char* toNarrowChar(wchar_t ch) {
  std::wctomb(buffer.narrow, ch);
  return buffer.narrow;
}

wchar_t* toWideString(CString s, size_t* len) {
  size_t _len = std::mbstowcs(buffer.wide, s, ARRAY_SIZE(buffer.wide));
  if (len)
    *len = _len;
  return buffer.wide;
}

char* toNarrowString(CWString s, size_t* len) {
  size_t _len = std::wcstombs(buffer.narrow, s, ARRAY_SIZE(buffer.narrow));
  if (len)
    *len = _len;
  return buffer.narrow;
}

char* time_format(std::time_t t, CString fmt) {
  std::tm* tm = std::localtime(&t);
  std::strftime(buffer.narrow, ARRAY_SIZE(buffer.narrow), fmt, tm);
  return buffer.narrow;
}

