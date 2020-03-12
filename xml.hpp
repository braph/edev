#ifndef _XML_HPP
#define _XML_HPP

#include <libxml/parser.h>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <libxml/xmlerror.h>

#include <string>
#include <iostream>
#include <stdexcept>

/* Very minimal OO interface to libxml2 - do not use! :D
 * - Function arguments are named as in original libxml
 * - ::xmlCleanupParser() has to be called
 */

// TODO: disable copy constructors etc..
// TODO: Node.iter_trailing()
// TODO: Node.iter_children()

namespace Xml {

/* Used for INPUT parameters that take `const char*` */
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

/* Object is bound to the lifetime of the Node that returned it. */
class Attribute {
private:
  xmlAttr *m_attr;
public:
  Attribute(xmlAttr *attribute)
  : m_attr(attribute)
  {
    if (! (attribute && attribute->name && attribute->children))
      m_attr = NULL;
  }

  bool valid()                  const { return m_attr; }
  explicit operator bool()      const { return m_attr; }
  explicit operator xmlAttr*()  const { return m_attr; }
  const char* name()            const { return reinterpret_cast<const char*>(m_attr->name); }
  Attribute   next()            const { return Attribute(m_attr->next); }

  std::string value() const { 
    xmlChar* _ = ::xmlNodeListGetString(m_attr->doc, m_attr->children, 1);
    std::string value = reinterpret_cast<const char*>(_);
    ::xmlFree(_);
    return value;
  }
};

/* Object is bound to the lifetime of the Doc that returned it. */
class Node {
private:
  xmlNode *m_node;
public:
  Node(xmlNode *node) : m_node(node) {}
  explicit operator xmlNode*() const { return m_node; }
  explicit operator bool()     const { return m_node; }
  bool         valid()         const { return m_node; }
  const char*  name()          const { return reinterpret_cast<const char*>(m_node->name);    }
  const char*  content()       const { return reinterpret_cast<const char*>(m_node->content); }
  int          type()          const { return m_node->type;               }
  Node         next()          const { return Node(m_node->next);      }
  Node         children()      const { return Node(m_node->children);  }
  Attribute    attributes()    const { return Attribute(m_node->properties); }

  // <currentNode><otherTags>bar</otherTags></currentNode>
  //   ^--- nearestContent() -^
  const char* nearestContent() const {
    const char* s = NULL;

    for (Node node = children(); node; node = node.next())
      if ((s = node.content()))
        return s;
      else if ((s = node.nearestContent()))
        return s;

    return s;
  }

#if 0 // xmlNodeListGetString() == (m_node->content + overhead) ?!?!
  std::string text() const {
    std::string value;
    xmlChar* _ = ::xmlNodeListGetString(m_node->doc, m_node->xmlChildrenNode, 1);
    if (_) {
      value = reinterpret_cast<const char*>(_);
      ::xmlFree(_);
    }
    return value;
  }
#endif

  std::string allText() const {
    std::string value;
    for (Node node = children(); node; node = node.next())
      if (node.type() == XML_TEXT_NODE) {
        if (node.content())
          value += node.content();
      }
      else
        value += node.allText();
    return value;
  }

  std::string dump(int level = 0, int format = 0) const {
    xmlBufferPtr buf = ::xmlBufferCreate();
    ::xmlNodeDump(buf, m_node->doc, m_node, level, format);
    std::string result = reinterpret_cast<const char*>(::xmlBufferContent(buf));
    ::xmlBufferFree(buf);
    return result;
  }

  std::string get_attribute(String name) const {
    std::string value;
    xmlChar* _ = ::xmlGetProp(m_node, name);
    if (_) {
      value = reinterpret_cast<const char*>(_);
      ::xmlFree(_);
    }
    return value;
  }

  void set_attribute(String name, String value) const
  { ::xmlSetProp(m_node, name, value); }

  std::string operator[](String name) const
  { return get_attribute(name); }


  // Iterators
  /*
  bool operator==(const Node&rhs) const { return m_node == rhs.m_node; }
  bool operator!=(const Node&rhs) const { return m_node != rhs.m_node; }
  Node operator++() { m_node = m_node->next; return *this; }
  Node begin() noexcept { return Node(m_xpathobject->nodesetval->nodeTab[0]); }
  Node end()   noexcept { return Node(NULL); }
  */
};

class XPathResult {
private:
  xmlXPathObjectPtr m_xpathobject;
public:
  XPathResult(xmlXPathObjectPtr xpathobject)
  : m_xpathobject(xpathobject)
  {}

