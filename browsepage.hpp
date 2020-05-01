#ifndef BROWSEPAGE_HPP
#define BROWSEPAGE_HPP

#include "lib/xml.hpp"

#include <ctime>
#include <string>
#include <vector>
#include <iosfwd>

struct Style {
  std::string url;
  std::string name;

  Style(std::string url, std::string name)
  : url(std::move(url))
  , name(std::move(name))
  {
  }

  friend inline std::ostream& operator<<(std::ostream&, const Style&);
};

struct Track {
  std::string url;
  std::string title;
  std::string artist;
  std::string remix;
  short       bpm;
  short       length;
  short       number;

  Track()
  : bpm(0)
  , length(0)
  , number(0)
  {}

  friend inline std::ostream& operator<<(std::ostream&, const Track&);
};

struct Album {
  std::string url;
  std::string title;
  std::string artist;
  std::string description;
  std::string cover_url;
  std::time_t date;
  int         download_count;
  int         votes;
  float       rating;
  bool        isSingleURL;
  std::vector<std::string> archive_urls;
  std::vector<Style> styles;
  std::vector<Track> tracks;

  Album()
  : date(0)
  , download_count(0)
  , votes(0)
  , rating(0)
  , isSingleURL(false)
  {
    styles.reserve(3);
    tracks.reserve(11);
    archive_urls.reserve(3);
  }

  inline bool empty() {
    return tracks.empty();
  }

  friend inline std::ostream& operator<<(std::ostream&, const Album&);
};

class BrowsePageParser {
public:
  BrowsePageParser(const std::string&);
  int num_pages();
  Album next_album();

private:
  Html::Doc doc;
  Xml::XPath xpath;
  Xml::XPathResult xpath_albums;
  Xml::XPathResult::iterator xpath_albums_it;
  Xml::XPathResult::iterator xpath_albums_end;
};

#endif
