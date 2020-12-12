#ifndef LIB_BASE64_HPP
#define LIB_BASE64_HPP

#include <string>

namespace base64 {

std::string decode(const char*, size_t);

template<class T>
std::string decode(const T& s) { return decode(s.c_str(), s.size()); }

} // namespace base64

#endif
