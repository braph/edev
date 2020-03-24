#include "browsepage.hpp"

#include "ektoplayer.hpp"
#include "lib/sscan.hpp"
#include "lib/xml.hpp"

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>

#include <string>
#include <algorithm>    

using boost::algorithm::trim;
using boost::algorithm::trim_if;
using boost::algorithm::split;
using boost::algorithm::is_any_of;
using boost::algorithm::erase_all;

static Album get_album(Xml::XPath&, Xml::Node post);
static void fix_album_data(Album& album);
static inline const char* strMayNULL(const char* s) { return (s ? s : ""); }

/* Taken and adapted from:
 * https://stackoverflow.com/questions/46349697/decode-base64-string-using-boost */
static std::string base64_decode(std::string input)
{
  using namespace boost::archive::iterators;
  typedef transform_width<binary_from_base64<std::string::const_iterator >, 8, 6> ItBinaryT;

  try {
    // If the input isn't a multiple of 4, pad with =
    size_t num_pad_chars((4 - input.size() % 4) % 4);
    input.append(num_pad_chars, '=');

    auto pad_chars = std::count(input.begin(), input.end(), '=');
    std::replace(input.begin(), input.end(), '=', 'A');
    std::string output(ItBinaryT(input.begin()), ItBinaryT(input.end()));
    output.erase(output.end() - pad_chars, output.end());
    return output;
  }
  catch (std::exception const&) {
    return "";
  }
}

void BrowsePage :: parse_src(const std::string& src) {
  Xml::Doc doc = Html::readDoc(src, NULL, NULL, HTML_PARSE_RECOVER|HTML_PARSE_NOERROR|HTML_PARSE_NOWARNING|HTML_PARSE_COMPACT);
  auto xpath = doc.xpath();

#if 0
  // Get base url
  // -> https://ektoplazm.com/style/uplifting/page/2 (remove "/2")
  std::string base_url = xpath.query_string("string(//div[contains(@class, 'wp-pagenavi')]//a/@href)");
  if (! base_url.empty()) {
    base_url = base_url.substr(0, base_url.find_last_not_of("0123456789") + 1);
  }
#endif

  // Find number of pages
  std::string result = xpath.query_string("string(//span[@class = 'pages']/text())");
  if (! result.empty()) {
    // <span class='pages'>Page 1 of 31</span>
    auto p = result.find_last_not_of("0123456789");
    if (p != std::string::npos)
      num_pages = std::atoi(result.c_str() + p);
  }

  // Collect albums
  for (const auto& post : xpath.query("//div[starts-with(@id, 'post-')]")) {
    Album album = get_album(xpath, post);
    if (album.tracks.empty())
      continue;
    fix_album_data(album);
    albums.push_back(std::move(album));
  }
}

static Album get_album(Xml::XPath& xpath, Xml::Node post) {
  Album album;

  // Date
  std::string result = xpath.query_string("string(.//span[@class = 'd']/text())", post);
  if (! result.empty()) { // TODO: make this prettier + case insensitive
    const char* months = "Jan" "Feb" "Mar" "Apr" "May" "Jun" "Jul" "Aug" "Sep" "Oct" "Nov" "Dec";
    std::tm t = {};
    char month[32];
    std::sscanf(result.c_str(), "%s %d, %d", month, &t.tm_mday, &t.tm_year); // TODO: remove sscanf
    month[3] = '\0';
    t.tm_year -= 1900;
    const char* found_month = std::strstr(months, month);
    if (found_month)
      t.tm_mon = (months - found_month) / 3;
    album.date = ::mktime(&t);
  }

  // Download count
  result = xpath.query_string("string(.//span[@class = 'dc']//strong/text())");
  if (! result.empty()) {
    erase_all(result, ",");
    album.download_count = std::atoi(result.c_str());
  }

  // Rating, Voting count
  // <span class="d">Rated <strong>89.10%</strong> with <strong>189</strong> 
  auto strongs = xpath.query(".//p[@class = 'postmetadata']//span[@class = 'd']//strong", post);
  album.rating = std::atof(strMayNULL(strongs[0].nearestContent()));
  album.votes  = std::atoi(strMayNULL(strongs[1].nearestContent()));

  // Styles
  for (const auto& a : xpath.query(".//span[@class = 'style']//a", post)) {
    std::string url = a["href"];
    if (! url.empty())
      album.styles.push_back(Style(std::move(url), a.nearestContent()));
  }

  // Description (first <p> </p>)
  album.description = xpath.query(".//p", post)[0].dump();
  album.description.erase(0, album.description.find('>')+1); // Remove first tag
  album.description.erase(album.description.rfind('<'));     // Remove last tag
  trim(album.description);

  // Cover URL
  album.cover_url = xpath.query(".//img[@class = 'cover']", post)[0]["src"];

  // Album title and URL
  auto a_title = xpath.query(".//h1/a", post)[0];
  album.url   = a_title["href"];
  album.title = a_title.allText();
  trim(album.title);

  // Archive URLs (<span class="dll"><a href="...zip">MP3 Download</a>)
  for (const auto& a : xpath.query(".//span[@class = 'dll']//a", post)) {
    std::string url = a["href"];
    if (! url.empty())
      album.archive_urls.push_back(std::move(url));
  }

  // Direct mp3 track URLs
  // <script type="text/javascript"> soundFile:"..."
  std::vector<std::string> tracks;
  for (const auto& script : xpath.query(".//script", post)) {
    const char *base64_begin, *base64_end;
    if (! (base64_begin = script.nearestContent()))
      continue;
    if (! (base64_begin = std::strstr(base64_begin, "soundFile:")))
      continue;
    if (! (base64_begin = std::strchr(base64_begin, '"')))
      continue;
    if (! (base64_end = std::strchr(++base64_begin, '"')))
      continue;

    result = std::string(base64_begin, size_t(base64_end-base64_begin));
    result = base64_decode(std::move(result));
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
  for (const auto& tracklist : xpath.query(".//div[@class = 'tl']", post)) {
    Track track;
    for (const auto& span : xpath.query(".//span", tracklist)) {
      switch (span["class"][0]) {
        case 'n':
          if (! track.url.empty()) {
            album.tracks.push_back(std::move(track));
            track = Track();
          }

          track.number = std::atoi(strMayNULL(span.nearestContent()));
          if (album.isSingleURL)
            track.url = *track_urls_iter;
          else if (track_urls_iter != track_urls_end)
            track.url = std::move(*track_urls_iter++);
          break;
        case 't':
          trim((track.title = span.allText()));
          break;
        case 'r':
          trim_if((track.remix = span.allText()), is_any_of("\t ()"));
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
