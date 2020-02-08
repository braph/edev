// Stuff used for testing
#include <iostream>
#include <unistd.h>
#include <cassert>

#define TEST_DB "/tmp/ektoplayer-test.db"

#define except(...) \
  try { __VA_ARGS__; throw 0; } \
  catch (int) { assert(!#__VA_ARGS__); } \
  catch (...) { /* OK */ }

#ifndef streq
#define streq(A,B) (!strcmp(A,B))
#endif

#define TEST_BEGIN \
  try {
#define TEST_END \
  } catch (const std::exception &e) { \
    std::cout << "Error: " << e.what() << std::endl; \
    return 1; \
  } \
  return 0;
