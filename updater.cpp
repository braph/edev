#include "updater.hpp"

#include "ektoplayer.hpp"
#include "browsepage.hpp"
#include "database.hpp"
#include "markdown.hpp"
#include "log.hpp"

#include "lib/stringpack.hpp"
#include "lib/downloads.hpp"
#include "lib/cstring.hpp"

#include <boost/algorithm/string/erase.hpp>

#include <cstring>

#define BROWSEPAGE_HTML_SIZE (60 * 1024) /* Bytes */

static std::string& clean_str(std::string& s) {
  size_t pos = 0;
  while (std::string::npos != (pos = s.find("  ", pos)))
    s.erase(pos, 1);
  return s;
}

static std::string make_markdown(const std::string& description) {
  std::string s = Html2Markdown::convert(description);

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

Updater :: Updater(Database::Database &db) noexcept
  : _db(db)
{
}

void Updater :: fetch_page(int page) noexcept {
  BufferDownload* dl = new BufferDownload(Ektoplayer::browse_url(page));
  dl->buffer().reserve(BROWSEPAGE_HTML_SIZE);

  dl->onFinished = [this,page](Download& dl, CURLcode code) {
    log_write("%s: %s [%d]\n", dl.lastURL(), curl_easy_strerror(code), dl.httpCode());

    if (code != CURLE_OK)
      fetch_page(page);
    else if (dl.httpCode() == 404)
      _max_pages = std::min(_max_pages, page);
    else if (dl.httpCode() == 200) {
      BrowsePageParser parser(static_cast<BufferDownload&>(dl).buffer());
      _max_pages = std::min(_max_pages, parser.num_pages());
      for (Album _; ! (_ = parser.next_album()).empty(); insert_album(_));

      int next_page = page + _downloads.parallel();
      if (next_page <= _max_pages)
        fetch_page(next_page);
    }
  };

  _downloads.addDownload(dl);
}

bool Updater :: start(int pages) noexcept {
  _max_pages = pages;

  if (! (_downloads.runningDownloads() + _downloads.queuedDownloads()))
    for (int i = 1; i <= _downloads.parallel(); ++i)
      fetch_page(i);

  return true;
}

void Updater :: insert_album(Album& album) {
  // Album Styles =============================================================
  Database::StylesArray albumStyleIDs;
  for (auto &style : album.styles) {
    Ektoplayer::url_shrink(style.url, EKTOPLAZM_STYLE_BASE_URL);
    auto styleRecord = _db.styles.find(style.url, true);
    if (! *(styleRecord.name()))
      styleRecord.name(style.name);
    albumStyleIDs.push_back(styleRecord.id);
  }
  if (albumStyleIDs.size() > 3)
    log_write("%s\n", album.url.c_str());
  // Move large IDs to the end, this compresses bitwidth of styleIDs.value
  std::sort(albumStyleIDs.begin(), albumStyleIDs.end(), std::greater<uint8_t>());

  // Album ====================================================================
  Ektoplayer::url_shrink(album.url, EKTOPLAZM_ALBUM_BASE_URL);
  Ektoplayer::url_shrink(album.cover_url, EKTOPLAZM_COVER_BASE_URL, ".jpg");
  album.description = make_markdown(album.description);

  auto albumRecord = _db.albums.find(album.url, true);
  albumRecord.title(clean_str(album.title));
  albumRecord.artist(clean_str(album.artist));
  albumRecord.cover_url(clean_str(album.cover_url));
  albumRecord.description(clean_str(album.description));
  albumRecord.date(album.date);
  albumRecord.rating(album.rating);
  albumRecord.votes(album.votes);
  albumRecord.download_count(album.download_count);
  albumRecord.styles(int(albumStyleIDs.data())); // XXX: cast silences warning

  // Album archive URLs =======================================================
  for (auto &u : album.archive_urls) {
    if (ends_with(u, "MP3.zip")) {
      Ektoplayer::url_shrink(u, EKTOPLAZM_ARCHIVE_BASE_URL, "MP3.zip");
      albumRecord.archive_mp3_url(u);
    }
    else if (ends_with(u, "WAV.rar")) {
      Ektoplayer::url_shrink(u, EKTOPLAZM_ARCHIVE_BASE_URL, "WAV.rar");
      albumRecord.archive_wav_url(u);
    }
    else if (ends_with(u, "FLAC.zip")) {
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
      char _[20];
      sprintf(_, ".mp3#%d", track.number);
      track.url += _;
    }

    auto trackRecord = _db.tracks.find(track.url, true);
    trackRecord.album_id(albumRecord.id);
    trackRecord.title(clean_str(track.title));
    trackRecord.artist(clean_str(track.artist));
    trackRecord.remix(clean_str(track.remix));
    trackRecord.number(track.number);
    trackRecord.bpm(track.bpm);
  }
}

void Updater :: insert_browsepage(const std::string& source) {
  BrowsePageParser parser(source);
  for (;;) {
    Album album = parser.next_album();
    if (album.empty())
      return;
    insert_album(album);
  }
}

#ifdef TEST_UPDATER
#include "lib/test.hpp"
#include "lib/filesystem.hpp"
#include <cstdio>
#include <fstream>
#include <streambuf>
#define USE_FILESYSTEM 0
#define TESTDATA_DIR "/tmp/testdata" // Dir that contains HTML files

using namespace std;

static void test_warning(const Database::Styles::Style& style, const char* msg)
{ printf("STYLE: %s: %s\n", style.url(), msg); }

static void test_warning(const Database::Albums::Album& album, const char* msg)
{ printf("ALBUM: %s: %s\n",  album.url(), msg); }

static void test_warning(const Database::Tracks::Track& track, const char* msg)
{ printf("TRACK: %s in album %s: %s", track.url(), track.album().url(), msg); }

#define warn_if(OBJECT, ...) \
  if (__VA_ARGS__) test_warning(OBJECT, #__VA_ARGS__)

static void read_file_into_string(const std::string& file, std::string& s) {
  s.clear();
  ifstream stream(file);
  stream.seekg(0, std::ios::end);
  std::streamoff size = stream.tellg();
  s.resize(size_t(size), '\0');
  stream.seekg(0);
  stream.read(&s[0], size);
}

// Updater tests *will replace* the existing database file!
// - Updater also covers some tests of BrowsePage and Database
int main() {
  TEST_BEGIN();
  Database::Database db;
  Downloads downloads;
  downloads.setParallel(10);

  db.styles.reserve(EKTOPLAZM_STYLE_COUNT);
  db.albums.reserve(EKTOPLAZM_ALBUM_COUNT);
  db.tracks.reserve(EKTOPLAZM_TRACK_COUNT);
  db.chunk_meta.reserve(EKTOPLAZM_META_SIZE);
  db.chunk_desc.reserve(EKTOPLAZM_DESC_SIZE);
  db.chunk_cover_url.reserve(EKTOPLAZM_COVER_URL_SIZE);
  db.chunk_album_url.reserve(EKTOPLAZM_ALBUM_URL_SIZE);
  db.chunk_track_url.reserve(EKTOPLAZM_TRACK_URL_SIZE);
  db.chunk_style_url.reserve(EKTOPLAZM_STYLE_URL_SIZE);
  db.chunk_archive_url.reserve(EKTOPLAZM_ARCHIVE_URL_SIZE);

  {
#ifdef USE_FILESYSTEM
    printf("Updating using filesystem ...\n");
    Updater u(db, downloads);
    Filesystem::error_code e;

    string src;
    for (auto& f : Filesystem::directory_iterator(TESTDATA_DIR)) {
      read_file_into_string(f.path().string(), src);
      u.insert_browsepage(src);
    }
#else
    printf("Updating using network ...\n");
    abort();
    Updater u(db, downloads);
    u.start(0);
    while (downloads.work()) { usleep(300 * 10); }
#endif
  }

  { // Save the database and ensure that the amount of data is the same
    size_t tracks_size = db.tracks.size();
    size_t albums_size = db.albums.size();
    printf("Inserted %zu tracks\n", tracks_size);
    printf("Inserted %zu albums\n", albums_size);
    printf("Saving database to " TEST_DB "\n");
    db.save(TEST_DB);
    { // Check if the data has not been altered
      Database::Database db2;
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
    warn_if(style, strlen(style.url()) < 3);
    warn_if(style, strlen(style.name()) < 3);
  }
  // - are all tracks valid?
  for (auto track : db.getTracks()) {
    warn_if(track, strlen(track.url()) < 3);
    warn_if(track, strlen(track.title()) < 1);
    warn_if(track, strlen(track.artist()) < 1);
  }
  // - are all albums valid?
  for (auto album : db.getAlbums()) {
    warn_if(album, strlen(album.url()) < 3);
    warn_if(album, strlen(album.title()) < 1);
    warn_if(album, strlen(album.artist()) < 1);
  }

  // Print out defines
  struct { const char* name; StringChunk& chunk; }
  chunks[] = {
    {"META",        db.chunk_meta},
    {"DESC",        db.chunk_desc},
    {"STYLE_URL",   db.chunk_style_url},
    {"ALBUM_URL",   db.chunk_album_url},
    {"TRACK_URL",   db.chunk_track_url},
    {"COVER_URL",   db.chunk_cover_url},
    {"ARCHIVE_URL", db.chunk_archive_url},
  };

  printf("#define EKTOPLAZM_STYLE_COUNT %zu\n", db.styles.size());
  printf("#define EKTOPLAZM_ALBUM_COUNT %zu\n", db.albums.size());
  printf("#define EKTOPLAZM_TRACK_COUNT %zu\n", db.tracks.size());

  for (const auto& p : chunks)
    printf("#define EKTOPLAZM_%s_SIZE %zu // average length: %zu\n",
        p.name, p.chunk.size(), p.chunk.size() / p.chunk.count());

  TEST_END();
}
#endif
