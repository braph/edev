#ifndef CSTRING_HPP
#define CSTRING_HPP

#include <string>
#include <cstring>
#include <ctime>

struct CString {
  CString(const CString& s)     noexcept : _s(s._s) {}
  CString(const char* s)        noexcept : _s(s) {}
  CString(const std::string& s) noexcept : _s(s.c_str()) {}
  operator const char*() const  noexcept { return _s; }
private:
  const char* _s;
};

struct CWString {
  CWString(const CWString& s)     noexcept : _s(s._s) {}
  CWString(const wchar_t* s)      noexcept : _s(s) {}
  CWString(const std::wstring& s) noexcept : _s(s.c_str()) {}
  operator const wchar_t*() const noexcept { return _s; }
private:
  const wchar_t* _s;
};

char*    toNarrowChar(wchar_t);
wchar_t* toWideString(CString, size_t* len = NULL);
char*    toNarrowString(CWString, size_t* len = NULL);
char*    time_format(std::time_t, CString);

#endif