  ~XPathResult() {
    ::xmlXPathFreeObject(m_xpathobject);
  }

  Node operator[](int i) const {
    return Node( m_xpathobject->nodesetval->nodeTab[i] );
  }

  int size() const {
    if (xmlXPathNodeSetIsEmpty(m_xpathobject->nodesetval))
      return 0;
    return m_xpathobject->nodesetval->nodeNr;
  }

  // XPathResult is wrong in this template XXX
  class iterator : public std::iterator<std::forward_iterator_tag, XPathResult> {
    private:
      xmlXPathObjectPtr m_xpathobject;
      int m_pos;
    public:
      iterator(xmlXPathObjectPtr ptr, int pos = 0)
      : m_xpathobject(ptr), m_pos(pos)
      { }

      Node operator*() const { return Node(m_xpathobject->nodesetval->nodeTab[m_pos]); }
      iterator operator++()     { ++m_pos; return *this; }
      bool operator!=(const iterator& rhs) const { return m_pos != rhs.m_pos; }
  };

  iterator begin() noexcept { return iterator(m_xpathobject); }
  iterator end()   noexcept { return iterator(m_xpathobject, size()); }
};

class XPath {
private:
  xmlXPathContextPtr m_xpathcontext;
public:
  XPath(xmlDocPtr doc)
  : m_xpathcontext(::xmlXPathNewContext(doc)) {
  }

  ~XPath() {
    ::xmlXPathFreeContext(m_xpathcontext);
  }

  XPathResult query(String xpath) {
    xmlXPathObjectPtr _ = ::xmlXPathEvalExpression(xpath, m_xpathcontext);
    if (!_) throw Error(xmlGetLastError());
    return XPathResult(_);
  }

  XPathResult query(String xpath, Node node) {
    xmlXPathObjectPtr _ = ::xmlXPathNodeEval(xmlNodePtr(node), xpath, m_xpathcontext);
    if (!_) throw Error(xmlGetLastError());
    return XPathResult(_);
  }

  std::string query_string(String xpath) {
    std::string value;
    xmlXPathObjectPtr _ = ::xmlXPathEvalExpression(xpath, m_xpathcontext);
    if (!_) throw Error(xmlGetLastError());
    if (_->stringval)
      value = reinterpret_cast<const char*>(_->stringval);
    ::xmlXPathFreeObject(_);
    return value;
  }

  std::string query_string(String xpath, Node node) {
    std::string value;
    xmlXPathObjectPtr _ = ::xmlXPathNodeEval(xmlNodePtr(node), xpath, m_xpathcontext);
    if (!_) throw Error(xmlGetLastError());
    if (_->stringval)
      value = reinterpret_cast<const char*>(_->stringval);
    ::xmlXPathFreeObject(_);
    return value;
  }
};

class Doc {
private:
  xmlDocPtr m_doc;
  //Doc(const Doc&);

public:
  /*
  Doc(const Doc&& rhs) {
    ::xmlFreeDoc(m_doc);
    m_doc = rhs.m_doc;
  }
  */

  Doc(xmlDocPtr doc) : m_doc(doc) { }

  ~Doc() {
    ::xmlFreeDoc(m_doc);
  }

  operator xmlDoc*() const { return m_doc; }

  Node getRootElement() const {
    return Node(::xmlDocGetRootElement(m_doc));
  }

  XPath xpath() const {
    return XPath(m_doc);
  }
};

static Doc readDoc(String cur, String URL = NULL, String encoding = NULL, int options = 0) {
  xmlDocPtr doc = ::xmlReadDoc(cur, URL, encoding, options);
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

static Doc readMemory(String buffer, int size, String URL, String encoding, int options = 0) {
  xmlDocPtr doc = ::xmlReadMemory(buffer, size, URL, encoding, options);
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

static Doc readFile(String url, String encoding = NULL, int options = 0) {
  xmlDocPtr doc = ::htmlReadFile(url, encoding, options);
  if (! doc)
    throw Error(xmlGetLastError());
  return Doc(doc);
}

static Doc readDoc(String cur, String URL = NULL, String encoding = NULL, int options = 0) {
  xmlDocPtr doc = ::htmlReadDoc(cur, URL, encoding, options);
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

static Doc readFd(int fd, String URL, String encoding, int options = 0) {
  xmlDocPtr doc = ::htmlReadFd(fd, URL, encoding, options);
  if (! doc)
    throw Error(xmlGetLastError());
  return Doc(doc);
}

} // namespace Html

#endif
