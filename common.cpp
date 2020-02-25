#include "common.hpp"

static union {
  wchar_t wide[1024];
  char  narrow[sizeof(wide)];
} buffer;

wchar_t* toWideString(const char* s, size_t* len) {
  size_t _len = mbstowcs(buffer.wide, s, ARRAY_SIZE(buffer.wide));
  if (len)
    *len = _len;
  return buffer.wide;
}

char* toNarrowString(const wchar_t* s, size_t* len) {
  size_t _len = wcstombs(buffer.narrow, s, ARRAY_SIZE(buffer.narrow));
  if (len)
    *len = _len;
  return buffer.narrow;
}

#if TEST_COMMON
#include "test.hpp"

int main() {
}
#endif
