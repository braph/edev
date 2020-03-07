
          //track.artist = track.artist.erase(0, idx + dash_len);
struct LocaleSetter {
  LocaleSetter(const std::locale& loc) : _old(std::locale::global(loc)) {}
 ~LocaleSetter() { std::locale::global(_old); }
  std::locale _old;
};

  std::streambuf *oldErrBuff;
  oldErrBuff = std::cerr.rdbuf();
  //std::ofstream *logfile = new std::ofstream(); /* "wanted" leak */
  logfile.exceptions(std::ofstream::failbit|std::ofstream::badbit);
  logfile.open(Config::log_file, std::ofstream::out|std::ofstream::app);
  std::cerr.rdbuf(logfile.rdbuf());

  xmlCleanupParser();
  delwin(stdscr);
  delscreen(set_term(NULL));


// Code that had been used in updater.cpp =====================================
static std::string clean_description(const std::string& desc) {
  size_t pos;

  /* Strip those garbage attributes off the description. */
  for (const char* search : { "href=\"/cdn-cgi/l/email", "class=\"__cf_email__", "data-cfemail=\""}) {
    while (std::string::npos != (pos = desc.find(search))) {
      size_t end = desc.find('"', pos+std::strlen(search));
      if (end != std::string::npos)
        desc.erase(pos, end-pos+1);
    }
  }

  /* Save space by using shorter tags and trimming whitespace */
  for (const char* search : {"strong>\0b>", "em>\0i>", "  \0 ", " >\0>"}) {
    size_t search_len = strlen(search);
    const char* replace = search + search_len + 1;
    while (std::string::npos != (pos = desc.find(search)))
      desc.replace(pos, search_len, replace);
  }
}

// Code that had been used in config.cpp ======================================
static std::string extract(std::string &s, const char *text) {
  std::string result;
  size_t pos = s.find(text);
  if (pos != std::string::npos) {
    size_t end = s.find_first_of(" \t", pos+1);
    if (end == std::string::npos) {
      result = s.substr(pos);
      s.erase(pos);
    } else {
      result = s.substr(pos, end - pos);
      s.erase(pos, end - pos);
    }
  }
  return result;
}

static void lstrip(std::string &s) {
  while (s.size() && (s[0] == ' ' || s[0] == '\t'))
    s.erase(0, 1);
}

#if 0
    if (!in_list<std::string>(w, {"splash", "playlist", "browser", "info", "help"}))
#endif

#if 0 // OLD
    for (auto &column_name : COLUMN_NAMES)
      if (extract(column, column_name) == column_name)
        { fmt.tag = column_name; break; }
    if (fmt.tag.empty())
      throw std::invalid_argument(column + ": Missing column name");

    std::string fg = extract(column, "fg=");
    if (!fg.empty())
      fmt.fg = UI::Color::parse(fg.substr(3));

    std::string bg = extract(column, "bg=");
    if (!bg.empty())
      fmt.bg = UI::Color::parse(bg.substr(3));

    if (extract(column, "right") == "right")
      fmt.justify = PlaylistColumnFormat::Right;
    else if (extract(column, "left") == "left")
      fmt.justify = PlaylistColumnFormat::Left;

    fmt.relative = (extract(column, "%") == "%");
    fmt.size = std::stoi(column);
    if (! fmt.size)
      throw std::invalid_argument(column + ": Invalid column size");
#endif
