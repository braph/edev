#include "common.hpp"

wchar_t* toWideString(const char* s, size_t* len) {
  static wchar_t buf[1024];
  size_t _len = mbstowcs(buf, s, sizeof(buf));
  if (len)
    *len = _len;
  return buf;
}

#if TEST_COMMON
#include "test.hpp"

int main() {
}
#endif
