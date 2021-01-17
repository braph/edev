#include "markdown.hpp"

#include <lib/stringpack.hpp>
#include <lib/xml/sax.hpp>

#include <libxml/parser.h>

namespace {

struct State {
  std::string result;
  std::string url;
};

void startElement(void* self_, const xmlChar* name, const xmlChar** attrs) {
  auto& self = *static_cast<State*>(self_);

  using pack = StringPack::Raw;
  switch (pack(name)) {
  case pack("i"):
  case pack("em"):
    self.result.append(2, '_');
    break;
  case pack("b"):
  case pack("strong"):
    self.result.append(2, '*');
    break;
  case pack("a"):
    self.result.append(2, '(');
    self.url = Xml::Sax::Attributes(attrs)["href"].value;
    break;
  case pack("br"):
    self.result.append(1, '\n');
    break;
  }
}

void endElement(void* self_, const xmlChar* name) {
  auto& self = *static_cast<State*>(self_);

  using pack = StringPack::Raw;
  switch (pack(name)) {
  case pack("i"):
  case pack("em"):
    self.result.append(2, '_');
    return;
  case pack("b"):
  case pack("strong"):
    self.result.append(2, '*');
    return;
  case pack("a"):
    self.result.append(2, ')');
    self.result.append(2, '[');
    self.result.append(self.url);
    self.result.append(2, ']');
  }
}

void characters(void* self_, const xmlChar* ch, int len) {
  auto& self = *static_cast<State*>(self_);
  self.result.append(reinterpret_cast<const char*>(ch), size_t(len));
}

} // namespace

namespace Html2Markdown {

std::string convert(const char* src, size_t size) {
  xmlSAXHandler handler = {};
  handler.startElement  = startElement;
  handler.endElement    = endElement;
  handler.characters    = characters;

  State state;
  state.result.reserve(size * 0.5);
  ::xmlSAXUserParseMemory(&handler, &state, src, size);
  return state.result;
}

} // namespace Html2Markdown

