#include "updater.hpp"

#include "xml.hpp"
#include "database.hpp"
#include "downloads.hpp"
#include "ektoplayer.hpp"
#include "browsepage.hpp"

#include <boost/algorithm/string/replace.hpp>

#include <iostream>

struct Html2Markup {
  std::string result;

  static std::string convert(const std::string& html, const char* encoding) {
    XmlDoc doc = HtmlDoc::readDoc(html.c_str(), NULL, encoding,
        HTML_PARSE_RECOVER|HTML_PARSE_NOERROR|HTML_PARSE_NOWARNING|HTML_PARSE_COMPACT);
    XmlNode root = doc.getRootElement();
    Html2Markup p;
    p.result.reserve(html.size() / 4);
    p.parse(root);
    return p.result;
  }

  void parse(const XmlNode& _node) {
    const char* tag;
    XmlNode node = _node;

    for (; node; node = node.next()) {
      switch (node.type()) {
        case XML_ELEMENT_NODE:
          tag = node.name();

          if (!strcmp(tag, "a")) {
            write("((");
            parse(node.children());
            write("))[[");
            write(node["href"]);
            write("]]");
          }
          else if (!strcmp(tag, "strong") || !strcmp(tag, "b")) {
            write("**");
            parse(node.children());
            write("**");
          }
          else if (!strcmp(tag, "em") || !strcmp(tag, "i")) {
            write("__");
            parse(node.children());
            write("__");
          }
          else if (!strcmp(tag, "br")) {
            write('\n');
          }
          else {
            parse(node.children());
          }

          break;

        case XML_TEXT_NODE:
          write(node.content());
          parse(node.children());
          break;

        default:
          parse(node.children());
      }
    }
  }

  template<typename T> inline void write(T value) {
    result += value;
  }
};

static std::string& clean_str(std::string& s) {
  size_t pos = 0;
  while (std::string::npos != (pos = s.find("  ", pos)))
    s.erase(pos, 1);
  return s;
}

static std::string makeMarkup(const std::string& description) {
  std::string s = Html2Markup::convert(description, "UTF-8");

  // Removing `www.` *may* corrupt some URLs, but saves 20KB!
  boost::algorithm::erase_all(s, "www.");
  boost::algorithm::erase_all(s, "http://");
  boost::algorithm::erase_all(s, "https://");

  // Replace protected email links:
  //  /cdn-cgi/l/email-protection#284b47444146684747474c06464d5c
  const char* protectedLink = "[[/cdn-cgi/l/email";
  size_t pos = 0;
  while (std::string::npos != (pos = s.find(protectedLink, pos))) {
    size_t end = s.find(']', pos);
    if (end != std::string::npos)
      s.replace(pos, end-pos, "[[@");
  }

  return s;
}

/* ============================================================================
 * Updater
 * ==========================================================================*/

Updater :: Updater(Database &db, Downloads &downloads)
  : db(db)
  , downloads(downloads)
{
}

bool Updater :: start(int pages) {
  // Not the best way to determine if we're already updating, but sufficient...
  if (downloads.queue().size() > 30)
    return true;

  std::function<void(Download&, CURLcode)> cb = [this](Download& _dl, CURLcode code) {
    BufferDownload &dl = static_cast<BufferDownload&>(_dl);
    if (code == CURLE_OK && dl.httpCode() == 200) {
      BrowsePage page(dl.buffer());
      this->insert_browsepage(page);
    }
    std::cerr << dl.lastURL() << ": " << curl_easy_strerror(code) << " [" << dl.httpCode() << "]\n";
  };

  // Retrieve the first page
  BufferDownload dl(Ektoplayer::browse_url(1));
  dl.onFinished = cb;
  CURLcode e = dl.perform();
  if (e != CURLE_OK)
    return false;

  BrowsePage page(dl.buffer());
  int num_pages = page.num_pages;
  int firstPage, lastPage;

#define LAST_PAGE_FALLBACK 450 // TODO

  if (pages > 0) { // Count from first page
    firstPage = pages;
    lastPage  = (num_pages ? num_pages : LAST_PAGE_FALLBACK);
  }
  else if (pages < 0) { // Count from last page
    lastPage  = (num_pages ? num_pages : LAST_PAGE_FALLBACK);
    firstPage = lastPage + pages;
  }
  else { // All pages
    firstPage = 2;
    lastPage  = (num_pages ? num_pages : LAST_PAGE_FALLBACK);
  }

  std::string url;
  while (firstPage <= lastPage) {
    url = Ektoplayer::browse_url(firstPage++);
    BufferDownload* dl = new BufferDownload(url);
    dl->onFinished = cb;
    downloads.addDownload(dl, Downloads::LOW);
  }

  return true;
}

