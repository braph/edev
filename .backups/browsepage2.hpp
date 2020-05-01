#ifndef BROWSEPAGE_HPP
#define BROWSEPAGE_HPP

#include "lib/xml.hpp"

#include <ctime>
#include <string>
#include <vector>

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

class TrackMetaDataParser {
  enum MetaType {
    TRACK_NUMBER,
    TRACK_TITLE,
    TRACK_REMIX,
    TRACK_ARTIST,


        case 't':
          trim((track.title = span.allText()));
          break;
        case 'r':
          trim((track.remix = span.allText()), "\t ()");
          break;
        case 'a':
          trim((track.artist = span.allText()));
          break;
        case 'd':
          const char* s = span.nearestContent();
          if (s) {
            if (std::strchr(s, ':')) { // "(4:32)"
              short minutes = 0;
              SScan(s).skip_until("0123456789").strtoi(minutes).read(':').strtoi(track.length);
              track.length += minutes * 60;
            }
            else { // "(134 BPM)"
              SScan(s).skip_until("0123456789").strtoi(track.bpm);
            }
          }
          break;
      }
    }





};

/**
 * Album parser.
 * Each method call will invalidate the result of a previous method call!
 */
class AlbumParser {
public:
  std::string& url();
  std::string& title();
//std::string& artist();
  std::string& description();
  std::string& cover_url();
  std::time_t  date();
  int          download_count();
  std::pair<float, int> rating_and_vote_count();
  std::vector<std::string> archive_urls();
  std::vector<std::pair<std::string, std::string>> styles();
  std::vector<std::string> track_urls();

private:
  std::string _string;

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
