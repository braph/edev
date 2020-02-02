#ifndef _BROWSEPAGE_HPP
#define _BROWSEPAGE_HPP

#include <map>
#include <ctime>
#include <vector>
#include <string>
#include <sstream>

struct Track {
  std::string    url;
  std::string    title;
  std::string    artist;
  std::string    remix;
  unsigned short bpm;
  unsigned short number;

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
  unsigned int download_count;
  unsigned int votes;
  float        rating;
  std::vector<std::string> styles;
  std::vector<std::string> archive_urls;
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
      << "\nStyles:      "; for (auto &i : styles)       { o << i << ','; }
    o << "\nArchives:    "; for (auto &i : archive_urls) { o << i << ','; }
    o << "\nTracks:      "; for (auto &t : tracks)       { o << '\n' << t.to_string(); }
    return o.str();
  }
};

class BrowsePage {
  public:
    unsigned int num_pages;
    unsigned int current_page;
    std::string base_url;
    std::vector<Album> albums;

    BrowsePage()
    : num_pages(0)
    , current_page(0)
    {}

    BrowsePage(const std::string &src)
    : num_pages(0)
    , current_page(0) {
      albums.reserve(5);
      parse_src(src);
    }

    void parse_src(const std::string&);
    std::map<std::string, std::string> getStyles(const std::string&);
    std::string getBaseUrl(const std::string&, int*n);

    inline std::string getPageUrl(unsigned int num) {
      return base_url + std::to_string(num);
    }
};

#endif
