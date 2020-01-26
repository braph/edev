#ifndef _XML_HPP
#define _XML_HPP

#include <string>
#include <libxml/parser.h>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <libxml/xmlerror.h>
#include <iostream>

/* Very minimal OO interface to libxml2 */
// TODO: disable copy constructors
// TODO: error handling?

/*
 * - Provides both overloads: const char* AND std::string
 * - Provides iterators
 * - Function arguments are named as in original libxml
 */

// operators: Node.iter_trailing()
// operators: Node.iter_children()

class XmlAttribute {
  private:
    xmlAttr *m_attribute;
  public:
    XmlAttribute(xmlAttr *attribute)
    : m_attribute(attribute)
    {
      if (! (attribute && attribute->name && attribute->children))
        m_attribute = NULL;
    }

    inline bool valid()         const { return m_attribute; }
    inline operator bool()      const { return m_attribute; }
    inline operator xmlAttr*()  const { return m_attribute; }
    inline const char*  name()  const { return (const char*) m_attribute->name; }
    inline XmlAttribute next()  const { return XmlAttribute(m_attribute->next); }

    inline std::string value() const { 
      xmlChar* v = ::xmlNodeListGetString(m_attribute->doc, m_attribute->children, 1);
      std::string value = (const char*) v;
      ::xmlFree(v);
      return value;
    }
};

class XmlNode {
  private:
    xmlNode *m_node;
  public:
    inline XmlNode(xmlNode *node) : m_node(node)  {}
    inline operator     xmlNode*()   const { return m_node; }
    inline operator     bool()       const { return m_node; }
    inline bool         valid()      const { return m_node; }
    inline const char*  name()       const { return (const char*) m_node->name; }
    inline int          type()       const { return m_node->type; }
    inline XmlNode      next()       const { return XmlNode(m_node->next); }
    inline XmlNode      children()   const { return XmlNode(m_node->children); }
    inline XmlAttribute attributes() const { return XmlAttribute(m_node->properties); }
    inline std::string  text()       const { 
      std::string value;
      xmlChar* v = xmlNodeListGetString(m_node->doc, m_node->xmlChildrenNode, 1);
      if (v) {
        value = (const char*) v;
        xmlFree(v);
      }
      return value;
    }

    inline std::string dump(int level = 0, int format = 0) {
      xmlBufferPtr buf = xmlBufferCreate();
      xmlNodeDump(buf, m_node->doc, m_node, level, format);
      std::string result = (const char*) xmlBufferContent(buf);
      xmlBufferFree(buf);
      return result;
    }

    // Iterators
    /*
    inline bool operator==(const XmlNode&rhs) const { return m_node == rhs.m_node; }
    inline bool operator!=(const XmlNode&rhs) const { return m_node != rhs.m_node; }
    inline XmlNode operator++() { m_node = m_node->next; return *this; }
    inline XmlNode begin() noexcept { return XmlNode(m_xpathobject->nodesetval->nodeTab[0]); }
    inline XmlNode end()   noexcept { return XmlNode(NULL); }
    */

    // get Attribute
    inline const char* get_attribute(const char *name) const {
      return (const char*) ::xmlGetProp(m_node, (xmlChar*) name);
    }

    // set Attribute
    inline void set_attribute(const char* name, const char *value) const {
      xmlSetProp(m_node, (xmlChar*) name, (xmlChar*) value);
    }

    inline const char* operator[](const char* name) const        { return get_attribute(name); }
    inline const char* operator[](const std::string &name) const { return get_attribute(name.c_str()); }

    // set Attribute


    //inline void operator delete[](int) const { };
};

class XmlXPathResult {
  private:
    xmlXPathObjectPtr m_xpathobject;
  public:
    XmlXPathResult(xmlXPathObjectPtr xpathobject)
    : m_xpathobject(xpathobject)
    {}

    ~XmlXPathResult() {
      ::xmlXPathFreeObject(m_xpathobject);
    }

    XmlNode operator[](unsigned int i) const {
      return XmlNode( m_xpathobject->nodesetval->nodeTab[i] );
    }

    inline unsigned int size() const {
      if (xmlXPathNodeSetIsEmpty(m_xpathobject->nodesetval))
        return 0;
      return m_xpathobject->nodesetval->nodeNr;
    }

    // XmlXPathResult is wrong in this template instatioation
    class iterator : public std::iterator<std::forward_iterator_tag, XmlXPathResult> {
      private:
        xmlXPathObjectPtr m_xpathobject;
        unsigned int m_pos;
      public:
        iterator(xmlXPathObjectPtr ptr, unsigned int pos = 0)
        : m_xpathobject(ptr), m_pos(pos)
        { }

        XmlNode operator*() const { return XmlNode(m_xpathobject->nodesetval->nodeTab[m_pos]); }
        iterator operator++()     { ++m_pos; return *this; }
        bool operator!=(const iterator& rhs) { return m_pos != rhs.m_pos; }

    };

    inline iterator begin() noexcept { return iterator(m_xpathobject); }
    inline iterator end()   noexcept { return iterator(m_xpathobject, size()); }
};

class XmlXPath {
  private:
    xmlXPathContextPtr m_xpathcontext;
  public:
    XmlXPath(xmlDocPtr doc)
    : m_xpathcontext(::xmlXPathNewContext(doc)) {
    }

