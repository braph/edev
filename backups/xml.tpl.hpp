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

class XmlAttribute {
  private:
    xmlAttr *m_attribute;
    xmlDoc  *m_doc;
  public:
    XmlAttribute(xmlAttr *attribute, xmlDocPtr doc)
    : m_attribute(attribute)
    , m_doc(doc)
    {
      if (! (attribute && attribute->name && attribute->children))
        m_attribute = NULL;
    }

    inline bool valid()         const { return m_attribute; }
    inline operator bool()      const { return m_attribute; }
    inline const char*  name()  const { return (const char*) m_attribute->name; }
    inline XmlAttribute next()  const { return XmlAttribute(m_attribute->next, m_attribute->doc); }

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
    inline XmlAttribute attributes() const { return XmlAttribute(m_node->properties, m_node->doc); }
    inline std::string  text()       const { 
      xmlChar* v = xmlNodeListGetString(m_node->doc, m_node->xmlChildrenNode, 1);
      std::string value = (const char*) v;
      xmlFree(v);
      return value;
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

    unsigned int size() const {
      if (xmlXPathNodeSetIsEmpty(m_xpathobject->nodesetval))
        return 0;
      return m_xpathobject->nodesetval->nodeNr;
    }

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
    inline iterator end()   noexcept { return iterator(m_xpathobject, m_xpathobject->nodesetval->nodeNr); }
};

class XmlXPath {
  private:
    xmlXPathContextPtr m_xpathcontext;
  public:
    XmlXPath(xmlDocPtr doc) {
      m_xpathcontext = ::xmlXPathNewContext(doc);
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

      std::cout << _->type << std::endl;

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
<?php
  $none = new StdClass;

  function to_c($obj) {
    if ($obj === NULL)
      return "NULL";
    else if (is_int($obj))
      return "$obj";
    else
      return "\"$obj\"";
  }

  class Parameter {
    private $type, $name, $default;

    function __construct($type, $name, $default = NULL) {
      $this->type     = $type;
      $this->name     = $name;
      $this->default  = $default;
    }

    function hasDefault() {
      return $this->default !== NULL;
    }

    function decl($with_default = true) {
      $r = "$this->type $this->name";
      if ($with_default && $this->default !== NULL)
        $r .= " = $this->default";
      return $r;
    }

    function c_call() {
      return $this->name;
    }
  }

  class ConstChar extends Parameter {
    private $name, $default;

    function __construct($name, $default = NULL) {
      $this->name     = name;
      $this->default  = $default;
    }

    function decl($with_default = true) {
      if ($with_default && $this->default)
        return "const char* $this->name = $this->default";
      return "const char* $this->name";
    }

    function c_call() {
      return $this->name;
    }
  }

  class StdString extends Parameter {
    private $name, $default;

    function __construct ($name) {
      $this->name = $name;
    }

    function decl($with_default = true) {
      return "const std::string &$this->name";
    }

    function c_call() {
      return "$this->name.c_str()";
    }
  }

  function P($a,$b,$c=NULL) { return new Parameter($a,$b,$c); }
  function constChar($name, $default=NULL) { return new ConstChar($name, $default); }
  function stdString($name)                { return new StdString($name); }

  class Parameters {
    private $params;

    function __construct($params) {
      $this->params = $params;
    }

    function decl() {
      $params = array();
      $useDefault = true;

      foreach (array_reverse($this->params) as $i => $param) {
        array_unshift($params, $param->decl($useDefault));
        $useDefault = $param->hasDefault();
      }

      return join(", ", $params);
    }

    function c_call() {
      return join(", ", array_map( function($p) { return $p->c_call(); }, $this->params));
    }
  }

  function D() { return new Parameters(func_get_args()); }

  function permute($arrays) {
    //$arrays = func_get_args();

    $result = array(array());
    foreach ($arrays as $array) {
      $new_result = array();
      foreach ($result as $r) {
        foreach ($array as $element) {
          $copy = $r;
          $copy[] = $element;
          $new_result[] = $copy;
        }
      }
      $result = $new_result;
    }

    return $result;
  }

  /*
  $arr = array(
    D(P('const char*','cur'),     P('const char*','URL','NULL'),  P('const char*','encoding','NULL'), P('int','options','0')),

    // 1st arg std::string
    /*
    D(P('string','cur'),          P('string','URL'),              P('string',     'encoding'),        P('int','options','0')),
    D(P('string','cur'),          P('string','URL'),              P('const char*','encoding','NULL'), P('int','options','0')),
    D(P('string','cur'),          P('const char*','URL','NULL'),  P('const char*','encoding','NULL'), P('int','options','0')),
    D(P('string','cur'),          P('const char*','URL'),         P('string',     'encoding'),        P('int','options','0')),

    // 1st arg const char*

    D(P('string','cur'),          P('const char*','URL'),         P('string',     'encoding'),        P('int','options','0')),
  );
     */

  /*
  foreach (array(stdString('cur'), P('const char*', 'cur')) as $first) {
    foreach (array(stdString('URL'), P('const char*', 'URL', 'NULL')) as $second) {
      foreach (array(stdString('encoding'), P('const char*', 'encoding', 'NULL')) as $third) {
        $arr[] = D($first, $second, $third, P('int','options','0'));
      }
    }
  }
   */

  $overloaded_functions = array(
    "readDoc" => array(
      array(stdString('cur'),         P('const char*', 'cur')),
      array(stdString('URL'),         P('const char*', 'URL', 'NULL')),
      array(stdString('encoding'),    P('const char*', 'encoding', 'NULL')),
      array(P('int','options','0'))
    ),

    "readFile" => array(
      array(stdString('URL'),         P('const char*', 'URL')),
      array(stdString('encoding'),    P('const char*', 'encoding', 'NULL')),
      array(P('int','options','0'))
    ),

    "readMemory" => array(
      array(stdString('buffer'),      P('const char*', 'buffer')),
      array(P('int', 'size')),
      array(stdString('URL'),         P('const char*', 'url', 'NULL')),
      array(stdString('encoding'),    P('const char*', 'encoding', 'NULL')),
      array(P('int', 'options', '0'))
    ),

    "readFd" => array(
      array(P('int', 'fd')),
      array(stdString('URL'),         P('const char*', 'URL', 'NULL')),
      array(stdString('encoding'),    P('const char*', 'encoding', 'NULL')),
      array(P('int', 'options', '0'))
    ),

    "readIO" => array(
      array(P('xmlInputReadCallback', 'ioread')),
      array(P('xmlInputCloseCallback', 'ioclose')),
      array(P('void*', 'ioctx')),
      array(stdString('URL'),         P('const char*', 'URL', 'NULL')),
      array(stdString('encoding'),    P('const char*', 'encoding', 'NULL')),
      array(P('int', 'options', '0'))
    )
  );

  $lol = array();

  foreach ($overloaded_functions as $func => $parameters) {
    $lol[$func] = array();
    foreach (permute($parameters) as $permutated_parameters) {
      $lol[$func][] = new Parameters($permutated_parameters);
    }
  }

  $overloaded_functions = $lol;

  //print_r($overloaded_functions);

?>
    static inline XmlDoc readDoc(TODO) {
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

    // Auto generated overloaded stuff ...
<?php foreach ($overloaded_functions as $func => $declarations) {
        foreach ($declarations as $d) { ?>
    static inline XmlDoc <?=$func?>(<?= $d->decl(); ?>) {
      return <?=$func?>(<?= $d->c_call(); ?>);
    }
<?php }} ?>
};

class HtmlDoc : public XmlDoc {
  public:
    static inline XmlDoc readFile(const char *url, const char *encoding = NULL, int options = 0) {
      xmlDocPtr doc = htmlReadFile(url, encoding, options);
      if (! doc)
        throw std::invalid_argument("Could not parse XML");
      return XmlDoc(doc);
    }

    // Auto generated overloaded stuff ...
<?php foreach ($overloaded_functions as $func => $declarations) {
        foreach ($declarations as $d) { ?>
    static inline XmlDoc <?=$func?>(<?= $d->decl(); ?>) {
      return <?=$func?>(<?= $d->c_call(); ?>);
    }
<?php }} ?>
};

#endif
