#include "xml.hpp"
#include <iostream>
#include <string>
#include <cstring>
#define streq(A,B) (!strcmp(A,B))
//const char* s = "<p><strong>Afterglow</strong>AKA&#8217;s <a href=\"https://www.soundcloud.com/obri_gado\">Obri</a>, a musician from Tbilisi, Georgia. As a b-boy, rapper, and graffiti artist raised in one of Soviet-era suburbs in late 90s, <a href=\"https://www.facebook.com/obritech/\">Obri</a> began exploring breakbeat and other electronic music genres, but was soon immersed in psychedelic trance music. Alongside other enthusiasts he founded <a href=\"https://www.facebook.com/GeoPsyOfficial/\">GeoPsy</a>, a pioneering underground psychedelic community in Georgia, and soon moved into production. <!-- comm --> <br/> These ten tracks summarize his long personal journey into sound, experimenting with different elements of psychedelia from deep ambient techno soundscapes to <em>hypnotic</em> acid roughness. Artwork by <a href=\"https://www.facebook.com/Giopsyart/\">Digital Visionary Art of Giohorus</a>. Mastered at CES Studio.</p>";

const char* s = "<strong>Afterglow</strong>AKA&#8217;s";

//XML_ELEMENT_NODE=   1,
//XML_TEXT_NODE;

struct Parser {
  std::string buffer;

  inline void write(const std::string &s) {
    std::cout << "Writing: " << s << std::endl;
    for (auto c : s)
      std::cout << std::hex << (int)c << ' ';
    std::cout << std::endl;
    buffer += s;
  }

  inline void write(const char*s) {
    std::cout << "Writing: " << s << std::endl;
    buffer += s;
    while (*s) 
      std::cout << std::hex << (int)(*s++) << ' ';
    std::cout << std::endl;
  }

  void parse(const XmlNode& _node) {
    const char* s;
    XmlNode node = _node;

    for (; node; node = node.next()) {
          std::cout << "TEXT()" << std::endl;
          write(node.text());
      switch (node.type()) {
        case XML_ELEMENT_NODE:
          s = node.name();

          if (streq(s, "a")) {
            write("((");
            parse(node.children());
            write("))[[");
            write(node["href"]);
            write("]]");
          }
          else if (streq(s, "strong") || streq(s, "b")) {
            write("**");
            parse(node.children());
            write("**");
          }
          else if (streq(s, "em") || streq(s, "i")) {
            write("__");
            parse(node.children());
            write("__");
          }
          else if (streq(s, "br")) {
            write("\n");
          }
          else {
            parse(node.children());
          }

          break;

        case XML_TEXT_NODE:
          std::cout << "TEXT NODE" << std::endl;
          write(node.content());
          parse(node.children());

          break;

        default:
          std::cout << "TEXT()" << std::endl;
          write(node.text());
          parse(node.children());
      }
    }
  }
};

int main() {
  XmlDoc doc = HtmlDoc::readDoc(s, NULL, NULL, HTML_PARSE_RECOVER|HTML_PARSE_NOERROR|HTML_PARSE_NOWARNING|HTML_PARSE_COMPACT);
  XmlNode root = doc.getRootElement();

  Parser p;
  p.parse(root);
  std::cout << "RESULT:" << p.buffer << std::endl;

#if 0
  std::cout << buffer << std::endl;


  std::cout << root.dump() << std::endl;

  for (; root; root = root.next()) {
    for (XmlNode c = root.children(); c; c = c.next())
      std::cout << c.text() << std::endl;

  }
#endif
}

