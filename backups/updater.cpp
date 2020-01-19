#include "browsepage.hpp"
#include "database.hpp"
#include "http.hpp"
#include "ektoplayer.hpp"
// TODO: update, parameter pages (count)

class Updater {
private:
  Database *db;

public:
  Updater(Database *_db)
  : db(_db)
  { }

  void update();
  void insert_album(const Album&);
  void insert_browsepage(const BrowsePage&);
};

void Updater :: update() {
  SimpleHTTP http;
  std::string src = http.get(EKTOPLAZM_BROWSE_BASE_URL);
  BrowsePage firstPage(src);

  // Styles first!
  const char* sql = "REPLACE INTO styles (style,url) VALUES (?,?)";
  for (auto &pair : firstPage.getStyles(src)) {
    Statement s(db, sql);
    s.bind(1, pair.first);
    s.bind(2, pair.second);
    s.exec();
  }

  // Insert first page
  insert_browsepage(firstPage);

  // Insert other pages
  for (unsigned int i = 2; i <= firstPage.num_pages; ++i) {
    std::string url = firstPage.getPageUrl(i);
    src = http.get(url);
    BrowsePage browserPage(src);
    insert_browsepage(browserPage);
  }
}

void Updater :: insert_album(const Album& album) {
  {
    Statement s(db, "REPLACE INTO albums "
      "(url, title, date, cover_url, description, download_count, rating, votes)"
      " VALUES (?,?,?,?,?,?,?,?)");
    unsigned int c=0/*lumn*/;
    s.bind(++c, album.url);
    s.bind(++c, album.title);
    s.bind(++c, album.date);
    s.bind(++c, album.cover_url);
    s.bind(++c, album.description);
    s.bind(++c, album.download_count);
    s.bind(++c, album.rating);
    s.bind(++c, album.votes);
    s.exec();
  }

  {
    for (const auto &style : album.styles) {
      Statement s(db, "REPLACE INTO albums_styles (album_url,style) VALUES (?,?)");
      s.bind(1, album.url);
      s.bind(2, style);
      s.exec();
    }
  }

  {
    const char *sql = "REPLACE INTO archive_urls (album_url, archive_type, archive_url) VALUES (?,?,?)";
    /*
     album[:archive_urls].each do |type, url|
        @db.replace_into(:archive_urls, {
           album_url:    album[:url],
           archive_type: type,
           archive_url:  url
        })
     end
    */
  }

  {
    for (const auto &track : album.tracks) {
      Statement s(db, "REPLACE INTO tracks"
                      " (url, album_url, number, title, remix, artist, bpm)"
                      " VALUES (?,?,?,?,?,?,?)");
      unsigned int c=0/*lumn*/;
      s.bind(++c, track.url);
      s.bind(++c, album.url);
      s.bind(++c, track.number);
      s.bind(++c, track.title);
      s.bind(++c, track.remix);
      s.bind(++c, track.artist);
      s.bind(++c, track.bpm);
      s.exec();
    }
  }
}

void Updater :: insert_browsepage(const BrowsePage& bp) {
  for (const auto &album : bp.albums)
    insert_album(album);
}

#if TEST_UPDATER
int main() {
  Database db("/tmp/test.db");
  Updater u(&db);
  u.update();
}
#endif
