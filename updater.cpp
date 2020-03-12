#include "updater.hpp"

#include "xml.hpp"
#include "database.hpp"
#include "downloads.hpp"
#include "ektoplayer.hpp"
#include "browsepage.hpp"

#include <boost/algorithm/string/erase.hpp>

#include <iostream>

struct Html2Markup {
  std::string result;

  static std::string convert(const std::string& html, const char* encoding) {
    Xml::Doc doc = Html::readDoc(html, NULL, encoding,
        HTML_PARSE_RECOVER|HTML_PARSE_NOERROR|HTML_PARSE_NOWARNING|HTML_PARSE_COMPACT);
    Xml::Node root = doc.getRootElement();
    Html2Markup p;
    p.result.reserve(html.size() / 4);
    p.parse(root);
    return p.result;
  }

  void parse(const Xml::Node& _node) {
    const char* tag;
    Xml::Node node = _node;

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

  // Replace protected email links:
  //  [[/cdn-cgi/l/email-protection#284b47444174...]]
  // by
  //  [[@]]
  size_t pos = 0;
  while (std::string::npos != (pos = s.find("/cdn-cgi/l/email", pos))) {
    size_t end = s.find(']', pos);
    if (end != std::string::npos)
      s.replace(pos, end-pos, 1, '@');
  }

  // Removing `www.` *may* corrupt some URLs, but saves 20KB!
  boost::algorithm::erase_all(s, "https://");
  boost::algorithm::erase_all(s, "http://");
  boost::algorithm::erase_all(s, "www.");

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
    auto styleRecord = db.styles.find(style.url, true);
    if (! *(styleRecord.name()))
      styleRecord.name(style.name);
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

  auto albumRecord = db.albums.find(album.url, true);
  albumRecord.title(clean_str(album.title));
  albumRecord.artist(clean_str(album.artist));
  albumRecord.cover_url(clean_str(album.cover_url));
  albumRecord.description(clean_str(album.description));
  albumRecord.date(album.date);
  albumRecord.rating(album.rating);
  albumRecord.votes(album.votes);
  albumRecord.download_count(album.download_count);
  albumRecord.styles(albumStyleIDs.data());

  // Album archive URLs =======================================================
  for (auto &u : album.archive_urls) {
    if (std::string::npos != u.rfind("MP3.zip")) {
      Ektoplayer::url_shrink(u, EKTOPLAZM_ARCHIVE_BASE_URL, "MP3.zip");
      albumRecord.archive_mp3_url(u);
    }
    else if (std::string::npos != u.rfind("WAV.rar")) {
      Ektoplayer::url_shrink(u, EKTOPLAZM_ARCHIVE_BASE_URL, "WAV.rar");
      albumRecord.archive_wav_url(u);
    }
    else if (std::string::npos != u.rfind("FLAC.zip")) {
      Ektoplayer::url_shrink(u, EKTOPLAZM_ARCHIVE_BASE_URL, "FLAC.zip");
      albumRecord.archive_flac_url(u);
    }
  }

  // Tracks ===================================================================
  for (auto &track : album.tracks) {
    Ektoplayer::url_shrink(track.url, EKTOPLAZM_TRACK_BASE_URL, ".mp3");

    // The track URL is used as a primary key. If an album has only one file
    // we need to create a unique URL for each track.
    if (album.isSingleURL) {
      track.url += ".mp3#";
      track.url += std::to_string(track.number);
    }

    auto trackRecord = db.tracks.find(track.url, true);
    trackRecord.album_id(albumRecord.id);
    trackRecord.title(clean_str(track.title));
    trackRecord.artist(clean_str(track.artist));
    trackRecord.remix(clean_str(track.remix));
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
#include "filesystem.hpp"
#include <fstream>
#include <streambuf>
#define USE_FILESYSTEM 0
#define TESTDATA_DIR "/tmp/testdata" // Dir that contains HTML files

void test_warning(const Database::Styles::Style& style, const char* reason) {
  std::cout << "STYLE: " << style.url() << ": " << reason << '\n';
}

void test_warning(const Database::Albums::Album& album, const char *reason) {
  std::cout << "ALBUM: " << album.url() << ": " << reason << '\n';
}

void test_warning(const Database::Tracks::Track& track, const char* reason) {
  std::cout << "TRACK: " << track.url() << " in album " << track.album().url() << ": " << reason << '\n';
}

// Updater tests *will replace* the existing database file!
// - Updater also covers some tests of BrowsePage and Database
int main() {
  TEST_BEGIN();
  Database db;
  Downloads downloads(10);

  db.styles.reserve(EKTOPLAZM_STYLE_COUNT);
  db.albums.reserve(EKTOPLAZM_ALBUM_COUNT);
  db.tracks.reserve(EKTOPLAZM_TRACK_COUNT);
  db.pool_meta.reserve(EKTOPLAZM_META_SIZE);
  db.pool_desc.reserve(EKTOPLAZM_DESC_SIZE);
  db.pool_cover_url.reserve(EKTOPLAZM_COVER_URL_SIZE);
  db.pool_album_url.reserve(EKTOPLAZM_ALBUM_URL_SIZE);
  db.pool_track_url.reserve(EKTOPLAZM_TRACK_URL_SIZE);
  db.pool_style_url.reserve(EKTOPLAZM_STYLE_URL_SIZE);
  db.pool_archive_url.reserve(EKTOPLAZM_ARCHIVE_URL_SIZE);

  {
#ifdef USE_FILESYSTEM
    std::cout << "Updating using filesystem ...\n";
    Updater u(db, downloads);
    Filesystem::error_code e;

    std::string src;
    for (auto& f : Filesystem::directory_iterator(TESTDATA_DIR)) {
      std::ifstream stream(f.path().string());
      src.clear();
      src.append((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

      BrowsePage bp(src);
      u.insert_browsepage(bp);
    }
#else
    std::cout << "Updating using network ...\n";
    abort();
    Updater u(db, downloads);
    u.start(0);
    while (downloads.work()) { usleep(300 * 10); }
#endif
  }

  { // Save the database and ensure that the amount of data is the same
    size_t tracks_size = db.tracks.size();
    size_t albums_size = db.albums.size();
    std::cout << "Inserted " << tracks_size << " tracks." << '\n';
    std::cout << "Inserted " << albums_size << " albums." << '\n';
    std::cout << "Saving database to " << TEST_DB << '\n';
    db.save(TEST_DB);
    { // Check if the data has not been altered
      Database db2;
      db2.load(TEST_DB);
      assert(tracks_size == db2.tracks.size());
      assert(albums_size == db2.albums.size());
      assert(streq(db.tracks[tracks_size-1].url(),         db2.tracks[tracks_size-1].url()));
      assert(streq(db.albums[albums_size-1].description(), db2.albums[albums_size-1].description()));
    }
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
  struct name_pool { const char* name; StringPool* ptr; };
  std::vector<name_pool> pools = {
    {"META",        &db.pool_meta},
    {"DESC",        &db.pool_desc},
    {"STYLE_URL",   &db.pool_style_url},
    {"ALBUM_URL",   &db.pool_album_url},
    {"TRACK_URL",   &db.pool_track_url},
    {"COVER_URL",   &db.pool_cover_url},
    {"ARCHIVE_URL", &db.pool_archive_url},
  };

  std::cout
    << "#define EKTOPLAZM_STYLE_COUNT " << db.styles.size() << '\n'
    << "#define EKTOPLAZM_ALBUM_COUNT " << db.albums.size() << '\n'
    << "#define EKTOPLAZM_TRACK_COUNT " << db.tracks.size() << '\n';

  for (auto pool : pools)
    std::cout
      << "#define EKTOPLAZM_" << pool.name << "_SIZE " << pool.ptr->size()
      << " // average lenth: " << pool.ptr->size() / pool.ptr->count() << '\n';

  TEST_END();
}
#endif
