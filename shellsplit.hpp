#ifndef _SHELLSPLIT_HPP
#define _SHELLSPLIT_HPP

#include <string>
#include <vector>

namespace ShellSplit {

enum Warning {
  NONE,
  UNMATCHED_SINGLE_QUOTE, // "string
  UNMATCHED_DOUBLE_QUOTE, // 'string
  UNMATCHED_BACKSLASH     // string\ 
};

std::vector<std::string> split(const std::string &s, Warning &w);

inline std::vector<std::string> split(const std::string &s) {
  Warning w;
  return split(s, w);
}

} // namespace ShellSplit

#endif
