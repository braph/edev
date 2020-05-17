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

void split(const std::string& s, std::vector<std::string>& result, Warning& w)
{
  result.clear();
  std::string word;
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
            result.push_back(std::move(word));
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
    result.push_back(std::move(word));

  if (escaped)
    w = UNMATCHED_BACKSLASH;
}

} // ShellSplit

#ifdef TEST_SHELLSPLIT
#include "test.hpp"

static std::vector<std::string> result; // Global (for checking if .clear works)

static void test(const char* line, const std::vector<std::string>& expected) {
  ShellSplit::split(line, result);
  if (result != expected) {
    std::cout << "Input: " << line << "\nResult: ";
    for (const auto& e : result)
      std::cout << '"' << e << "\",";
    std::cout << '\n';
    throw;
  }
}

int main() {
  TEST_BEGIN();
  test("",          {});
  test(" \t",       {});
  test("1",         {"1"});
  test(" 1",        {"1"});
  test(" 1 ",       {"1"});
  test("1 2",       {"1", "2"});
  test("1 2 3",     {"1", "2", "3"});
  test("'1'",       {"1"});
  test("\"1\"",     {"1"});
  test("' 1 '",     {" 1 "});
  test("\" 1 \"",   {" 1 "});
  test("'a''b'",    {"ab"});
  test("\"a\'\"",   {"a\'"});
  test("\'\\\'",    {"\\"});
  test("\"\\\\\"",  {"\\"});
  test("'\"'",      {"\""});
  test("\"'\"",     {"'"});
  TEST_END();
}
#endif