void Updater :: insert_album(Album& album) {
  // Album Styles =============================================================
  Database::StylesArray albumStyleIDs;
  for (auto &style : album.styles) {
    Ektoplayer::url_shrink(style.url, EKTOPLAZM_STYLE_BASE_URL);
    auto styleRecord = db.styles.find(style.url.c_str(), true);
    if (! *(styleRecord.name()))
      styleRecord.name(style.name.c_str());
    albumStyleIDs.push_back(styleRecord.id);
  }
  if (albumStyleIDs.size() > 3)
    std::cerr << album.url << '\n';
  // Move large IDs to the end, this compresses bitwidth of styleIDs.value
  std::sort(albumStyleIDs.begin(), albumStyleIDs.end(), std::greater<uint8_t>());

  // Album ====================================================================
  Ektoplayer::url_shrink(album.url, EKTOPLAZM_ALBUM_BASE_URL);
  Ektoplayer::url_shrink(album.cover_url, EKTOPLAZM_COVER_BASE_URL, ".jpg");
  album.description = makeMarkup(album.description);
  clean_str(album.title);
  clean_str(album.artist);
  clean_str(album.cover_url);
  clean_str(album.description);

  auto albumRecord = db.albums.find(album.url.c_str(), true);
  albumRecord.title(album.title.c_str());
  albumRecord.artist(album.artist.c_str());
  albumRecord.cover_url(album.cover_url.c_str());
  albumRecord.description(album.description.c_str());
  albumRecord.date(album.date);
  albumRecord.rating(album.rating);
  albumRecord.votes(album.votes);
  albumRecord.download_count(album.download_count);
  albumRecord.styles(albumStyleIDs.data());

  // Album archive URLs =======================================================
  for (auto &u : album.archive_urls) {
    if (std::string::npos != u.rfind("MP3.zip")) {
      Ektoplayer::url_shrink(u, EKTOPLAZM_ARCHIVE_BASE_URL, "MP3.zip");
      albumRecord.archive_mp3_url(u.c_str());
    }
    else if (std::string::npos != u.rfind("WAV.rar")) {
      Ektoplayer::url_shrink(u, EKTOPLAZM_ARCHIVE_BASE_URL, "WAV.rar");
      albumRecord.archive_wav_url(u.c_str());
    }
    else if (std::string::npos != u.rfind("FLAC.zip")) {
      Ektoplayer::url_shrink(u, EKTOPLAZM_ARCHIVE_BASE_URL, "FLAC.zip");
      albumRecord.archive_flac_url(u.c_str());
    }
  }

  // Tracks ===================================================================
  for (auto &track : album.tracks) {
    Ektoplayer::url_shrink(track.url, EKTOPLAZM_TRACK_BASE_URL, ".mp3");
    clean_str(track.title);
    clean_str(track.artist);
    clean_str(track.remix);

    auto trackRecord = db.tracks.find(track.url.c_str(), true);
    trackRecord.album_id(albumRecord.id);
    trackRecord.title(track.title.c_str());
    trackRecord.artist(track.artist.c_str());
    trackRecord.remix(track.remix.c_str());
    trackRecord.number(track.number);
    trackRecord.bpm(track.bpm);
  }
}

void Updater :: insert_browsepage(BrowsePage& page) {
  for (auto &album : page.albums)
    insert_album(album);
}

#ifdef TEST_UPDATER
#include "test.hpp"

