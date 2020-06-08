// Stuff used for testing
#include <iostream>
#include <stdexcept>
#include <cstring>
#undef NDEBUG
#include <cassert>

#define except(...) \
  try { __VA_ARGS__; throw 0; } \
  catch (int) { assert(!#__VA_ARGS__); } \
  catch (...) { /* OK */ }

#define assert0(...) \
  if (! (__VA_ARGS__)) assert(!#__VA_ARGS__)

#ifndef streq
#define streq(A,B) (!std::strcmp(A,B))
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
  use_default_colors(); \
  noecho(); \
  curs_set(0)

