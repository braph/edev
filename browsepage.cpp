#include "ektoplayer.hpp"
#include "browsepage.hpp"
#include "common.hpp"
#include "xml.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>

#include <algorithm>    

static inline std::string& lstrip(std::string& s, char what) {
  while (s.size() && s.front() == what)
    s.erase(0, 1);
  return s;
}

static inline std::string& rstrip(std::string& s, char what) {
  while (s.size() && s.back() == what)
    s.pop_back();
  return s;
}

static inline std::string& strip(std::string& s, char what) {
  return lstrip(rstrip(s, what), what);
}

static inline std::string _basename(std::string s) {
  size_t pos = s.rfind('/');
  if (pos != std::string::npos)
    s.erase(0, pos+1);
  return s;
}

static inline size_t find_dash(const std::string& s, size_t &dash_len) {
  size_t pos;
  for (const auto dash : { "–" /* Unicode */, "-" /* ASCII */ })
    if (std::string::npos != (pos = s.find(dash))) {
      dash_len = std::strlen(dash);
      return pos;
    }
  return std::string::npos;
}

/* Taken and adapted from:
 * https://stackoverflow.com/questions/46349697/decode-base64-string-using-boost */
std::string base64_decode(std::string input)
{
  using namespace boost::archive::iterators;
  typedef transform_width<binary_from_base64<std::string::const_iterator >, 8, 6> ItBinaryT;

  try {
    // If the input isn't a multiple of 4, pad with =
    size_t num_pad_chars((4 - input.size() % 4) % 4);
    input.append(num_pad_chars, '=');

    size_t pad_chars(std::count(input.begin(), input.end(), '='));
    std::replace(input.begin(), input.end(), '=', 'A');
    std::string output(ItBinaryT(input.begin()), ItBinaryT(input.end()));
    output.erase(output.end() - pad_chars, output.end());
    return output;
  }
  catch (std::exception const&)
  {
    return std::string("");
  }
}

