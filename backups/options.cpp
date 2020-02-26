#include "options.hpp"
#include "common.hpp"
#include "colors.hpp"
#include "xml.hpp"

#include <iostream>//XXX
#include <boost/algorithm/string.hpp>

#define COLUMN_NAMES { "number", "artist", "album", "title", "styles", "bpm" }

PlaylistColumns parse_playlist_format(const std::string &_xml) {
  std::string xml = "<doc>" + _xml + "</doc>";
  XmlDoc doc = XmlDoc::readDoc(xml, NULL, NULL, XML_PARSE_COMPACT);

  xmlNode *root = xmlDocGetRootElement(doc);
  if (! root)
    throw std::invalid_argument("XML misses root element");
  root = root->children;

  std::vector<PlaylistColumnFormat> result;

  for (xmlNode *node = root; node; node = node->next) {
    std::cout << "node type is: " << node->type << std::endl;
    /*
    if (node->type == XML_TEXT_NODE)
      continue;
    */

    if (node->type != XML_ELEMENT_NODE)
      continue;
      //throw std::invalid_argument("THIS IS NOT A ELEMENT NODE");

    if (node->children)
      throw std::invalid_argument("THIS NODE HAS CHILDREN");

    std::string column = (const char*) node->name;
    if (!in_list<std::string>(column, COLUMN_NAMES))
      throw std::invalid_argument(column + ": No such tag");

    PlaylistColumnFormat fmt;
    fmt.column = column;

    xmlAttr* attribute = node->properties;
    while(attribute && attribute->name && attribute->children)
    {
      std::string name = (const char*) attribute->name;

      xmlChar* v = xmlNodeListGetString(node->doc, attribute->children, 1);
      std::string value = (const char*) v;
      xmlFree(v);

      if (name == "fg")           fmt.fg   = UI::Color::parse(value);
      else if (name == "bg")      fmt.bg   = UI::Color::parse(value);
      else if (name == "rel")     fmt.rel  = std::stoi(value);
      else if (name == "size")    fmt.size = std::stoi(value);
      else if (name == "justify") {
        if (value == "right")
          fmt.justification = Right;
        else if (value == "left")
          fmt.justification = Left;
        else
          throw std::invalid_argument("Invalid argument for justify");
      }
      else {
        //XXX:THROW
      }

      attribute = attribute->next;
    }

    if (fmt.size && fmt.rel)
      throw std::invalid_argument("REL AND SIZE CANNOT BE USED WITH SIZE");
  }

  return result;
}



PlayingInfoFormat opt_parse_playinginfo_format(const std::string &_xml) {
  std::vector<PlayingInfoFormatFoo> result;
#if 0
static const char * const DEFAULT_PLAYINGINFO_FORMAT_TOP =
  "<text fg='black'>&lt;&lt; </text><title bold='on' fg='yellow' /><text fg='black'> &gt;&gt;</text>";
#endif
  std::string xml = "<doc>" + _xml + "</doc>";
  //PlayingInfoFormat result;

#if lskfjsklfjsdklfjsldjkf
  xmlDoc *doc = xmlReadDoc((const xmlChar*) xml.c_str(), NULL, NULL, XML_PARSE_COMPACT);
  if (! doc)
    throw std::invalid_argument("Malformed XML");

  xmlNode *root = xmlDocGetRootElement(doc);
  if (! root)
    throw std::invalid_argument("XML misses root element");
  root = root->children;

  for (xmlNode *node = root; node; node = node->next) {
    std::cout << "node type is: " << node->type << std::endl;
    /*
    if (node->type == XML_TEXT_NODE)
      continue;
    */

    if (node->type != XML_ELEMENT_NODE)
      continue;
      //throw std::invalid_argument("THIS IS NOT A ELEMENT NODE");

    if (node->children)
      throw std::invalid_argument("THIS NODE HAS CHILDREN");

    std::string column = (const char*) node->name;
    for (unsigned i = 0; i < ARRAY_SIZE(COLUMN_NAMES); ++i)
      if (column == COLUMN_NAMES[i])
        goto FOUND;
    throw std::invalid_argument("THIS COLUMN NAME DOES NOT EXIST");
FOUND:

    PlaylistColumnFormat fmt;
    fmt.column = column;

    xmlAttr* attribute = node->properties;
    while(attribute && attribute->name && attribute->children)
    {
      std::string name = (const char*) attribute->name;

      xmlChar* v = xmlNodeListGetString(node->doc, attribute->children, 1);
      std::string value = (const char*) v;
      xmlFree(v);

      if (name == "fg")           fmt.fg   = UI::Color::parse(value);
      else if (name == "bg")      fmt.bg   = UI::Color::parse(value);
      else if (name == "rel")     fmt.rel  = std::stoi(value);
      else if (name == "size")    fmt.size = std::stoi(value);
      else if (name == "justify") {
        if (value == "right")
          fmt.justification = Right;
        else if (value == "left")
          fmt.justification = Left;
        else
          throw std::invalid_argument("Invalid argument for justify");
      }
      else {
        //XXX:THROW
      }

      attribute = attribute->next;
    }

    if (fmt.size && fmt.rel)
      throw std::invalid_argument("REL AND SIZE CANNOT BE USED WITH SIZE");
  }

  xmlFreeDoc(doc);
  xmlCleanupParser();

#endif
  return result;
}









int opt_parse_int(const std::string &s) {
  int i;
  if (sscanf(s.c_str(), "%d", &i) != 1)
    throw std::invalid_argument("not an integer");
  return i;
}

bool opt_parse_bool(const std::string &s) {
  if (s == "true")
    return true;
  else if (s == "false")
    return false;
  else
    throw std::invalid_argument("expected `true` or `false`");
}

std::string opt_parse_string(const std::string &s) {
  return s;
}

std::string validate_use_colors(const std::string &s) {
  if (s == "auto" || s == "mono" || s == "8" || s == "256")
    return s;

  throw std::invalid_argument("expected auto|mono|8|256");
}

int validate_threads(const std::string &s) {
  int i = opt_parse_int(s);
  if (i < 1)
    throw std::invalid_argument("integer must be > 1");
  return i;
}

std::vector<std::string> opt_parse_tabs_widgets(const std::string &s) {
  std::vector<std::string> widgets;
  boost::split(widgets, s, boost::is_any_of(",")); // XXX: \s*,\s* ???
  for (const auto& w : widgets)
    if (!in_list<std::string>(w, {"splash", "playlist", "browser", "info", "help"}))
      throw std::invalid_argument(w + ": Invalid widget");
  return widgets;
}

std::vector<std::string> opt_parse_main_widgets(const std::string &s) {
  std::vector<std::string> widgets;
  boost::split(widgets, s, boost::is_any_of(",")); // XXX: \s*,\s* ???
  for (const auto& w : widgets)
    if (!in_list<std::string>(w, {"playinginfo", "tabbar", "mainwindow", "progressbar"}))
      throw std::invalid_argument(w + ": Invalid widget");
  return widgets;
}

#ifdef TEST_OPTIONS
int main() {
  UI::Color::init();
  std::vector<PlaylistColumnFormat> r = parse_playlist_format(DEFAULT_PLAYLIST_FORMAT);
}
#endif
