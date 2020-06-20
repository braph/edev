
static void startElementUser(void* ctxt, const xmlChar* name, const xmlChar** attrs) {
  // total heap usage: 7,827,521 allocs, 7,827,402 frees, 840,546,531 bytes allocated
  // total heap usage: 7,429,129 allocs, 7,429,011 frees, 810,903,274 bytes allocated
  enum AttributeEnum : unsigned int { CLASS = 1, HREF = 2, SRC = 4 };
  unsigned int keep = 0;
  switch (pack::pack_runtime(name)) {
    case pack("a"):     keep = HREF;         break;
    case pack("img"):   keep = SRC|CLASS;    break;
    // Keep tags and "class"
    case pack("div"):
    case pack("span"):
    case pack("p"):     keep = CLASS;
    // Keep those tags (without attributes)
    case pack("i"):
    case pack("b"):
    case pack("em"):
    case pack("strong"):
    case pack("h1"):
    case pack("br"):
    case pack("script"): break;
    default: return; // discard other tags
  }

  if (attrs == reinterpret_cast<const xmlChar**>(1))
    return xmlSAX2EndElement(ctxt, name);

  int i = 0;
  const xmlChar* new_attrs[10] = {};
  if (attrs) {
    for (; keep && *attrs; attrs += 2) {
      AttributeEnum ae;
      switch (pack::pack_runtime(*attrs)) {
        case pack("href"):  ae = HREF;  break;
        case pack("class"): ae = CLASS; break;
        case pack("src"):   ae = SRC;   break;
        default: continue;
      }

      keep -= ae;
      new_attrs[i++] = *(attrs);
      new_attrs[i++] = *(attrs+1);
    }
  }

  xmlSAX2StartElement(ctxt, name, new_attrs);
}

static void endElementUser(void* ctxt, const xmlChar* name) {
  startElementUser(ctxt, name, reinterpret_cast<const xmlChar**>(1));
}

BrowsePageParser :: BrowsePageParser(const std::string& source)
: doc(Html::readDoc(source, NULL, NULL,
    HTML_PARSE_RECOVER|HTML_PARSE_NOERROR|HTML_PARSE_NOWARNING|HTML_PARSE_COMPACT|HTML_PARSE_NOBLANKS))
, xpath(doc.xpath())
, xpath_albums(xpath.query(cache["//div[@class = 'post']"]))
, xpath_albums_it(xpath_albums)
{
  xmlSetBufferAllocationScheme(XML_BUFFER_ALLOC_DOUBLEIT);

  htmlDefaultSAXHandler.comment = NULL;
  htmlDefaultSAXHandler.cdataBlock = NULL;
  htmlDefaultSAXHandler.ignorableWhitespace = NULL;
  htmlDefaultSAXHandler.startElement = startElementUser;
  htmlDefaultSAXHandler.endElement = endElementUser;
}

