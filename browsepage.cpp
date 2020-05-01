#include "browsepage.hpp"

#include "ektoplayer.hpp"
#include "lib/sscan.hpp"
#include "lib/cstring.hpp"

//#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
//#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/classification.hpp> // is_any_of
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>

#include <string>
#include <iostream>
#include <algorithm>    

//using boost::algorithm::trim;
//using boost::algorithm::trim_if;
using boost::algorithm::split;
using boost::algorithm::is_any_of;
//using boost::algorithm::erase_all;

inline std::ostream& operator<<(std::ostream& o, const Style& s) {
  return o << s.url << '|' << s.name;
}

inline std::ostream& operator<<(std::ostream& o, const Track& t) {
  return o
    << t.number << '|'
    << t.artist << '|'
    << t.title  << '|'
    << t.remix  << '|'
    << t.bpm    << " BPM|"
    << t.length << " Sec|"
    << t.url;
}

inline std::ostream& operator<<(std::ostream& o, const Album& a) {
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

static void fix_album_data(Album& album);
static inline const char* safe_str(const char* s) { return (s ? s : ""); }

static std::string base64_decode(const char* begin, const char* end) {
  using namespace boost::archive::iterators;
  using DecIt = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
  std::string r;
  r.reserve(size_t(end-begin));

  try {
    auto it_end = DecIt(end);
    for (auto it = DecIt(begin); it != it_end; ++it)
      r.push_back(*it);
  } catch (...) {}

  return r;
}

struct XPathExpressionCache {
  std::vector<std::pair<std::string, xmlXPathCompExprPtr>> _compExprPtrs;

  template<size_t size>
  xmlXPathCompExprPtr operator[](const char (&expr)[size]) {
    return get(&expr[0], size - 1);
  }

  xmlXPathCompExprPtr get(const char* expr, size_t len) {
    for (const auto& pair : _compExprPtrs)
      if (pair.first.length() == len && pair.first == expr)
        return pair.second;

    _compExprPtrs.push_back({expr, xmlXPathCompile(reinterpret_cast<const xmlChar*>(expr))});
    return _compExprPtrs.back().second;
  }
};

XPathExpressionCache cache;

static void startElementUser(void* ctxt, const xmlChar* name, const xmlChar** _attrs) {
  const char** attrs = reinterpret_cast<const char**>(_attrs);

  const char* new_attrs[30];
  int i = 0;

  if (attrs)
    for (; *attrs; attrs += 2)
      if (! std::strcmp(*attrs, "href") ||
          ! std::strcmp(*attrs, "id") ||
          ! std::strcmp(*attrs, "class") ||
          ! std::strcmp(*attrs, "src")) {
        new_attrs[i++] = *(attrs);
        new_attrs[i++] = *(attrs+1);
      }

  new_attrs[i] = NULL;

  xmlSAX2StartElement(ctxt,name,reinterpret_cast<const xmlChar**>(new_attrs));
}

BrowsePageParser :: BrowsePageParser(const std::string& source)
: doc(Html::readDoc(source, NULL, NULL,
      HTML_PARSE_RECOVER|HTML_PARSE_NOERROR|HTML_PARSE_NOWARNING|
      HTML_PARSE_COMPACT|HTML_PARSE_NOBLANKS))
, xpath(doc.xpath())
, xpath_albums(xpath.query(cache["//div[starts-with(@id, 'post-')]"]))
, xpath_albums_it(xpath_albums.begin())
, xpath_albums_end(xpath_albums.end())
{
  xmlSetBufferAllocationScheme(XML_BUFFER_ALLOC_DOUBLEIT);

  htmlDefaultSAXHandler.comment = NULL;
  htmlDefaultSAXHandler.cdataBlock = NULL;
  htmlDefaultSAXHandler.startElement = startElementUser;
}

int BrowsePageParser :: num_pages() {
  // <span class='pages'>Page 1 of 31</span>
  auto result = xpath.query_string(cache["string(//span[@class = 'pages']/text())"]);
  if (result) {
    const char* s = result.c_str();
    s = strrchr(s, ' ');
    if (s)
      return std::atoi(s);
  }

  return 0;
}

#if 0
int BrowsePageParser :: get_base_url() {
  // -> https://ektoplazm.com/style/uplifting/page/2 (remove "/2")
  std::string base_url = xpath.query_string("string(//div[contains(@class, 'wp-pagenavi')]//a/@href)");
  if (! base_url.empty()) {
    base_url = base_url.substr(0, base_url.find_last_not_of("0123456789") + 1);
  }
}
#endif

Album BrowsePageParser :: next_album() {
  Album album;

  if (xpath_albums_it == xpath_albums_end)
    return album;

  Xml::Node post = *xpath_albums_it;
  ++xpath_albums_it;

  // Date
  auto date = xpath.query_string(cache["string(.//span[@class = 'd']/text())"], post);
  if (date) { // TODO: make this prettier + case insensitive
    const char* months = "Jan" "Feb" "Mar" "Apr" "May" "Jun" "Jul" "Aug" "Sep" "Oct" "Nov" "Dec";
    std::tm t = {};
    char month[32];
    std::sscanf(date.c_str(), "%s %d, %d", month, &t.tm_mday, &t.tm_year); // TODO: remove sscanf
    month[3] = '\0';
    t.tm_year -= 1900;
    const char* found_month = std::strstr(months, month);
    if (found_month)
      t.tm_mon = (months - found_month) / 3;
    album.date = std::mktime(&t);
  }

  // Download count
  auto dc = xpath.query_string(cache["string(.//span[@class = 'dc']//strong/text())"], post);
  if (dc) {
    album.download_count = std::atoi(erase_all(dc.c_str(), ','));
  }

  // Rating, Voting count
  // <span class="d">Rated <strong>89.10%</strong> with <strong>189</strong> 
  auto strongs = xpath.query(cache[".//p[@class = 'postmetadata']//span[@class = 'd']//strong"], post);
  album.rating = std::atof(safe_str(strongs[0].nearestContent()));
  album.votes  = std::atoi(safe_str(strongs[1].nearestContent()));

  // Styles
  for (const auto& a : xpath.query(cache[".//span[@class = 'style']//a"], post)) {
    std::string url = a["href"];
    if (! url.empty())
      album.styles.push_back(Style(std::move(url), a.nearestContent()));
  }

  // Description (first <p> </p>)
  album.description = xpath.query(cache[".//p"], post)[0].dump();
  trim(album.description);

  // Cover URL
  album.cover_url = xpath.query(cache[".//img[@class = 'cover']"], post)[0]["src"];

  // Album title and URL
  auto a_title = xpath.query(cache[".//h1/a"], post)[0];
  album.url   = a_title["href"];
  album.title = a_title.allText();
  trim(album.title);

  // Archive URLs (<span class="dll"><a href="...zip">MP3 Download</a>)
  for (const auto& a : xpath.query(cache[".//span[@class = 'dll']//a"], post)) {
    std::string url = a["href"];
    if (! url.empty())
      album.archive_urls.push_back(std::move(url));
  }

  // Direct mp3 track URLs
  // <script type="text/javascript"> soundFile:"..."
  std::vector<std::string> tracks;
  for (const auto& script : xpath.query(cache[".//script"], post)) {
    const char *base64_begin, *base64_end;
    if (! (base64_begin = script.nearestContent()))
      continue;
    if (! (base64_begin = std::strstr(base64_begin, "soundFile:")))
      continue;
    if (! (base64_begin = std::strchr(base64_begin, '"')))
      continue;
    if (! (base64_end = std::strchr(++base64_begin, '"')))
      continue;

    std::string result = base64_decode(base64_begin, base64_end);
    split(tracks, result, is_any_of(","));

    if (tracks.size())
      break;
  }

  // This should only happen on `.../dj-basilisk-the-colours-of-ektoplazm`
  if (tracks.empty())
    return album;

  // Some albums only have one MP3 url for multiple tracks
  if (tracks.size() == 1)
    album.isSingleURL = true;

  // Assign metadata to track urls
  // - There may be multiple tracklists (evidence url?)
  auto track_urls_iter = tracks.begin();
  auto track_urls_end  = tracks.end();
  for (const auto& tracklist : xpath.query(cache[".//div[@class = 'tl']"], post)) {
    Track track;
    for (const auto& span : xpath.query(cache[".//span"], tracklist)) {
      switch (span["class"][0]) {
        case 'n':
          if (! track.url.empty()) {
            album.tracks.push_back(std::move(track));
            track = Track();
          }

          track.number = std::atoi(safe_str(span.nearestContent()));
          if (album.isSingleURL)
            track.url = *track_urls_iter;
          else if (track_urls_iter != track_urls_end)
            track.url = std::move(*track_urls_iter++);
          break;
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

    if (! track.url.empty())
      album.tracks.push_back(std::move(track));
  }

  fix_album_data(album);
  return album;
}

static inline size_t find_dash(const std::string& s, size_t& dash_len) {
  size_t pos;
  if ((pos = s.find("–")) != std::string::npos) // Unicode dash (precedence!)
    dash_len = sizeof("–") - 1;
  else if ((pos = s.find("-")) != std::string::npos) // ASCII dash
    dash_len = 1;
  return pos;
}

static void fix_album_data(Album& album) {
  size_t idx, dash_len = 0;

  // Sometimes the track title is merged into the track artist.
  // ("artist - track") -> https://ektoplazm.com/free-music/gods-food
  for (auto& track : album.tracks)
    if (track.title.empty()) {
      idx = find_dash(track.artist, dash_len);
      if (idx != std::string::npos) {
        track.title = track.artist.substr(idx + dash_len);
        track.artist.erase(idx);
        trim(track.title);
        trim(track.artist);
      }
    }

  // Extract the artist name from the album title ("album_artist - album_title")
  idx = find_dash(album.title, dash_len);
  if (idx != std::string::npos) {
    album.artist = album.title.substr(0, idx);
    album.title.erase(0, idx + dash_len);
    trim(album.title);
    trim(album.artist);
  } else {
    album.artist = "Unknown Artist";
  }

  // Use album.artist if track.artist is empty
  for (auto& track : album.tracks)
    if (track.artist.empty())
      track.artist = album.artist;
}

#ifdef TEST_BROWSEPAGE
int main() {
  return 0;
}
#endif
