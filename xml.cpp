#include "xml.hpp"

//      throw std::invalid_argument("XML misses root element");
//      throw std::invalid_argument("XML has no children");

  //  xmlDoc  *m_doc = htmlReadFile("/tmp/ekt.html", NULL, 0);
  //HtmlDoc::readDoc("invalid document", NULL, NULL, XML_PARSE_COMPACT | XML_PARSE_NOERROR | XML_PARSE_NOWARNING);

#ifdef TEST_XML
#include <iostream>
#include <cassert>
#define NOT_REACHED assert(!"Not reached")
int main() {
  try {
    XmlDoc::readDoc("invalid document", NULL, NULL, XML_PARSE_COMPACT | XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
    NOT_REACHED;
  } catch (...) { /* OK */ }

  std::string xml = "<doc><foo bar='baz' rofl='lol'></foo></doc>";
  XmlDoc  doc = XmlDoc::readDoc(xml, NULL, NULL, XML_PARSE_COMPACT);
  XmlNode node = doc.getRootElement();

  for (node = node.children(); node; node = node.next()) {
    std::cout << "node type is" << node.type() << std::endl;

    std::cout << "lol:" << node["bar"] << std::endl;
    std::string bar = "bar";
    std::cout << "lol:" << node[bar] << std::endl;

    XmlAttribute attr = node.attributes();
    if (attr.valid()) {
      std::cout << attr.name() << "=" << attr.value() << std::endl;
    }
    attr = attr.next();

    if (attr.valid()) {
      std::cout << attr.name() << "=" << attr.value() << std::endl;
    }
  }

    //if (node->type != XML_ELEMENT_NODE)
    //  continue;
      //throw std::invalid_argument("THIS IS NOT A ELEMENT NODE");

    //if (node->children)
    //  throw std::invalid_argument("THIS NODE HAS CHILDREN");


  XmlDoc doc2 = XmlDoc::readHtmlFile("/tmp/ekt.html", NULL, 0);
  XmlXPath xpath = doc2.xpath();
  XmlXPathResult res = xpath.query("//span[contains(@class, 'pages')]");
  std::cout << res.size() << std::endl;
  XmlNode node2 = res[0];
  std::cout << node2.text() << std::endl;

  return 0;
}
#endif

