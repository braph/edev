#ifndef BROWSEPAGE_HPP
#define BROWSEPAGE_HPP

#include <lib/xml.hpp>
#include <lib/iterator/iterator_pair.hpp>

#include <ctime>
#include <string>
#include <vector>

#ifndef NDEBUG
#include <iosfwd>
#endif

struct Style {
  std::string url;
  std::string name;

  inline Style(std::string url, std::string name) noexcept
  : url(std::move(url))
  , name(std::move(name))
  {}

#ifndef NDEBUG
  friend inline std::ostream& operator<<(std::ostream&, const Style&);
#endif
};

struct Track {
  std::string url;
  std::string title;
  std::string artist;
  std::string remix;
  short       bpm;
  short       length;
  short       number;

  inline Track() noexcept
  : bpm(0)
  , length(0)
  , number(0)
  {}

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
  int         download_count;
  int         votes;
  float       rating;
  bool        is_single_url;
  std::vector<std::string> archive_urls;
  std::vector<Style> styles;
  std::vector<Track> tracks;

  inline Album() noexcept
  : date(0)
  , download_count(0)
  , votes(0)
  , rating(0)
  , is_single_url(false)
  {
    styles.reserve(3);
    tracks.reserve(11);
    archive_urls.reserve(3);
  }

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
  Album next_album();

private:
  Html::Doc doc;
  Xml::XPath xpath;
  Xml::XPathResult xpath_albums;
  IteratorPair<Xml::XPathResult::iterator> xpath_albums_it;
};

#endif
