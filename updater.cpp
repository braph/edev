#include "ektoplayer.hpp"
#include "browsepage.hpp"
#include "database.hpp"
#include "http.hpp"
// TODO: update, parameter pages (count)

class Updater {
private:
  Database *db;
  Statement insertAlbum;
  Statement insertTrack;
  Statement insertAlbum_Style;
  Statement insertArchive_Url;

public:
  Updater(Database *_db)
  : db(_db)
  , insertAlbum(_db,
      "REPLACE INTO album"
      "(id, url, title, date, cover_url, description, download_count, rating, votes) VALUES"
      "((SELECT id FROM album WHERE url=?),?,?,?,?,?,?,?,?)")
  , insertTrack(_db,
      "REPLACE INTO track"
      "(id, url, album_id, number, title, remix, artist, bpm) VALUES ("
      "(SELECT id FROM track WHERE url=?), ?,"
      "(SELECT id FROM album WHERE url=?),"
      "?,?,?,?,?)")
  , insertAlbum_Style(_db,
      "REPLACE INTO album_style"
      "(album_id, style_id) VALUES ("
      "(SELECT id FROM album WHERE url=?),"
      "(SELECT id FROM style WHERE url=?))")
  , insertArchive_Url(_db,
      "REPLACE INTO archive_url"
      "(album_id, url) VALUES"
      "((SELECT id FROM album WHERE url=?),?)")
  {
  }

  void update();
  void insert_album(const Album&);
  void insert_browsepage(const BrowsePage&);
};

#if 0
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
  Statement s(db,
      "REPLACE INTO style"
      "(id,url,name) VALUES"
      "((SELECT id FROM style WHERE url=?),?,?)"
  );
  for (auto &pair : firstPage.getStyles(src)) {
    s.bind(1, pair.first); // url
    s.bind(2, pair.first); // url
    s.bind(3, pair.second);  // name
    s.exec();
    s.reset();
  }

  // Insert first page
  insert_browsepage(firstPage);

  int num_pages = 0;
  std::string base_url = firstPage.getBaseUrl(src, &num_pages);

  // Insert other pages
  for (int i = 2; i <= num_pages; ++i) {
    std::string url = base_url + std::to_string(i); //firstPage.getPageUrl(i);
    BrowsePage browserPage(http.get(url));
    insert_browsepage(browserPage);
  }
}
#endif
void Updater :: insert_album(const Album& album) {
  Statement *s = &insertAlbum;
  s->bind(1, album.url);
  s->bind(2, album.url);
  s->bind(3, album.title);
  s->bind(4, (int) album.date);
  s->bind(5, album.cover_url);
  s->bind(6, album.description);
  s->bind(7, (int) album.download_count);
  s->bind(8, album.rating);
  s->bind(9, (int) album.votes);
  s->exec();
  s->reset();

  s = &insertAlbum_Style;
  s->bind(1, album.url);
  for (const auto &style : album.styles) {
    s->bind(2, style);
    s->exec();
    s->reset();
  }

  s = &insertArchive_Url;
  s->bind(1, album.url);
  for (const auto &url : album.archive_urls) {
    s->bind(2, url);
    s->exec();
    s->reset();
  }

  s = &insertTrack;
  s->bind(3, album.url);
  for (const auto &track : album.tracks) {
    s->bind(1, track.url);
    s->bind(2, track.url);
    s->bind(4, track.number);
    s->bind(5, track.title);
    s->bind(6, track.remix);
    s->bind(7, track.artist);
    s->bind(8, track.bpm);
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
