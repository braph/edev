#include "shellsplit.hpp"

#include <stdexcept>

std::vector<std::string> shellsplit(const std::string &s) {
  std::string word;
  std::vector<std::string> words;
  const char *it = s.c_str();

  for (;;) {
    while (isspace(*it)) { ++it;  } /* Skip whitespace */
    if (*it == '\0')     { break; } /* No more words   */

read_word:
    switch (*it) {
      case '\'':
        for (;;)
          switch (*++it) {
            case '\0':  throw std::invalid_argument("missing terminating single quote");
            case '\'':  ++it; goto read_word;
            default:    word += *it;
          }

      case '"':
        for (;;)
          switch (*++it) {
            case '\0':  throw std::invalid_argument("missing terminating double quote");
            case '"':   ++it; goto read_word;
            case '\\':  ++it; /* FALLTHROUGH */
            default:    word += *it;
          }

      case ' ':
      case '\t':
      case '\n':
      case '\0':
        break;

      case '\\': ++it; /* FALLTHROUGH */
      default:
        word += *it++;
        goto read_word;
    }

    words.push_back(word);
    word.clear();
  }

  return words;
}

#ifdef TEST_SHELLSPLIT
#include <cassert>
#include <iostream>

#define test(LINE, ...) \
  assert(shellsplit(LINE) == std::vector<std::string>(__VA_ARGS__))

void dump(const std::string &s) {
  std::vector<std::string> result = shellsplit(s);
  for (auto it = result.cbegin(); it != result.cend(); ++it)
    std::cout << "<<" << *it << ">>" << std::endl;
}

int main() {
  test("",        {});
  test(" \t",     {});
  test("1",       {"1"});
  test(" 1",      {"1"});
  test(" 1 ",     {"1"});
  test("1 2",     {"1", "2"});
  test("1 2 3",   {"1", "2", "3"});
  test("'1'",     {"1"});
  test("\"1\"",   {"1"});
  test("' 1 '",   {" 1 "});
  test("\" 1 \"", {" 1 "});
  // TODO: test escape; test nested quotes
  //dump("1 2");
}
#endif
