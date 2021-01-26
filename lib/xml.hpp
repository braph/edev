#ifndef XML_HPP
#define XML_HPP

#include <libxml/parser.h>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <libxml/xmlerror.h>

#include <memory>
#include <string>
#include <stdexcept>

/* Very minimal and broken OO interface to libxml2 - do not use! :D */

// TODO: disable copy constructors etc..
// TODO: Node.iter_trailing()
// TODO: Node.iter_children()

namespace Xml {

/**
 * Used for *INPUT* parameters
 */
struct String {
  String(const String& s)         noexcept : _s(s._s) {}
  String(const char* s)           noexcept : _s(s) {}
  String(const std::string& s)    noexcept : _s(s.c_str()) {}
  operator const char*()    const noexcept { return _s; }
  operator const xmlChar*() const noexcept { return reinterpret_cast<const xmlChar*>(_s); }
private:
  const char* _s;
};

struct Error : public std::exception {
  Error(xmlErrorPtr e) : _e(e) {}
 ~Error() { xmlResetError(_e); }
  const char* what()       const noexcept { return _e->message; }
  operator xmlErrorPtr()   const noexcept { return _e; }
  xmlErrorPtr operator->() const noexcept { return _e; }
private:
  xmlErrorPtr _e;
};

static void xmlGenericFree(xmlXPathObjectPtr p)  { ::xmlXPathFreeObject(p); }
static void xmlGenericFree(xmlNodePtr p)         { ::xmlFreeNode(p); }
static void xmlGenericFree(xmlDocPtr p)          { ::xmlFreeDoc(p);  }
static void xmlGenericFree(xmlXPathContextPtr p) { ::xmlXPathFreeContext(p); }
static void xmlGenericFree(xmlBufferPtr p)       { ::xmlBufferFree(p); }
static void xmlGenericFree(xmlAttrPtr p)         { ::xmlFreeProp(p); }

template<class PtrType, bool Owning = true>
struct XmlObject {
  PtrType _obj;
  XmlObject(PtrType p) : _obj(p) {}
 ~XmlObject() { if (Owning) { xmlGenericFree(_obj); } }

  operator PtrType()          const noexcept { return _obj; }
  bool              valid()   const noexcept { return _obj; }
  explicit operator bool()    const noexcept { return _obj; }
};

#if 0
struct Result {
  Result(xmlChar* s) : _s(s) {}
 ~Result() { xmlFree(_s); }
  operator std::string() { return
    std::string(reinterpret_cast<char*>(_s))
  }

private:
  xmlChar* _s;
};
#endif

struct XPathResultString : public XmlObject<xmlXPathObjectPtr> {
  using XmlObject::XmlObject;

  operator bool() { return _obj->stringval; }

  char* c_str() {
    return reinterpret_cast<char*>(_obj->stringval);
  }

  std::string str() { return std::string(*this); }

  operator std::string() {
    std::string value;
    if (_obj->stringval)
      value = reinterpret_cast<const char*>(_obj->stringval);
    return value;
  }
};

/* Object is bound to the lifetime of the Node that returned it. */
struct Attribute : public XmlObject<xmlAttrPtr, false> {
  using XmlObject::XmlObject;

  Attribute(xmlAttrPtr p) : XmlObject(p) {
    if (! (p && p->name && p->children))
      _obj = NULL;
  }

  const char* name()            const noexcept { return reinterpret_cast<const char*>(_obj->name); }
  Attribute   next()            const noexcept { return Attribute(_obj->next); }

  std::string value() const {
    xmlChar* _ = ::xmlNodeListGetString(_obj->doc, _obj->children, 1);
    std::string value = reinterpret_cast<const char*>(_);
    ::xmlFree(_);
    return value;
  }
};

struct Buffer : public XmlObject<xmlBufferPtr> {
  using XmlObject::XmlObject;
  static Buffer create() { return Buffer(::xmlBufferCreate()); }

  const char* content() const noexcept { return reinterpret_cast<const char*>(::xmlBufferContent(_obj)); }
};

/* Object is bound to the lifetime of the Doc that returned it. */
struct Node : public XmlObject<xmlNodePtr, false> {
  using XmlObject::XmlObject;

  const char*  name()          const noexcept { return reinterpret_cast<const char*>(_obj->name);    }
  const char*  content()       const noexcept { return reinterpret_cast<const char*>(_obj->content); }
  int          type()          const noexcept { return _obj->type;               }
  Node         next()          const noexcept { return Node(_obj->next);      }
  Node         children()      const noexcept { return Node(_obj->children);  }
  Attribute    attributes()    const noexcept { return Attribute(_obj->properties); }

