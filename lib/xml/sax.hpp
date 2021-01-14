#ifndef LIB_XML_SAX_HPP
#define LIB_XML_SAX_HPP

#include <libxml/xmlstring.h>

#include <cstring>

namespace Xml {

const xmlChar empty[1] = {'\0'};

namespace Sax {

template<typename TClass>
static void wrap_startElement(void* self, const xmlChar* name, const xmlChar** attrs) {
  static_cast<TClass*>(self)->startElement(
      reinterpret_cast<const char*>(name),
      reinterpret_cast<const char**>(attrs));
}

template<typename TClass>
static void wrap_endElement(void* self, const xmlChar* name) {
  static_cast<TClass*>(self)->endElement(
      reinterpret_cast<const char*>(name));
}

template<typename TClass>
static void wrap_characters(void* self, const xmlChar* ch, int len) {
  static_cast<TClass*>(self)->characters(
      reinterpret_cast<const char*>(ch),
      len);
}

template<bool UseEmptyStringFallback>
class XmlString {
  const xmlChar* _s;
public:
  XmlString(const xmlChar* s)
    : _s(UseEmptyStringFallback ? (s ? s : empty) : s)
  {}

  explicit inline operator bool() const noexcept
  { return *_s; }

  inline bool operator!() const noexcept
  { return !*_s; }

  inline operator const char*() const noexcept
  { return reinterpret_cast<const char*>(_s); }

  inline operator const xmlChar*() const noexcept
  { return reinterpret_cast<const xmlChar*>(_s); }

  static inline int cmp(XmlString l, const char* r) noexcept {
    return std::strcmp(l, r);
  }

  static inline int cmp(XmlString l, const xmlChar* r) noexcept {
    return std::strcmp(l, reinterpret_cast<const char*>(r));
  }

  static inline int cmp(XmlString l, const std::string& r) noexcept {
    return std::strcmp(l, r.c_str());
  }

#define LIB_XML_DEFINE_COMPARISON(T) \
  friend inline bool operator==(T l, XmlString r) noexcept { return   (cmp(r, l) == 0); } \
  friend inline bool operator!=(T l, XmlString r) noexcept { return   (cmp(r, l) != 0); } \
  friend inline bool operator< (T l, XmlString r) noexcept { return   (cmp(l, r) <  0); } \
  friend inline bool operator<=(T l, XmlString r) noexcept { return   (cmp(l, r) <= 0); } \
  friend inline bool operator> (T l, XmlString r) noexcept { return   (cmp(l, r) >  0); } \
  friend inline bool operator>=(T l, XmlString r) noexcept { return   (cmp(l, r) >= 0); } \
  friend inline bool operator==(XmlString l, T r) noexcept { return   (cmp(l, r) == 0); } \
  friend inline bool operator!=(XmlString l, T r) noexcept { return   (cmp(l, r) != 0); } \
  friend inline bool operator< (XmlString l, T r) noexcept { return ! (cmp(r, l) <  0); } \
  friend inline bool operator<=(XmlString l, T r) noexcept { return ! (cmp(r, l) <= 0); } \
  friend inline bool operator> (XmlString l, T r) noexcept { return ! (cmp(r, l) >  0); } \
  friend inline bool operator>=(XmlString l, T r) noexcept { return ! (cmp(r, l) >= 0); }
  LIB_XML_DEFINE_COMPARISON(const char*)
  LIB_XML_DEFINE_COMPARISON(const xmlChar*)
  LIB_XML_DEFINE_COMPARISON(const std::string&)
//LIB_XML_DEFINE_COMPARISON(const XmlString)
#undef LIB_XML_DEFINE_COMPARISON

  inline bool operator=(char c) const noexcept {
    return *_s && *_s == c && _s[1] == '\0';
  }

#if 0
  inline bool starts_with(const char* rhs) {
    return _value && !std::strncmp(_value, rhs, std::strlen(rhs));
  }
#endif
};

struct Attribute {
  const XmlString<false> name;
  const XmlString<true> value;

  inline Attribute(const xmlChar* name_, const xmlChar* value_) noexcept
    : name(name_)
    , value(value_)
  {}
};

class Attributes {
  const xmlChar** _attrs;
public:
  Attributes(const xmlChar** attributes) noexcept : _attrs(attributes) {}

  Attribute get(const char* name) const noexcept {
    if (_attrs) {
      for (auto attrs = _attrs; *attrs; attrs += 2)
        if (! std::strcmp(reinterpret_cast<const char*>(*attrs), name))
          return Attribute(*attrs, *(attrs + 1));
    }
    return Attribute(NULL, NULL);
  }

  inline Attribute operator[](const char* name) const noexcept
  { return get(reinterpret_cast<const char*>(name)); }

  inline Attribute operator[](const xmlChar* name) const noexcept
  { return get(reinterpret_cast<const char*>(name)); }

  inline Attribute operator[](const std::string& name) const noexcept
  { return get(name.c_str()); }
};

} // namespace Sax

} // namespace Xml

#endif
