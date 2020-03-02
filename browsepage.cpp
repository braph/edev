#include "ektoplayer.hpp"
#include "browsepage.hpp"
#include "common.hpp"
#include "xml.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>

#include <string>
#include <algorithm>    

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
  // Get base url
  // -> https://ektoplazm.com/style/uplifting/page/2 (remove "/2")
  std::string base_url = xpath.query_string("string(//div[contains(@class, 'wp-pagenavi')]//a/@href)");
  if (! base_url.empty()) {
    base_url = base_url.substr(0, base_url.find_last_not_of("0123456789") + 1);
  }
#endif

  // Find number of pages (XXX Strange, this query fails with [0] ...)
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

    // Date
    std::string date = xpath.query_string("string(.//span[@class = 'd']/text())", post);
    if (! date.empty()) {
      struct tm tm = {0,0,0,0,0,0,0,0,0,0,0};
      ::strptime(date.c_str(), "%B %d, %Y", &tm);
      album.date = ::mktime(&tm);
    }

    // Styles
    for (auto a : xpath.query(".//span[@class = 'style']//a", post)) {
      std::string url = a["href"];
      if (! url.empty())
        album.styles.push_back(Style(url, a.nearestContent()));
    }

    // Description (first <p> </p>)
    album.description = xpath.query(".//p", post)[0].dump();
    album.description.erase(0, album.description.find('>')+1); // Remove first tag
    album.description.erase(album.description.rfind('<'));     // Remove last tag
    boost::trim(album.description);

    // Cover URL
    album.cover_url = xpath.query(".//img[@class = 'cover']", post)[0]["src"];

    // Download count
    std::string download_count = xpath.query_string("string(.//span[@class = 'dc']//strong/text())");
    if (! download_count.empty()) {
      std::string v = download_count;
      for (size_t pos; (pos = v.find(',')) != std::string::npos;)
        v.erase(pos, 1);
      album.download_count = std::atoi(v.c_str());
    }

    // Album title and URL
    auto a_title = xpath.query(".//h1/a", post)[0];
    album.url   = a_title["href"];
    album.title = a_title.text();
    boost::trim(album.title);

    // Archive URLs (<span class="dll"><a href="...zip">MP3 Download</a>)
    for (auto a : xpath.query(".//span[@class = 'dll']//a", post)) {
      std::string url = a["href"];
      if (! url.empty())
        album.archive_urls.push_back(url);
    }

    // Rating, Voting count
    // <span class="d">Rated <strong>89.10%</strong> with <strong>189</strong> 
    auto strongs = xpath.query(".//p[@class = 'postmetadata']//span[@class = 'd']//strong", post);
    album.rating = std::atof(strMayNULL(strongs[0].nearestContent()));
    album.votes  = std::atoi(strMayNULL(strongs[1].nearestContent()));

    // Direct mp3 track URLs
    // <script type="text/javascript"> soundFile:"..."
    tracks.clear();
    tracks.reserve(10);
    for (auto script : xpath.query(".//script", post)) {
      std::string text = script.text();
      size_t idx = text.find('"', text.find("soundFile:")); // soundFile:.*"
      if (idx != std::string::npos) {
        text = text.substr(idx + 1);
        text = text.substr(0, text.find('"'));
        text = base64_decode(text);
        boost::split(tracks, text, boost::is_any_of(","));

        if (tracks.size())
          break;
      }
    }

    // This should only happen on `.../dj-basilisk-the-colours-of-ektoplazm`
    if (tracks.empty())
      continue;

    // Assign metadata to track urls
    // - There may be multiple tracklists (evidence url?)
    auto trackit  = tracks.cbegin();
    auto trackend = tracks.cend();
    for (auto tracklist : xpath.query(".//div[@class = 'tl']", post)) {
      Track track;
      for (auto span : xpath.query(".//span", tracklist)) {
        switch (span["class"][0]) {
          case 'n':
            if (! track.url.empty()) {
              album.tracks.push_back(track);
              track = Track();
            }
            if (trackit != trackend) {
              track.url    = *trackit++;
              track.number = std::atoi(strMayNULL(span.nearestContent()));
            }
            break;
          case 't':
            track.title  = boost::trim_copy(span.text());
            break;
          case 'r':
            track.remix  = boost::trim_copy_if(span.text(), boost::is_any_of("\t ()"));
            break;
          case 'a':
            track.artist = boost::trim_copy(span.text());
            break;
          case 'd':
            const char* s = span.nearestContent(); // "(134 BPM)"
            std::sscanf(strMayNULL(s), "%*[^0-9]%hd", &track.bpm);
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
      boost::trim(album.title);
      boost::trim(album.artist);
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
          boost::trim(track.title);
          boost::trim(track.artist);
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
