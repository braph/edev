
struct Html2Markup {
  std::string result;

  static std::string convert(const std::string& html, const char* encoding) {
    Xml::Doc doc = Html::readDoc(html, NULL, encoding,
        HTML_PARSE_RECOVER|HTML_PARSE_NOERROR|HTML_PARSE_NOWARNING|
        HTML_PARSE_COMPACT|HTML_PARSE_NOBLANKS|HTML_PARSE_NOIMPLIED);
    Xml::Node root = doc.getRootElement();
    Html2Markup p;
    p.result.reserve(html.size() / 4);
    p.parse(root);
    return p.result;
  }

  void parse(const Xml::Node& _node) {
    const char* tag;
    Xml::Node node = _node;

    for (; node; node = node.next()) {
      switch (node.type()) {
        case XML_ELEMENT_NODE:
          tag = node.name();

          if (! std::strcmp(tag, "a")) {
            write("((");
            parse(node.children());
            write("))[[");
            write(node["href"]);
            write("]]");
          }
          else if (! std::strcmp(tag, "strong") || ! std::strcmp(tag, "b")) {
            write("**");
            parse(node.children());
            write("**");
          }
          else if (! std::strcmp(tag, "em") || ! std::strcmp(tag, "i")) {
            write("__");
            parse(node.children());
            write("__");
          }
          else if (! std::strcmp(tag, "br")) {
            write('\n');
          }
          else {
            parse(node.children());
          }

          break;

        case XML_TEXT_NODE:
          write(node.content());
          parse(node.children());
          break;

        default:
          parse(node.children());
      }
    }
  }

private:
  template<typename T> inline void write(T value) {
    result += value;
  }
};

