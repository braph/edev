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

void split(const char* s, std::vector<std::string>& result, Warning& w)
{
  result.clear();
  std::string word;
  bool havingWord = false;
  bool escaped = false;

  while (*s) {
    if (escaped) {
      escaped = false;
      word.push_back(*s);
    } else {
      switch (*s) {
        case '\'':
          havingWord = true;
          readSingleQuoted(s, word, w);
          break;
        case '"':
          havingWord = true;
          readDoubleQuoted(s, word, w);
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
            word.clear();
          }
          break;

        case '\\':
          escaped = true;
          break;

        default:
          havingWord = true;
          word.push_back(*s);
      }
    }

    ++s;
  }

  if (havingWord)
    result.push_back(std::move(word));

  if (escaped)
    w = UNMATCHED_BACKSLASH;
}

} // ShellSplit
