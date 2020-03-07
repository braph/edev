#include "shellsplit.hpp"

namespace ShellSplit {

static inline void
readSingleQuoted(const char*& s, std::string& word, Warning& w)
{
  while (*++s)
    if (*s == '\'')
      return;
    else
      word.push_back(*s);
  w = UNMATCHED_SINGLE_QUOTE;
}

static inline void
readDoubleQuoted(const char*& s, std::string& word, Warning& w)
{
  bool escaped = false;
  while (*++s)
    if (escaped) {
      word.push_back(*s);
      escaped = false;
    } else if (*s == '"')
      return;
    else if (*s == '\\')
      escaped = true;
    else
      word.push_back(*s);
  w = UNMATCHED_DOUBLE_QUOTE;
}

std::vector<std::string>
split(const std::string& s, Warning& w)
{
  std::string word;
  std::vector<std::string> words;
  const char* it = s.c_str();
  bool havingWord = false;
  bool escaped = false;

  while (*it) {
    if (escaped) {
      escaped = false;
      word.push_back(*it);
    } else {
      switch (*it) {
        case '\'':
          havingWord = true;
          readSingleQuoted(it, word, w);
          break;
        case '"':
          havingWord = true;
          readDoubleQuoted(it, word, w);
          break;
        case ' ':
        case '\t':
        case '\n':
        case '\r':
        case '\v':
        case '\f':
          if (havingWord) {
            havingWord = false;
            words.push_back(std::move(word));
          }
          break;

        case '\\':
          escaped = true;
          break;

        default:
          havingWord = true;
          word.push_back(*it);
      }
    }

    ++it;
  }

  if (havingWord)
    words.push_back(std::move(word));

  if (escaped)
    w = UNMATCHED_BACKSLASH;

  return words;
}

} // ShellSplit

#ifdef TEST_SHELLSPLIT
#include "test.hpp"

#define test(LINE, ...) \
  assert(ShellSplit::split(LINE) == std::vector<std::string>(__VA_ARGS__))

void dump(const std::string &s) {
  std::vector<std::string> result = ShellSplit::split(s);
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
  test("'a''b'",  {"ab"});
  test("\"a\'\"", {"a\'"});
  test("\'\\\'",  {"\\"});
  // TODO: test escape; test nested quotes
  //dump("1 2");
}
#endif