void BrowsePage :: parse_src(const std::string &src) {
  XmlDoc doc = HtmlDoc::readDoc(src, NULL, NULL, HTML_PARSE_RECOVER|HTML_PARSE_NOERROR|HTML_PARSE_NOWARNING|HTML_PARSE_COMPACT);
  auto xpath = doc.xpath();
  std::vector<std::string> tracks;

#if 0
  // === Get base url ===
  // -> https://ektoplazm.com/style/uplifting/page/2 (remove "/2")
  std::string base_url = xpath.query_string("string(//div[contains(@class, 'wp-pagenavi')]//a/@href)");
  if (! base_url.empty()) {
    base_url = base_url.substr(0, base_url.find_last_not_of("0123456789") + 1);
  }
#endif

  // === Find number of pages === (XXX Strange, this query fails with [0] ...)
  std::string result = xpath.query_string("string(//span[@class = 'pages']/text())");
  if (! result.empty()) {
    // <span class='pages'>Page 1 of 31</span>
    std::string pages = result;
    auto p = pages.find_last_not_of("0123456789");
    if (p != std::string::npos) {
      num_pages = std::stoi(pages.substr(p + 1));
    }
  }

  // Collect albums
  for (auto post : xpath.query("//div[starts-with(@id, 'post-')]")) {
    Album album;

    // === Date === //
    std::string date = xpath.query_string("string(.//span[@class = 'd']/text())", post);
    if (! date.empty()) {
      struct tm tm = {0};
      ::strptime(date.c_str(), "%B %d, %Y", &tm);
      album.date = ::mktime(&tm);
    }

    // === Styles ===
    for (auto a : xpath.query(".//span[@class = 'style']//a", post)) {
      std::string url = a["href"]; // "http://.../style/<STYLE>/"
      if (! url.empty()) {
        if (url.back() == '/')
          url.pop_back();
        size_t idx = url.rfind('/');
        if (idx != std::string::npos)
          url.erase(0, idx+1);
        if (! url.empty())
          album.styles.push_back(Style(url, a.text()));
      }
    }

    // === Description ===
    // First <p> </p> is description
    album.description = xpath.query(".//p", post)[0].dump();
    album.description.erase(0, album.description.find('>')+1); // Remove first tag
    album.description.erase(album.description.rfind('<'));     // Remove last tag

    // === Cover URL ===
    album.cover_url = xpath.query(".//img[@class = 'cover']", post)[0]["src"];
    if (! album.cover_url.empty())
      album.cover_url = _basename(album.cover_url);

    // === Download count ===
    std::string download_count = xpath.query_string("string(.//span[@class = 'dc']//strong/text())");
    if (! download_count.empty()) { // TODO!
      std::string v = download_count;
      for (size_t pos; (pos = v.find(',')) != std::string::npos;)
        v.erase(pos, 1);
      album.download_count = std::stoi(v);
    }

    // === Album title and URL ===
    auto a_title = xpath.query(".//h1/a", post)[0];
    album.title = a_title.text();
    album.url   = _basename(a_title["href"]);

    // === Archive URLs ===
    // <span class="dll"><a href="(...)zip">MP3 Download</a>
    for (auto a : xpath.query(".//span[@class = 'dll']//a", post)) {
      std::string url = a["href"];
      if (! url.empty()) {
        url = _basename(url);
        album.archive_urls.push_back(url);
      }
    }

    // === Rating, Voting count ===
    // <span class="d">Rated <strong>89.10%</strong> with <strong>189</strong> 
    auto strongs = xpath.query(".//p[@class = 'postmetadata']//span[@class = 'd']//strong", post);
    try { album.rating = std::stof(strongs[0].text()); }
    catch (...) { /* XXX: handle_error("album.rating"); */ }
    try { album.votes = std::stoi(strongs[1].text()); }
    catch (...) { /* XXX: handle_error("album.votes");  */ }

    // === Direct mp3 track URLs ===
    // <script type="text/javascript"> (...) soundFile:"(...)"
    tracks.clear();
    tracks.reserve(10);
    for (auto script : xpath.query(".//script", post)) {
      std::string text = script.text();
      auto idx = text.find('"', text.find("soundFile:")); // soundFile:.*"
      if (idx != std::string::npos) {
        text = text.substr(idx + 1);
        text = text.substr(0, text.find('"'));
        text = base64_decode(text);
        boost::split(tracks, text, boost::is_any_of(","));

        if (tracks.size()) {
          for (auto &url : tracks)
            url.erase(0, url.rfind('/')+1); // basename
          break;
        }
      }
    }

    // === Assign metadata to track urls
    // - There may be multiple tracklists (evidence url?)
    auto trackit  = tracks.cbegin();
    auto trackend = tracks.cend();
    for (auto tracklist : xpath.query(".//div[@class = 'tl']", post)) {
      Track track;
      for (auto span : xpath.query(".//span", tracklist)) {
        switch (span["class"][0]) { // Luckily, class is only a char
          case 'n': if (! track.url.empty()) {
                      album.tracks.push_back(track);
                      track = Track();
                    }
                    if (trackit != trackend) {
                      track.url    = *trackit++;
                      track.number = std::stoi(span.text());
                    }
                    break;
          case 't': track.title  = span.text(); break;
          case 'r': track.remix  = span.text(); break;
          case 'a': track.artist = span.text(); break;
          case 'd': try {
                      std::string bpm = span.text(); // "(134 BPM)"
                      bpm.erase(0, bpm.find_first_of("0123456789"));
                      track.bpm = std::stoi(bpm);
                    } catch (const std::exception &e) {
                      // XXX handle_error(e);
                    }
                    break;
        }
      }

      if (! track.url.empty())
        album.tracks.push_back(track);
    }

    // Extract the artist name from the album title ("artist - album_name")
    // and set the artist on tracks that dont have it yet.
    size_t dash_len;
    size_t idx = find_dash(album.title, dash_len);
    if (idx != std::string::npos) {
      album.artist = album.title.substr(0, idx);
      album.title  = album.title.substr(idx + dash_len);
      strip(album.title,  ' ');
      strip(album.artist, ' ');
      for (auto &track : album.tracks)
        if (track.artist.empty())
          track.artist = album.artist;
    } else {
      for (auto &track : album.tracks)
        if (track.artist.empty())
          track.artist = "Unknown Artist";
    }

    // Fix missing track titles:
    // Sometimes the track title is merged into the track artist. ("artist - track")
    // Example: https://ektoplazm.com/free-music/gods-food
    for (auto &track : album.tracks)
      if (track.title.empty()) {
        size_t idx = find_dash(track.artist, dash_len);
        if (idx != std::string::npos) {
          track.title  = track.artist.substr(0, idx);
          track.artist = track.artist.substr(idx + dash_len);
          strip(track.title,  ' ');
          strip(track.artist, ' ');
        }
      }

    // Push back album
    albums.push_back(album);
  }
}

#ifdef TEST_BROWSEPAGE
int main() {
  // Testing of BrowsePage is done in Updater class
  return 0;
}
#endif
