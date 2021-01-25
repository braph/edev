#include "updater.hpp"

#include "ektoplayer.hpp"
#include "browsepage.hpp"
#include "database.hpp"
#include "markdown.hpp"

#include <lib/stringpack.hpp>
#include <lib/downloads.hpp>
#include <lib/string.hpp>

#include <cstring>

#define BROWSEPAGE_HTML_SIZE (60 * 1024) // bytes

static std::string& clean_str(std::string& s) {
  for (size_t pos; std::string::npos != (pos = s.find("  "));)
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
  erase_all(s, "https://");
  erase_all(s, "http://");
  erase_all(s, "www.");

  return s;
}

/* ============================================================================
 * Updater
 * ==========================================================================*/

Updater :: Updater(Database::Database &db) noexcept
  : _db(db)
{
}

void Updater :: fetch_page(int page, std::string&& recycle_buffer) noexcept {
  auto dl = new BufferDownload(Ektoplayer::browse_url(page));
  dl->setopt(CURLOPT_FOLLOWLOCATION, 1);
  dl->buffer() = std::move(recycle_buffer);
  dl->buffer().reserve(BROWSEPAGE_HTML_SIZE);
  dl->buffer().clear();

  _downloads.add_download(dl, [this,page](Download& dl_, CURLcode code) {
    auto& dl = static_cast<BufferDownload&>(dl_);
    log_write("%s: %s [%d]\n", dl.effective_url(), curl_easy_strerror(code), dl.http_code());

    if (code != CURLE_OK)
      fetch_page(page);
    else if (dl.http_code() == 404)
      _max_pages = std::min(_max_pages, page);
    else if (dl.http_code() == 200) {
      BrowsePageParser parser(dl.buffer());

      const int num_pages = parser.num_pages();
      if (num_pages > 0 && num_pages < _max_pages)
        _max_pages = num_pages;

      insert_browsepage(parser);

      int next_page = page + _downloads.parallel();
      if (next_page <= _max_pages)
        fetch_page(next_page, std::move(dl.buffer()));
    }

    return Downloads::Action::Remove;
  });
}

void Updater :: start(int pages) noexcept {
  log_write("Started database update (max %d pages)\n", pages);
  _max_pages = pages;

  if (! (_downloads.running_downloads() + _downloads.queued_downloads()))
    for (int i = 1; i <= _downloads.parallel(); ++i)
      fetch_page(i);
}

void Updater :: insert_album(Album& album) noexcept {
  // Album Styles =============================================================
  unsigned albumStyleIDs = 0;
  for (auto &style : album.styles) {
    Ektoplayer::url_shrink(style.url, EKTOPLAZM_STYLE_BASE_URL);
    auto styleRecord = _db.styles.find(style.url, true);
    if (! *(styleRecord.name()))
      styleRecord.name(style.name);
    albumStyleIDs |= (1U << (styleRecord.id - 1));
  }

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
  albumRecord.styles(int(albumStyleIDs));

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
    if (album.is_single_url) {
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

void Updater :: insert_browsepage(BrowsePageParser& parser) noexcept {
  for (;;) {
    try {
      Album album = parser.next_album();
      if (album.empty())
        return;
      insert_album(album);
    } catch (const std::exception& e) {
      log_write("%s\n", e);
    }
  }
}

#ifdef TEST_UPDATER
#include <lib/test.hpp>
#include <lib/cfile.hpp>
#include <lib/filesystem.hpp>
#define USE_FILESYSTEM 1
#define TESTDATA_DIR "/tmp/testdata" // Dir that contains HTML files
/* mkdir /tmp/testdata
 * cd /tmp/testdata
 * for i in $(seq 1 416); do curl "https://ektoplazm.com/section/free-music/page/$i" > $i; done
 */

using namespace std;

static void test_warning(const Database::Styles::Style& style, const char* msg)
{ printf("STYLE: %s: %s\n", style.url(), msg); }

static void test_warning(const Database::Albums::Album& album, const char* msg)
{ printf("ALBUM: %s: %s\n",  album.url(), msg); }

static void test_warning(const Database::Tracks::Track& track, const char* msg)
{ printf("TRACK: %s in album %s: %s\n", track.url(), track.album().url(), msg); }

#define warn_if(OBJECT, ...) \
  if (__VA_ARGS__) test_warning(OBJECT, #__VA_ARGS__)

static void read_file_into_string(const Filesystem::path& file, std::string& str) {
  static char buf[4*1024*1024]; buf[0] = '\0';
  buf[CFile::open(file, "r").read(buf, 1, sizeof(buf))] = '\0';
  str = buf;
}

// Updater tests *will replace* the existing database file!
// - Updater also covers some tests of BrowsePage and Database
int main() {
  TEST_BEGIN();

  Database::Database db;
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

  // Perform a database update ================================================
  Updater updater(db);
  updater.downloads().parallel(10);
#if defined(USE_FILESYSTEM) && USE_FILESYSTEM
  printf("Updating using filesystem ...\n");
  Filesystem::error_code e;

  string src;
  for (auto& f : Filesystem::directory_iterator(TESTDATA_DIR)) {
    read_file_into_string(f.path(), src);
    BrowsePageParser parser(src);
    updater.insert_browsepage(parser);
  }
#else
  printf("Updating using network ...\n");
  updater.start();
  while (updater.downloads().work() || updater.downloads().running_downloads()) { usleep(3000); }
#endif

  // Save the database and ensure that the amount of data is the same =========
  const size_t tracks_size = db.tracks.size();
  const size_t albums_size = db.albums.size();
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

  // Tests of BrowsePage ======================================================
  // - are all styles valid?
  for (auto style : db.get_styles()) {
    warn_if(style, strlen(style.url()) < 3);
    warn_if(style, strlen(style.name()) < 3);
  }
  // - are all tracks valid?
  for (auto track : db.get_tracks()) {
    warn_if(track, strlen(track.url()) < 3);
    warn_if(track, strlen(track.title()) < 1);
    warn_if(track, strlen(track.artist()) < 1);
  }
  // - are all albums valid?
  for (auto album : db.get_albums()) {
    warn_if(album, strlen(album.url()) < 3);
    warn_if(album, strlen(album.title()) < 1);
    warn_if(album, strlen(album.artist()) < 1);
  }

  // Print out defines ========================================================
  printf("#define EKTOPLAZM_STYLE_COUNT       %zu\n", db.styles.size());
  printf("#define EKTOPLAZM_ALBUM_COUNT       %zu\n", db.albums.size());
  printf("#define EKTOPLAZM_TRACK_COUNT       %zu\n", db.tracks.size());

  const struct { const char* define; StringChunk& chunk; }
  chunks[] = {
    {"EKTOPLAZM_META_SIZE       ",        db.chunk_meta},
    {"EKTOPLAZM_DESC_SIZE       ",        db.chunk_desc},
    {"EKTOPLAZM_STYLE_URL_SIZE  ",   db.chunk_style_url},
    {"EKTOPLAZM_ALBUM_URL_SIZE  ",   db.chunk_album_url},
    {"EKTOPLAZM_TRACK_URL_SIZE  ",   db.chunk_track_url},
    {"EKTOPLAZM_COVER_URL_SIZE  ",   db.chunk_cover_url},
    {"EKTOPLAZM_ARCHIVE_URL_SIZE", db.chunk_archive_url},
  };

  for (const auto& p : chunks)
    printf("#define %s % -10d // average length: %d\n",
        p.define, p.chunk.size(), p.chunk.size() / p.chunk.count());

  TEST_END();
}
#endif
