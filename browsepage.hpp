#ifndef _BROWSEPAGE_HPP
#define _BROWSEPAGE_HPP

#include <ctime>
#include <vector>
#include <string>
#include <sstream>

struct Style {
  std::string url;
  std::string name;

  inline Style(const std::string& url, const std::string& name)
  : url(url), name(name)
  {
  }

  inline std::string to_string() const {
    return url + '|' + name;
  }
};

struct Track {
  std::string  url;
  std::string  title;
  std::string  artist;
  std::string  remix;
  short        bpm;
  short        number;

  inline Track()
  : bpm(0)
  , number(0)
  {}

  inline std::string to_string() const {
    std::stringstream o;o
      << number << '|'
      << artist << '|'
      << title  << '|'
      << remix  << '|'
      << bpm    << '|'
      << url;
    return o.str();
  }
};

struct Album {
  std::string  url;
  std::string  title;
  std::string  artist;
  std::string  description;
  std::string  cover_url;
  time_t       date;
  short        download_count;
  short        votes;
  float        rating;
  std::vector<std::string> archive_urls;
  std::vector<Style> styles;
  std::vector<Track> tracks;

  inline Album()
  : date(0)
  , download_count(0)
  , votes(0)
  , rating(0)
  {
    styles.reserve(3);
    tracks.reserve(10);
    archive_urls.reserve(3);
  }

  inline std::string to_string() const {
    struct tm* tm = localtime(&date);
    char sdate[20];
    ::strftime(sdate, sizeof(sdate), "%Y-%m-%d 00:00:00", tm);

    std::stringstream o;o
      <<   "Title:       " << title
      << "\nArtist:      " << artist
      << "\nDate:        " << sdate
      << "\nDescription: " << description
      << "\nCover URL:   " << cover_url
      << "\nDownloads:   " << download_count
      << "\nRated:       " << rating << " (" << votes << " votes)"
      << "\nURL:         " << url
      << "\nStyles:      "; for (auto &s : styles)       { o << s.to_string() << ',';  }
    o << "\nArchives:    "; for (auto &i : archive_urls) { o << i << ',';              }
    o << "\nTracks:      "; for (auto &t : tracks)       { o << '\n' << t.to_string(); }
    return o.str();
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

  BrowsePage(const std::string &src)
  : num_pages(0)
  {
    albums.reserve(5);
    parse_src(src);
  }

  void parse_src(const std::string&);
};

#endif
