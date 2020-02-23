#include "updater.hpp"

#include "ektoplayer.hpp"
#include "database.hpp"
#include "browsepage.hpp"
#include "downloads.hpp"

#include <iostream>

static std::string& clean_description(std::string& desc) {
  size_t pos;

  /* Strip those garbage attributes off the description. */
  for (const char* search : { "href=\"/cdn-cgi/l/email", "class=\"__cf_email__", "data-cfemail=\""}) {
    while (std::string::npos != (pos = desc.find(search))) {
      size_t end = desc.find('"', pos+std::strlen(search));
      if (end != std::string::npos)
        desc.erase(pos, end-pos+1);
    }
  }

  /* Save space by using shorter tags and trimming whitespace */
  for (const char* search : {"strong>\0b>", "em>\0i>", "  \0 ", " >\0>"}) {
    size_t search_len = strlen(search);
    const char* replace = search + search_len + 1;
    while (std::string::npos != (pos = desc.find(search)))
      desc.replace(pos, search_len, replace);
  }

  return desc;
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
  // TODO: dont start the download multiple times

  std::function<void(Download&, CURLcode)> cb = [this](Download& _dl, CURLcode code) {
    BufferDownload &dl = static_cast<BufferDownload&>(_dl);
    if (code == CURLE_OK && dl.httpCode() == 200) {
      BrowsePage page(dl.getContent());
      this->insert_browsepage(page);
    }
  };

  // Retrieve the first page
  BufferDownload dl(Ektoplayer::browse_url(1));
  dl.onFinished = cb;
  CURLcode e = dl.perform();
  if (e != CURLE_OK) {
    std::cerr << __func__ << "E: " << curl_easy_strerror(e) << std::endl;
    return false;
  }

  BrowsePage page(dl.getContent());
  int num_pages = page.num_pages;
  int firstPage, lastPage;

#define LAST_PAGE_FALLBACK 450

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
    Download* dl = new BufferDownload(url);
    dl->onFinished = cb;
    downloads.addDownload(dl, Downloads::LOW);
  }

  return true;
}

void Updater :: insert_album(Album& album) {
  int32_t styles = 0;

  for (auto &style : album.styles) {
    Ektoplayer::url_shrink(style.url, EKTOPLAZM_STYLE_BASE_URL, NULL);

    auto row = db.styles.find(style.url.c_str(), true);
    if (! *(row.name()))
      row.name(style.name.c_str());
    styles <<= 8; // TODO: move assignment logic got Database.cpp
    styles += row.id;
  }

  Ektoplayer::url_shrink(album.url, EKTOPLAZM_ALBUM_BASE_URL, NULL);
  Ektoplayer::url_shrink(album.cover_url, EKTOPLAZM_COVER_BASE_URL, ".jpg");
  clean_description(album.description);

  auto a = db.albums.find(album.url.c_str(), true);
  a.title(album.title.c_str());
  a.artist(album.artist.c_str());
  a.cover_url(album.cover_url.c_str());
  a.description(album.description.c_str());
  a.date(album.date);
  a.rating(album.rating);
  a.votes(album.votes);
  a.download_count(album.download_count);
  a.styles(styles);

  for (auto &u : album.archive_urls) {
    if (std::string::npos != u.rfind("MP3.zip")) {
      Ektoplayer::url_shrink(u, EKTOPLAZM_ARCHIVE_BASE_URL, "MP3.zip");
      a.archive_mp3_url(u.c_str());
    }
    else if (std::string::npos != u.rfind("WAV.rar")) {
      Ektoplayer::url_shrink(u, EKTOPLAZM_ARCHIVE_BASE_URL, "WAV.rar");
      a.archive_wav_url(u.c_str());
    }
    else if (std::string::npos != u.rfind("FLAC.zip")) {
      Ektoplayer::url_shrink(u, EKTOPLAZM_ARCHIVE_BASE_URL, "FLAC.zip");
      a.archive_flac_url(u.c_str());
    }
  }

  for (auto &track : album.tracks) {
    Ektoplayer::url_shrink(track.url, EKTOPLAZM_TRACK_BASE_URL, ".mp3");

    auto t = db.tracks.find(track.url.c_str(), true);
    t.album_id(a.id);
    t.title(track.title.c_str());
    t.artist(track.artist.c_str());
    t.remix(track.remix.c_str());
    t.number(track.number);
    t.bpm(track.bpm);
  }
}

void Updater :: insert_browsepage(BrowsePage& page) {
  for (auto &album : page.albums)
    insert_album(album);
}

#if TEST_UPDATER

#if PERFORMANCE_TEST
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

#include "test.hpp"

void test_warning(const Database::Styles::Style& style, const char* reason) {
  std::cout << "STYLE: " << style.url() << ": " << reason << std::endl;
}

void test_warning(const Database::Albums::Album& album, const char *reason) {
  std::cout << "ALBUM: " << album.url() << ": " << reason << std::endl;
}

void test_warning(const Database::Tracks::Track& track, const char* reason) {
  std::cout << "TRACK: " << track.url() << " in album " << track.album().url() << ": " << reason << std::endl;
}

int main() {
  TEST_BEGIN();

#define DO_UPDATE 1
#if DO_UPDATE
  // This also covers some testing of the Database class ======================
  unlink(TEST_DB);
  std::cout << TEST_DB << std::endl;
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
    std::cout << "Inserted " << tracks_size << " tracks." << std::endl;
    std::cout << "Inserted " << albums_size << " albums." << std::endl;
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
        std::cout << pair.first << ':' << &data[i+1] << std::endl;
  }
#endif

  std::cout
    << "#define EKTOPLAZM_STYLE_COUNT " << db.styles.size() << std::endl
    << "#define EKTOPLAZM_ALBUM_COUNT " << db.albums.size() << std::endl
    << "#define EKTOPLAZM_TRACK_COUNT " << db.tracks.size() << std::endl;

  for (auto pair : pools) {
    size_t n_strings = 0;
    char* data = pair.second->data();
    for (size_t i = pair.second->size(); i--;)
      n_strings += (data[i] == '\0');

    std::cout
      << "#define EKTOPLAZM_" << pair.first << "_SIZE " << pair.second->size()
      << " // average lenth: " << pair.second->size() / n_strings << std::endl;
  }

  TEST_END();
}
#endif
