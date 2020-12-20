#ifndef SHELLSPLIT_HPP
#define SHELLSPLIT_HPP

#include <string>
#include <vector>

namespace ShellSplit {

enum Warning {
  NONE,
  UNMATCHED_SINGLE_QUOTE, // "string
  UNMATCHED_DOUBLE_QUOTE, // 'string
  UNMATCHED_BACKSLASH     // string\ 
};

void split(const char*, std::vector<std::string>&, Warning &);

inline void split(const char* s, std::vector<std::string>& v) {
  Warning w;
  split(s, v, w);
}

} // namespace ShellSplit

#endif