#ifdef USE_LOCAL_FILES
#include <fstream>
#include <streambuf>
  for (int i = 1; i <= 416; ++i) {
    std::ifstream t(std::string("/tmp/testdata/") + std::to_string(i));
    std::string src((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());

    BrowsePage browserPage(src);
    insert_browsepage(browserPage);
  }
#endif

void test_warning(const Database::Styles::Style& style, const char* reason) {
  std::cout << "STYLE: " << style.url() << ": " << reason << '\n';
}

void test_warning(const Database::Albums::Album& album, const char *reason) {
  std::cout << "ALBUM: " << album.url() << ": " << reason << '\n';
}

void test_warning(const Database::Tracks::Track& track, const char* reason) {
  std::cout << "TRACK: " << track.url() << " in album " << track.album().url() << ": " << reason << '\n';
}

int main() {
  TEST_BEGIN();

#define DO_UPDATE 1
#if DO_UPDATE
  // This also covers some testing of the Database class ======================
  unlink(TEST_DB);
  std::cout << TEST_DB << '\n';
  size_t tracks_size;
  size_t albums_size;
  std::string track_url;
  std::string album_desc;
  {
    Database db;
    Downloads downloads(10);
    Updater u(db, downloads);
    u.start(0);
    while (downloads.work()) { usleep(1000 * 10); }
    tracks_size = db.tracks.size();
    albums_size = db.albums.size();
    track_url   = db.tracks[tracks_size-1].url();
    album_desc  = db.albums[albums_size-1].description();
    std::cout << "Inserted " << tracks_size << " tracks." << '\n';
    std::cout << "Inserted " << albums_size << " albums." << '\n';
    db.save(TEST_DB); // Test saving the database
  }
#endif

  Database db;
  db.load(TEST_DB); // Test loading the database
#if DO_UPDATE
  assert(tracks_size == db.tracks.size());
  assert(albums_size == db.albums.size());
  assert(track_url   == db.tracks[tracks_size-1].url());
  assert(album_desc  == db.albums[albums_size-1].description());
#endif
  
  // Testing shrink_to_fit()
  {
    Database db2;
    db2.load(TEST_DB);
    db2.shrink_to_fit();
    for (size_t i = 0; i < db.tracks.size(); ++i)
      assert(streq(db.tracks[i].title(), db2.tracks[i].title()));
    for (size_t i = 0; i < db.albums.size(); ++i)
      assert(streq(db.albums[i].title(), db2.albums[i].title()));
  }

  // Tests of BrowsePage ======================================================
  // - are all styles valid?
  for (auto style : db.getStyles()) {
    if (std::strlen(style.url()) < 3)
      test_warning(style, "URL < 3");
    if (std::strlen(style.name()) < 3)
      test_warning(style, "NAME < 3");
  }
  // - are all tracks valid?
  for (auto track : db.getTracks()) {
    if (std::strlen(track.url()) < 3)
      test_warning(track, "URL < 3");
    if (std::strlen(track.title()) < 1)
      test_warning(track, "TITLE < 1");
    if (std::strlen(track.artist()) < 1)
      test_warning(track, "ARTIST < 1");
  }
  // - are all albums valid?
  for (auto album : db.getAlbums()) {
    if (std::strlen(album.url()) < 3)
      test_warning(album, "URL < 3");
    if (std::strlen(album.title()) < 1)
      test_warning(album, "TITLE < 1");
#if 0
    if (std::strlen(album.artist()) < 1)
      test_warning(album, "ARTIST < 1");
#endif
  }

  // Print out defines
  std::vector<std::pair<const char*, StringPool*>> pools = {
    {"META",        &db.pool_meta},
    {"DESC",        &db.pool_desc},
    {"STYLE_URL",   &db.pool_style_url},
    {"ALBUM_URL",   &db.pool_album_url},
    {"TRACK_URL",   &db.pool_track_url},
    {"COVER_URL",   &db.pool_cover_url},
    {"ARCHIVE_URL", &db.pool_archive_url},
  };

#if 0 /* Dump content of string pools */
  for (auto pair : pools) {
    char* data = pair.second->data();
    for (size_t i = pair.second->size() - 1; i--;)
      if (data[i] == '\0')
        std::cout << pair.first << ':' << &data[i+1] << '\n';
  }
#endif

  std::cout
    << "#define EKTOPLAZM_STYLE_COUNT " << db.styles.size() << '\n';
    << "#define EKTOPLAZM_ALBUM_COUNT " << db.albums.size() << '\n';
    << "#define EKTOPLAZM_TRACK_COUNT " << db.tracks.size() << '\n';

  for (auto pair : pools) {
    size_t n_strings = 0;
    char* data = pair.second->data();
    for (size_t i = pair.second->size(); i--;)
      n_strings += (data[i] == '\0');

    std::cout
      << "#define EKTOPLAZM_" << pair.first << "_SIZE " << pair.second->size()
      << " // average lenth: " << pair.second->size() / n_strings << '\n';
  }

  TEST_END();
}
#endif