    ~XmlXPath() {
      ::xmlXPathFreeContext(m_xpathcontext);
    }

    inline XmlXPathResult query(const char *xpath) {
      xmlXPathObjectPtr _ = ::xmlXPathEvalExpression((xmlChar*) xpath, m_xpathcontext);
      //if (!_) { error; } XXX
      return XmlXPathResult(_);
    }

    inline XmlXPathResult query(const char *xpath, xmlNodePtr node) {
      xmlXPathObjectPtr _ = ::xmlXPathNodeEval(node, (xmlChar*) xpath, m_xpathcontext);
      //if (!_) { error; } XXX
      return XmlXPathResult(_);
    }

    inline const char* query_string(const char *xpath) {
      xmlXPathObjectPtr _ = ::xmlXPathEvalExpression((xmlChar*) xpath, m_xpathcontext);
      //if (!_) { error; }
      return (const char*) _->stringval;
    }

    inline const char* query_string(const char *xpath, xmlNodePtr node) {
      xmlXPathObjectPtr _ = ::xmlXPathNodeEval(node, (xmlChar*) xpath, m_xpathcontext);
      if (! _) {
        xmlErrorPtr e = xmlGetLastError();
        throw std::invalid_argument(e->str1);
        throw std::invalid_argument(e->message);
      }

      //XXX std::cout << _->type << std::endl;

      return (const char*) _->stringval;
    }

    // === std::string === //

    inline XmlXPathResult query(const std::string &xpath) const {
      return query(xpath.c_str());
    }

    inline XmlXPathResult query(const std::string &xpath, xmlNodePtr node) const {
      return query(xpath.c_str(), node);
    }
};

class XmlDoc {
  private:
    xmlDocPtr m_doc;
    //XmlDoc(const XmlDoc&);

  public:
    /*
    XmlDoc(const XmlDoc&& rhs) {
      xmlFreeDoc(m_doc);
      m_doc = rhs.m_doc;
    }
    */

    inline XmlDoc(xmlDocPtr doc) : m_doc(doc) { }

    inline ~XmlDoc() {
      ::xmlFreeDoc(m_doc);
      ::xmlCleanupParser();
    }

    inline operator xmlDoc*() const { return m_doc; }

    /* Wrappers for read-functions */
    static inline XmlDoc readDoc(const char *cur, const char *URL = NULL, const char *encoding = NULL, int options = 0) {
      xmlDocPtr doc = xmlReadDoc((const xmlChar*) cur, URL, encoding, options);
      if (! doc)
        throw std::invalid_argument("Could not parse XML");
      return XmlDoc(doc);
    }

    static inline XmlDoc readFile(const char *URL, const char *encoding = NULL, int options = 0) {
      xmlDocPtr doc = xmlReadFile(URL, encoding, options);
      if (! doc)
        throw std::invalid_argument("Could not parse XML");
      return XmlDoc(doc);
    }

    static inline XmlDoc readMemory(const char *buffer, int size, const char *URL, const char *encoding, int options = 0) {
      xmlDocPtr doc = xmlReadMemory(buffer, size, URL, encoding, options);
      if (! doc)
        throw std::invalid_argument("Could not parse XML");
      return XmlDoc(doc);
    }

    static inline XmlDoc readFd(int fd, const char *URL, const char *encoding, int options = 0) {
      xmlDocPtr doc = xmlReadFd(fd, URL, encoding, options);
      if (! doc)
        throw std::invalid_argument("Could not parse XML");
      return XmlDoc(doc);
    }
    /* End of wrappers */

    inline XmlNode getRootElement() const {
      return XmlNode(xmlDocGetRootElement(m_doc));
    }

    inline XmlXPath xpath() const {
      return XmlXPath(m_doc);
    }

    // std::string wrapper

    static inline XmlDoc readFile(const std::string& url, const char *encoding = NULL, int options = 0) {
      return readFile(url.c_str(), encoding, options);
    }

    static inline XmlDoc readDoc(const std::string& s, const char *url = NULL, const char *encoding = NULL, int options = 0) {
      return readDoc(s.c_str(), url, encoding, options);
    }
};

class HtmlDoc : public XmlDoc {
  public:
    static inline XmlDoc readFile(const char *url, const char *encoding = NULL, int options = 0) {
      xmlDocPtr doc = htmlReadFile(url, encoding, options);
      if (! doc)
        throw std::invalid_argument("Could not parse XML");
      return XmlDoc(doc);
    }

    static inline XmlDoc readFile(const std::string& s, const char *url = NULL, const char *encoding = NULL, int options = 0) {
      return readDoc(s.c_str(), url, encoding, options);
    }

    static inline XmlDoc readDoc(const std::string &cur, const char *URL = NULL, const char *encoding = NULL, int options = 0) {
      return readDoc(cur.c_str(), URL, encoding, options);
    }

    static inline XmlDoc readDoc(const char *cur, const char *URL = NULL, const char *encoding = NULL, int options = 0) {
      xmlDocPtr doc = htmlReadDoc((const xmlChar*) cur, URL, encoding, options);
      if (! doc)
        throw std::invalid_argument("Could not parse XML");
      return XmlDoc(doc);
    }
};

#endif
