#include "ektoplayer.hpp"
#include "browsepage.hpp"
#include "database.hpp"
#include "http.hpp"
// TODO: update, parameter pages (count)

class Updater {
private:
  Database &db;
public:
  Updater(Database &db) : db(db) { }
  void update();
  void insert_album(const Album&);
  void insert_styles(const std::map<std::string, std::string>&);
  void insert_browsepage(const BrowsePage&);
};

#if 1
#include <fstream>
#include <streambuf>
void Updater :: update() {
  for (int i = 1; i <= 416; ++i) { // XXX-4
    std::ifstream t(std::string("/tmp/testdata/") + std::to_string(i));
    std::string src((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());

    BrowsePage browserPage(src);
    if (i == 1)
      insert_styles(browserPage.getStyles(src));
    insert_browsepage(browserPage);
  }
}
#else
void Updater :: update() {
  SimpleHTTP http;
  std::string src = http.get(EKTOPLAZM_BROWSE_BASE_URL);
  BrowsePage firstPage(src);

  // Insert styles first, as the albums link to them
  insert_styles(firstPage.getStyles(src));

  // Insert first page
  insert_browsepage(firstPage);

  int num_pages = 0;
  std::string base_url = firstPage.getBaseUrl(src, &num_pages);

  // Insert other pages
  for (int i = 2; i <= num_pages; ++i) {
    std::string url = base_url + std::to_string(i); //firstPage.getPageUrl(i);
    BrowsePage browserPage(http.get(url));
    insert_browsepage(browserPage);
    std::cout << "inserting " << i << " of " << num_pages << std::endl;
  }
}

#endif
void Updater :: insert_styles(const std::map<std::string, std::string> &styles) {
  for (auto &pair : styles) {
    auto style = db.styles.find(pair.first.c_str(), true);
    style.name(pair.second.c_str());
  }
}

void Updater :: insert_album(const Album& album) {
  int32_t styles = 0;
  for (const auto &style : album.styles) {
    auto r = db.styles.find(style.c_str(), false); // TODO: if valid
    styles <<= 8;
    styles += r.id;
  }

  auto a = db.albums.find(album.url.c_str(), true);
  //r.url(album.url.c_str());
  a.title(album.title.c_str());
  a.artist(album.artist.c_str());
  a.cover_url(album.cover_url.c_str());
  a.description(album.description.c_str());
  a.date(album.date);
  a.rating(album.rating);
  a.votes(album.votes);
  a.download_count(album.download_count);
  a.styles(styles);

  /*
  s = &insertArchive_Url;
  s->bind(1, album.url);
  for (const auto &url : album.archive_urls) {
    s->bind(2, url);
    s->exec();
    s->reset();
  }
  */

  for (const auto &track : album.tracks) {
    auto t = db.tracks.find(track.url.c_str(), true);
    t.album_id(a.id);
    t.title(track.title.c_str());
    t.artist(track.artist.c_str());
    t.remix(track.remix.c_str());
    t.number(track.number);
    t.bpm(track.bpm);
  }
}

void Updater :: insert_browsepage(const BrowsePage& page) {
  for (const auto &album : page.albums)
    insert_album(album);
}

#if TEST_UPDATER
#include <unistd.h>
#include <cassert>
#include <cstring>
#include <iostream>
#define TEST_DB "/tmp/ektoplayer-test.db"
int main() {
  unlink(TEST_DB);
  std::cout << TEST_DB << std::endl;
  size_t tracks_size;
  size_t albums_size;
  char*  track_url;
  char*  album_desc;
  {
    Database db(TEST_DB);
    db.load(); // Loading a non-existend DB should be fine
    db.pool.reserve(500 * 1024);
    db.pool_url.reserve(500 * 1024);
    db.pool_desc.reserve(1500 * 1024);
    db.styles.reserve(20);
    db.albums.reserve(2000);
    db.tracks.reserve(14000);
    Updater u(db);
    u.update();
    tracks_size = db.tracks.size();
    albums_size = db.albums.size();
    track_url   = strdup(db.tracks[tracks_size-1].url());
    album_desc  = strdup(db.albums[albums_size-1].description());
    std::cout << "Inserted " << tracks_size << " tracks." << std::endl;
    std::cout << "Inserted " << albums_size << " albums." << std::endl;
    db.save(); // Write the database!
  }

  Database db(TEST_DB);
  db.load(); // This should succeed.
  assert(tracks_size == db.tracks.size());
  assert(albums_size == db.albums.size());
  assert(!strcmp(track_url,  db.tracks[tracks_size-1].url()));
  assert(!strcmp(album_desc, db.albums[albums_size-1].description()));
  return 0;

  for (auto style : db.getStyles())
    std::cout << style.url() << '|' << style.name() << std::endl;

  return 0;
  auto albums = db.getAlbums();
  albums.order_by(Database::ALBUM_URL, Database::ASCENDING);
  return 0;

  for (auto a : albums) {
    time_t date = a.date();
    struct tm* tm = localtime(&date);
    char sdate[20];
    ::strftime(sdate, sizeof(sdate), "%Y-%m-%d 00:00:00", tm);
    std::cout << a.url() << '|' << a.title() << '|' << a.rating() << '|' << sdate << std::endl;
  }

  albums.where(Database::ALBUM_URL, Database::EQUAL, "zis0ky-into-the-abyss");

  for (auto a : albums) {
    time_t date = a.date();
    struct tm* tm = localtime(&date);
    char sdate[20];
    ::strftime(sdate, sizeof(sdate), "%Y-%m-%d 00:00:00", tm);
    std::cout << a.url() << '|' << a.title() << '|' << a.rating() << '|' << sdate << std::endl;
  }
  
  return 0;

  auto a = albums[0];
  std::cout << a.url() << '|' << a.title() << '|' << a.rating() << '|' << std::endl;
  a = albums[1];
  std::cout << a.url() << '|' << a.title() << '|' << a.rating() << '|' << std::endl;
  a = std::move(albums[3]);
  std::cout << a.url() << '|' << a.title() << '|' << a.rating() << '|' << std::endl;

  /*
  for (auto track : db.getTracks())
    std::cout << track.url() << '|' << track.title() << track.album().rating() << std::endl;
  */
}
#endif
