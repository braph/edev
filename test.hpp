// Stuff used for testing
#include <iostream>
#include <unistd.h>
#include <cassert>
#include <cstring>
#include "common.hpp"

#undef assert // May be defined in common.hpp

#define TEST_DB "/tmp/ektoplayer-test.db"

#define except(...) \
  try { __VA_ARGS__; throw 0; } \
  catch (int) { assert(!#__VA_ARGS__); } \
  catch (...) { /* OK */ }

#ifndef streq
#define streq(A,B) (!strcmp(A,B))
#endif

#define TEST_BEGIN() \
  try { (void)0
#define TEST_END() \
  } catch (const std::exception &e) { \
    std::cout << "Error: " << e.what() << std::endl; \
    return 1; \
  } \
  return 0;

#define NCURSES_INIT() \
  initscr(); \
  start_color(); \
  use_default_colors()

