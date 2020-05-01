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
#include <algorithm>    

//using boost::algorithm::trim;
//using boost::algorithm::trim_if;
using boost::algorithm::split;
using boost::algorithm::is_any_of;
//using boost::algorithm::erase_all;

static inline const char* safe_str(const char* s) { return (s ? s : ""); }

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

// ============================================================================
// AlbumParser
// ============================================================================

std::string& AlbumParser :: url() {
  auto a_title = xpath.query(cache[".//h1/a"], post)[0];
  _string = a_title["href"];
  return _string;
}

std::string& AlbumParser :: title() {
  auto a_title = xpath.query_string(cache[".//h1/a"], post)[0];
  _string = a_title.allText();
  return _string;
}

std::string& AlbumParser :: description() {
  // Description (first <p> </p>)
  _string = xpath.query(cache[".//p"], post)[0].dump();
  return _string;
}

std::string& AlbumParser :: cover_url() {
  _string = xpath.query(cache[".//img[@class = 'cover']"], post)[0]["src"];
  return _string;
}

std::time_t AlbumParser :: date() {
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
    return std::mktime(&t);
  }

  return 0;
}

int AlbumParser :: download_count() {
  auto dc = xpath.query_string(cache["string(.//span[@class = 'dc']//strong/text())"], post);
  if (dc)
    return std::atoi(erase_all(dc.c_str(), ','));
  return 0;
}

std::pair<float, int> AlbumParser :: rating_and_vote_count() {
  // <span class="d">Rated <strong>89.10%</strong> with <strong>189</strong> 
  auto strongs = xpath.query(cache[".//p[@class = 'postmetadata']//span[@class = 'd']//strong"], post);
  return std::pair<float, int>(
    std::atof(safe_str(strongs[0].nearestContent())),
    std::atoi(safe_str(strongs[1].nearestContent()))
  );
}

std::vector<std::string> AlbumParser :: archive_urls() {
  // Archive URLs (<span class="dll"><a href="...zip">MP3 Download</a>)
  std::vector<std::string> urls;
  for (const auto& a : xpath.query(cache[".//span[@class = 'dll']//a"], post)) {
    std::string url = a["href"];
    if (! url.empty())
      urls.push_back(std::move(url));
  }
  return urls;
}

std::vector<std::pair<std::string, std::string>> AlbumParser :: styles() {
  std::vector<std::pair<std::string, std::string>> styles;
  for (const auto& a : xpath.query(cache[".//span[@class = 'style']//a"], post)) {
    std::string url = a["href"];
    if (! url.empty())
      styles.push_back({std::move(url), a.nearestContent()});
  }
  return styles;
}

std::vector<std::string> AlbumParser :: track_urls() {
  using namespace boost::archive::iterators;
  using DecodeIt = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;

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

    _string.clear();
    try {
      auto it_end = DecodeIt(end);
      for (auto it = DecodeIt(begin); it != it_end; ++it)
        _string.push_back(*it);
    } catch (...) {}

    split(tracks, _string, is_any_of(","));
    if (tracks.size())
      break;
  }

  return tracks;
}

Album BrowsePageParser :: next_album() {
  Album album;

  if (xpath_albums_it == xpath_albums_end)
    return album;

  Xml::Node post = *xpath_albums_it;
  ++xpath_albums_it;

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


#ifdef TEST_BROWSEPAGE
int main() {
  return 0;
}
#endif