  // <currentNode><otherTags>bar</otherTags></currentNode>
  //   ^--- nearest_content() -^
  const char* nearest_content() const noexcept {
    const char* s = NULL;

    for (Node node = children(); node; node = node.next())
      if ((s = node.content()))
        return s;
      else if ((s = node.nearest_content()))
        return s;

    return s;
  }

#if 0 // xmlNodeListGetString() == (_obj->content + overhead) ?!?!
  std::string text() const {
    std::string value;
    xmlChar* _ = ::xmlNodeListGetString(_obj->doc, _obj->xmlChildrenNode, 1);
    if (_) {
      value = reinterpret_cast<const char*>(_);
      ::xmlFree(_);
    }
    return value;
  }
#endif

  std::string all_text() const {
    std::string value;
    for (Node node = children(); node; node = node.next())
      if (node.type() == XML_TEXT_NODE) {
        if (node.content())
          value += node.content();
      }
      else
        value += node.all_text();
    return value;
  }

  std::string dump(int level = 0, int format = 0) const {
    auto buf = Buffer::create();
    ::xmlNodeDump(buf, _obj->doc, _obj, level, format);
    return buf.content();
  }

  std::string get_attribute(String name) const {
    std::string value;
    xmlChar* _ = ::xmlGetProp(_obj, name);
    if (_) {
      value = reinterpret_cast<const char*>(_);
      ::xmlFree(_);
    }
    return value;
  }

  void set_attribute(String name, String value) const
  { ::xmlSetProp(_obj, name, value); }

  std::string operator[](String name) const
  { return get_attribute(name); }


  // Iterators
  /*
  bool operator==(const Node&rhs) const { return _obj == rhs._obj; }
  bool operator!=(const Node&rhs) const { return _obj != rhs._obj; }
  Node operator++() { _obj = _obj->next; return *this; }
  Node begin() noexcept { return Node(m_xpathobject->nodesetval->nodeTab[0]); }
  Node end()   noexcept { return Node(NULL); }
  */
};

struct XPathResult : public XmlObject<xmlXPathObjectPtr> {
  using XmlObject::XmlObject;

  Node operator[](int i) const {
    return Node( _obj->nodesetval->nodeTab[i] );
  }

  int size() const {
    if (xmlXPathNodeSetIsEmpty(_obj->nodesetval))
      return 0;
    return _obj->nodesetval->nodeNr;
  }

  struct iterator {
  using iterator_category = std::forward_iterator_tag;
  using difference_type   = std::ptrdiff_t;

  private:
    xmlXPathObjectPtr _obj;
    int m_pos;
  public:
    iterator(xmlXPathObjectPtr ptr, int pos = 0)
    : _obj(ptr), m_pos(pos)
    { }

    Node operator*() const noexcept { return Node(_obj->nodesetval->nodeTab[m_pos]); }
    iterator operator++()                      noexcept { ++m_pos; return *this;     }
    bool operator!=(const iterator& rhs) const noexcept { return m_pos != rhs.m_pos; }
    bool operator==(const iterator& rhs) const noexcept { return m_pos == rhs.m_pos; }
  };

  iterator begin() noexcept { return iterator(_obj); }
  iterator end()   noexcept { return iterator(_obj, size()); }
};

struct XPath : public XmlObject<xmlXPathContextPtr> {
  using XmlObject::XmlObject;

  XPathResult query(String xpath) {
    return query(xpath, _obj->doc->children);
  }

  XPathResult query(String xpath, Node node) {
    xmlXPathObjectPtr _ = ::xmlXPathNodeEval(xmlNodePtr(node), xpath, _obj);
    if (!_) throw Error(xmlGetLastError());
    return XPathResult(_);
  }

  XPathResult query(xmlXPathCompExprPtr expr) {
    return query(expr, _obj->doc->children);
  }

  XPathResult query(xmlXPathCompExprPtr expr, Node node) {
    ::xmlXPathSetContextNode(xmlNodePtr(node), _obj);
    xmlXPathObjectPtr _ = ::xmlXPathCompiledEval(expr, _obj);
    if (!_) throw Error(xmlGetLastError());
    return XPathResult(_);
  }

  // ===========================================================================

