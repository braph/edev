#include "xml.hpp"

// XML_PARSE_COMPACT | XML_PARSE_NOERROR | XML_PARSE_NOWARNING

#ifdef TEST_XML
#include "test.hpp"

using std::cout;
using std::endl;
static void errorHandler(void*, const char*, ...) {}
const std::string xml = "<doc><foo bar='baz' rofl='lol'><a>Text1</a> <a>Text2</a></foo></doc>";

int main() {
  TEST_BEGIN();
  xmlSetGenericErrorFunc(NULL, errorHandler);

  except( Xml::readDoc("invalid document", NULL, NULL, 0) );

  Xml::Doc  doc = Xml::readDoc(xml, NULL, NULL, XML_PARSE_COMPACT);
  Xml::Node node = doc.getRootElement();

  Xml::XPath xpath = doc.xpath();

  try { xpath.query("["); } catch (const Xml::Error& e) {
    cout << e.what() << e->code << endl;
  }

  for (node = node.children(); node; node = node.next()) {
    cout << "node type is" << node.type() << endl;

    cout << "lol:" << node["bar"] << endl;
    std::string bar = "bar";
    cout << "lol:" << node[bar] << endl;

    Xml::Attribute attr = node.attributes();
    if (attr.valid()) {
      cout << attr.name() << '=' << attr.value() << endl;
    }
    attr = attr.next();

    if (attr.valid()) {
      cout << attr.name() << '=' << attr.value() << endl;
    }
  }

  cout << "alltext=" << doc.getRootElement().allText() << "<" << endl;
  cout << "content=" << doc.getRootElement().nearestContent() << "<" << endl;

#if 0
  Xml::Doc doc2 = Html::readDoc("/tmp/ekt.html", NULL, 0);
  XmlXPath xpath = doc2.xpath();
  XmlXPathResult res = xpath.query("//span[contains(@class, 'pages')]");
  cout << res.size() << endl;
  XmlNode node2 = res[0];
  cout << node2.text() << endl;
#endif

  TEST_END();
}
#endif

