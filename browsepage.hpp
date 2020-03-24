#ifndef BROWSEPAGE_HPP
#define BROWSEPAGE_HPP

#include <ctime>
#include <string>
#include <vector>
#include <ostream>

struct Style {
  std::string url;
  std::string name;

  Style(std::string url, std::string name)
  : url(std::move(url)), name(std::move(name))
  {
  }

  inline friend std::ostream& operator<<(std::ostream& o, const Style& s) {
    return o << s.url << '|' << s.name;
  }
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

  inline friend std::ostream& operator<<(std::ostream& o, const Track& t) {
    return o
      << t.number << '|'
      << t.artist << '|'
      << t.title  << '|'
      << t.remix  << '|'
      << t.bpm    << " BPM|"
      << t.length << " Sec|"
      << t.url;
  }
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
    tracks.reserve(10);
    archive_urls.reserve(3);
  }

  inline friend std::ostream& operator<<(std::ostream& o, const Album& a) {
    std::tm* t = std::localtime(&a.date);
    char date_string[12];
    std::strftime(date_string, sizeof(date_string), "%Y-%m-%d", t);

    o <<   "Title:       " << a.title
      << "\nArtist:      " << a.artist
      << "\nDate:        " << date_string
      << "\nDescription: " << a.description
      << "\nCover URL:   " << a.cover_url
      << "\nDownloads:   " << a.download_count
      << "\nRated:       " << a.rating << " (" << a.votes << " votes)"
      << "\nURL:         " << a.url
      << "\nStyles:      "; for (auto& s : a.styles)       { o << s << ',';  }
    o << "\nArchives:    "; for (auto& i : a.archive_urls) { o << i << ',';  }
    o << "\nTracks:      "; for (auto& t : a.tracks)       { o << '\n' << t; }
    return o;
  }
};

class BrowsePage {
public:
  int num_pages;
  std::vector<Album> albums;

  BrowsePage()
  : num_pages(0)
  {
    albums.reserve(5);
  }

  BrowsePage(const std::string& src)
  : num_pages(0)
  {
    albums.reserve(5);
    parse_src(src);
  }

  void parse_src(const std::string&);
};

#endif
