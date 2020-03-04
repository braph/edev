static PlaylistColumns opt_parse_playlist_columns(const std::string &s) {
  std::vector<std::string> opts;
  std::vector<std::string> columns;
  boost::split(columns, s, boost::is_any_of("|"));

  PlaylistColumns result;
  for (auto &column : columns) {
    PlaylistColumnFormat fmt;

    boost::split(opts, column, boost::is_any_of(" \t"), boost::token_compress_on);
    for (auto &opt : opts) {
      if (opt.empty())
        continue;
      else if (0 == opt.find("fg="))
        fmt.fg = UI::Color::parse(opt.substr(3));
      else if (0 == opt.find("bg="))
        fmt.bg = UI::Color::parse(opt.substr(3));
      else if (opt == "right")
        fmt.justify = PlaylistColumnFormat::Right;
      else if (opt == "left")
        fmt.justify = PlaylistColumnFormat::Left;
      else if (std::isdigit(opt[0])) {
        fmt.size = std::stoi(opt);
        fmt.relative = (opt.back() == '%');
      }
      else {
        fmt.tag = (Database::ColumnID) Database::columnIDFromStr(opt);
        if (fmt.tag == Database::COLUMN_NONE)
          throw std::invalid_argument(opt + ": No such tag");
      }
    }

    if (! fmt.size)
      throw std::invalid_argument(column + ": Missing column size");
    if (fmt.tag == Database::COLUMN_NONE)
      throw std::invalid_argument(column + ": Missing tag name");

    result.push_back(fmt);
  }

  return result;
}

//"<text fg='black'>&lt;&lt; </text><title bold='on' fg='yellow' /><text fg='black'> &gt;&gt;</text>";
static PlayingInfoFormat opt_parse_playinginfo_format(const std::string &_xml) {
  PlayingInfoFormat result;
  std::string xml = "<r>" + _xml + "</r>"; // Add fake root element
  XmlDoc   doc = XmlDoc::readDoc(xml, NULL, NULL, XML_PARSE_COMPACT);
  XmlNode root = doc.getRootElement(); // Safe, always having root element

  for (XmlNode node = root.children(); node; node = node.next()) {
    PlayingInfoFormatFoo fmt;
    std::string tag = node.name();

    //std::cout << "node type is: " << node.type() << std::endl;
    /*
    if (node->type == XML_TEXT_NODE)
      continue;
    */

    if (node.type() != XML_ELEMENT_NODE)
      continue;
      //throw std::invalid_argument("THIS IS NOT A ELEMENT NODE");

    if (tag == "text") {
      fmt.text = node.text();
    }
    else {
      if (node.children())
        throw std::invalid_argument(tag + ": THIS NODE HAS CHILDREN");

      fmt.tag = (Database::ColumnID) Database::columnIDFromStr(tag);
      if (fmt.tag == Database::COLUMN_NONE)
        throw std::invalid_argument(tag + ": No such tag");
    }

    for (XmlAttribute attribute = node.attributes(); attribute; attribute = attribute.next()) {
      std::string name  = attribute.name();
      std::string value = attribute.value();

      if /**/ (name == "fg")   fmt.fg   = UI::Color::parse(value);
      else if (name == "bg")   fmt.bg   = UI::Color::parse(value);
      else                     fmt.attributes |= UI::Attribute::parse(name);
    }

    result.push_back(fmt);
  }

  return result;
}
