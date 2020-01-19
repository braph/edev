#include "ektoplayer.hpp"
#include "browsepage.hpp"
#include "database.hpp"
#include "http.hpp"
// TODO: update, parameter pages (count)

class Updater {
private:
  Database *db;
  Statement insertAlbums;
  Statement insertAlbums_Styles;
  Statement insertTracks;

public:
  Updater(Database *_db)
  : db(_db)
  , insertAlbums(_db,
      "REPLACE INTO albums"
      " (url, title, date, cover_url, description, download_count, rating, votes)"
      " VALUES (?,?,?,?,?,?,?,?)")
  , insertAlbums_Styles(_db,
      "REPLACE INTO albums_styles"
      " (album_url,style)"
      " VALUES (?,?)")
  , insertTracks(_db,
      "REPLACE INTO tracks"
      " (url, album_url, number, title, remix, artist, bpm)"
      " VALUES (?,?,?,?,?,?,?)")
  {
  }

  void update();
  void insert_album(const Album&);
  void insert_browsepage(const BrowsePage&);
};

#if 1
#include <fstream>
#include <streambuf>
void Updater :: update() {
  for (int i = 2; i < 416; ++i) {
    std::ifstream t(std::string("/tmp/testdata/") + std::to_string(i));
    std::string src((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());

    BrowsePage browserPage(src);
    insert_browsepage(browserPage);
  }
}
#else
void Updater :: update() {
  SimpleHTTP http;
  std::string src = http.get(EKTOPLAZM_BROWSE_BASE_URL);
  BrowsePage firstPage(src);

  // Insert styles first, as the albums link to them
  Statement s(db, "REPLACE INTO styles (style,url) VALUES (?,?)");
  for (auto &pair : firstPage.getStyles(src)) {
    s.bind(1, pair.first);
    s.bind(2, pair.second);
    s.exec();
    s.reset();
  }

  // Insert first page
  insert_browsepage(firstPage);

  int num_pages = 0;
  std::string base_url = firstPage.getBaseUrl(src, &num_pages);

  // Insert other pages
  for (unsigned int i = 2; i <= num_pages; ++i) {
    std::string url = base_url + std::to_string(i); //firstPage.getPageUrl(i);
    BrowsePage browserPage(http.get(url));
    insert_browsepage(browserPage);
  }
}
#endif
void Updater :: insert_album(const Album& album) {
  Statement *s = &insertAlbums;
  s->bind(1, album.url);
  s->bind(2, album.title);
  s->bind(3, album.date);
  s->bind(4, album.cover_url);
  s->bind(5, album.description);
  s->bind(6, (int) album.download_count);
  s->bind(7, album.rating);
  s->bind(8, (int) album.votes);
  s->exec();
  s->reset();

  s = &insertAlbums_Styles;
  s->bind(1, album.url);
  for (const auto &style : album.styles) {
    s->bind(2, style);
    s->exec();
    s->reset();
  }

  const char *sql = "REPLACE INTO archive_urls (album_url, archive_type, archive_url) VALUES (?,?,?)"; /*
  album[:archive_urls].each do |type, url| {
      @db.replace_into(:archive_urls, {
         album_url:    album[:url],
         archive_type: type,
         archive_url:  url
      })
  } */

  s = &insertTracks;
  s->bind(2, album.url);
  for (const auto &track : album.tracks) {
    s->bind(1, track.url);
    s->bind(3, track.number);
    s->bind(4, track.title);
    s->bind(5, track.remix);
    s->bind(6, track.artist);
    s->bind(7, track.bpm);
    s->exec();
    s->reset();
  }
}

void Updater :: insert_browsepage(const BrowsePage& page) {
  db->begin_transaction();
  for (const auto &album : page.albums)
    insert_album(album);
  db->end_transaction();
}

#if TEST_UPDATER
#include <unistd.h>
#include <iostream>
#define TEST_DB "/tmp/ektoplayer-test.db"
int main() {
  unlink(TEST_DB);
  Database db(TEST_DB);
  Updater u(&db);
  u.update();
  std::cout << TEST_DB << std::endl;
  std::cout << "Inserted " << db.track_count() << " tracks." << std::endl;
  std::cout << "Inserted " << db.album_count() << " albums." << std::endl;
}
#endif
