#ifndef _BROWSEPAGE_HPP
#define _BROWSEPAGE_HPP

#include <map>
#include <vector>
#include <string>
#include <sstream>

class Track {
  public:
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
      std::stringstream ss; ss
        << number << '|' << artist << '|' << title << '|' << remix << '|' << bpm << '|' << url;
      return ss.str();
    }
};

class Album {
  public:
    std::string  url;
    std::string  title;
    std::string  artist;
    std::string  date;
    std::string  description;
    std::string  cover_url;
    unsigned int download_count;
    unsigned int votes;
    float        rating;
    std::vector<std::string> styles;
    std::vector<std::string> archive_urls;
    std::vector<Track> tracks;

    inline Album()
    : download_count(0)
    , votes(0)
    , rating(0)
    {
      tracks.reserve(10); // Average track count on an album
    }

    inline std::string to_string() const {
      std::stringstream ss; ss
        <<   "Title:       " << title
        << "\nArtist:      " << artist
        << "\nDate:        " << date
        << "\nDescription: " << description
        << "\nCover URL:   " << cover_url
        << "\nDownloads:   " << download_count
        << "\nRated:       " << rating << " (" << votes << " votes)"
        << "\nURL:         " << url
        << "\nStyles:      ";
      for (const auto &style : styles) { ss << style << ',';              }
      for (const auto &track : tracks) { ss << '\n' << track.to_string(); }
      //for (const auto &url : archive_urls) { ss << 
      return ss.str();
    }
};

class BrowsePage {
  public:
    unsigned int num_pages;
    unsigned int current_page;
    std::string base_url;
    std::vector<Album> albums;

    BrowsePage() : num_pages(0), current_page(0) {}
    BrowsePage(const std::string &src) : num_pages(0), current_page(0) {
      albums.reserve(5); // One page holds 5 albums
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