  XPathResultString query_string(String xpath) {
    return query_string(xpath, _obj->doc->children);
  }

  XPathResultString query_string(String xpath, Node node) {
    xmlXPathObjectPtr _ = ::xmlXPathNodeEval(xmlNodePtr(node), xpath, _obj);
    if (!_) throw Error(xmlGetLastError());
    return XPathResultString(_);
  }

  XPathResultString query_string(xmlXPathCompExprPtr expr) {
    return query_string(expr, _obj->doc->children);
  }

  XPathResultString query_string(xmlXPathCompExprPtr expr, Node node) {
    ::xmlXPathSetContextNode(xmlNodePtr(node), _obj);
    xmlXPathObjectPtr _ = ::xmlXPathCompiledEval(expr, _obj);
    if (!_) throw Error(xmlGetLastError());
    return XPathResultString(_);
  }
};

struct Doc : public XmlObject<xmlDocPtr> {
  using XmlObject::XmlObject;

  /*
  Doc(const Doc&& rhs) {
    ::xmlFreeDoc(_obj);
    _obj = rhs._obj;
  }
  */

  operator xmlDoc*() const { return _obj; }

  Node getRootElement() const {
    return Node(::xmlDocGetRootElement(_obj));
  }

  XPath xpath() const {
    return XPath(::xmlXPathNewContext(_obj));
  }
};

static Doc readDoc(const char* cur, String URL = NULL, String encoding = NULL, int options = 0) {
  xmlDocPtr doc = ::xmlReadDoc(reinterpret_cast<const xmlChar*>(cur), URL, encoding, options);
  if (! doc)
    throw Error(xmlGetLastError());
  return Doc(doc);
}

// Special overload for std::string making use of size() XXX
static Doc readDoc(const std::string& cur, String URL = NULL, String encoding = NULL, int options = 0) {
  xmlDocPtr doc = ::xmlReadMemory(cur.c_str(), int(cur.size()), URL, encoding, options);
  if (! doc)
    throw Error(xmlGetLastError());
  return Doc(doc);
}

static Doc readMemory(String buffer, int size, String URL, String encoding, int options = 0) {
  xmlDocPtr doc = ::xmlReadMemory(buffer, size, URL, encoding, options);
  if (! doc)
    throw Error(xmlGetLastError());
  return Doc(doc);
}

static Doc readFile(String URL, String encoding = NULL, int options = 0) {
  xmlDocPtr doc = ::xmlReadFile(URL, encoding, options);
  if (! doc)
    throw Error(xmlGetLastError());
  return Doc(doc);
}

static Doc readFd(int fd, String URL, String encoding, int options = 0) {
  xmlDocPtr doc = ::xmlReadFd(fd, URL, encoding, options);
  if (! doc)
    throw Error(xmlGetLastError());
  return Doc(doc);
}

} // namespace Xml

namespace Html {

using Xml::Doc;
using Xml::String;
using Xml::Error;

static Doc readDoc(const char* cur, String URL = NULL, String encoding = NULL, int options = 0) {
  xmlDocPtr doc = ::htmlReadDoc(reinterpret_cast<const xmlChar*>(cur), URL, encoding, options);
  if (! doc)
    throw Error(xmlGetLastError());
  return Doc(doc);
}

// Special overload for std::string making use of size() XXX?
static Doc readDoc(const std::string& cur, String URL = NULL, String encoding = NULL, int options = 0) {
  xmlDocPtr doc = ::htmlReadMemory(cur.c_str(), int(cur.size()), URL, encoding, options);
  if (! doc)
    throw Error(xmlGetLastError());
  return Doc(doc);
}

static Doc readMemory(String buffer, int size, String URL, String encoding, int options = 0) {
  xmlDocPtr doc = ::htmlReadMemory(buffer, size, URL, encoding, options);
  if (! doc)
    throw Error(xmlGetLastError());
  return Doc(doc);
}

static Doc readFile(String url, String encoding = NULL, int options = 0) {
  xmlDocPtr doc = ::htmlReadFile(url, encoding, options);
  if (! doc)
    throw Error(xmlGetLastError());
  return Doc(doc);
}

static Doc readFd(int fd, String URL, String encoding, int options = 0) {
  xmlDocPtr doc = ::htmlReadFd(fd, URL, encoding, options);
  if (! doc)
    throw Error(xmlGetLastError());
  return Doc(doc);
}

} // namespace Html

#endif
