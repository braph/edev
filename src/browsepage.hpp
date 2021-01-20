#ifndef BROWSEPAGE_HPP
#define BROWSEPAGE_HPP

#include <lib/xml.hpp>
#include <lib/iterator/iterator_pair.hpp>

#include <ctime>
#include <string>
#include <vector>
#include <iosfwd>

struct Style {
  std::string url;
  std::string name;

#ifndef NDEBUG
  friend inline std::ostream& operator<<(std::ostream&, const Style&);
#endif
};

struct Track {
  std::string url;
  std::string title;
  std::string artist;
  std::string remix;
  short       bpm = 0;
  short       length = 0;
  short       number = 0;

#ifndef NDEBUG
  friend inline std::ostream& operator<<(std::ostream&, const Track&);
#endif
};

struct Album {
  std::string url;
  std::string title;
  std::string artist;
  std::string description;
  std::string cover_url;
  std::time_t date;
  int         download_count = 0;
  int         votes = 0;
  float       rating = 0.0;
  bool        is_single_url = 0;
  std::vector<std::string> archive_urls;
  std::vector<Style> styles;
  std::vector<Track> tracks;

  inline bool empty() const noexcept {
    return tracks.empty();
  }

#ifndef NDEBUG
  friend inline std::ostream& operator<<(std::ostream&, const Album&);
#endif
};

class BrowsePageParser {
public:
  BrowsePageParser(const std::string&);
  int num_pages();
  Album next_album(); /* throws */

private:
  Html::Doc doc;
  Xml::XPath xpath;
  Xml::XPathResult xpath_albums;
  IteratorPair<Xml::XPathResult::iterator> xpath_albums_it;
};

#endif
