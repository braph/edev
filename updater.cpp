#include "updater.hpp"
#include "ektoplayer.hpp"
#include <iostream>

static size_t curl_write_cb(char *data, size_t size, size_t nmemb, void *s) {
  ((std::string*) s)->append(data, size*nmemb);
  return size*nmemb;
}

/* ============================================================================
 * Fetcher
 * ==========================================================================*/

Fetcher :: Fetcher(int first, int last, int parallel) : alive(true) {
  curl_global_init(CURL_GLOBAL_ALL);
  if (! (curl_multi = curl_multi_init()))
    throw std::runtime_error("curl_multi_init");
  curl_multi_setopt(curl_multi, CURLMOPT_MAXCONNECTS, (long) parallel);
  thread = std::thread(&Fetcher::work, this, first, last, parallel);
}

Fetcher :: ~Fetcher() {
  curl_multi_cleanup(curl_multi);
  alive = false;
  if (thread.joinable())
    thread.join();
}

void Fetcher :: add_job(int page) {
  std::string url = "https://ektoplazm.com/section/free-music/page/" + std::to_string(page);
  CURL *curl = curl_easy_init();
  if (curl) {
    std::string *buffer = new std::string();
    buffer->reserve(50 * 1024);
    curl_easy_setopt(curl, CURLOPT_URL,             url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,   curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION,  1);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,       buffer);
    curl_easy_setopt(curl, CURLOPT_PRIVATE,         buffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,         20);
    curl_multi_add_handle(curl_multi, curl);
  }
}

void Fetcher :: work(int first, int last, int parallel) {
  CURLMsg *msg;
  int   msgs_left         = -1;
  int   abnormally_failed = 0;
  bool  got404            = false;
  int   page              = first;
  int   running_handles;

  while (parallel-- && page <= last)
    add_job(page++);

  do {
    curl_multi_perform(curl_multi, &running_handles);

    while ((msg = curl_multi_info_read(curl_multi, &msgs_left))) {
      CURL *curl = msg->easy_handle;
      const char *url = "<UNKNOWN URL>";
      std::string *buffer = NULL;
      if (curl) {
        curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE,    &buffer);
        curl_easy_getinfo(msg->easy_handle, CURLINFO_EFFECTIVE_URL, &url);
      }

      if (msg->msg == CURLMSG_DONE) {
        if (msg->data.result == CURLE_OK) {
          long code = 0;
          curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &code);
          if (code == 200) {
            std::lock_guard<std::mutex> _(lock);
            results.push(buffer);
            buffer = NULL;
          } else {
            got404 = true;
          }
          std::cerr << url << ": " << code << std::endl;
        } else {
          std::cerr << url << ": " << curl_easy_strerror(msg->data.result) << std::endl;
          ++abnormally_failed;
        }

        curl_multi_remove_handle(curl_multi, curl);
        curl_easy_cleanup(curl);
      }
      else {
        std::cerr << url << ": " << msg->msg << std::endl;
        ++abnormally_failed;
      }

      if (buffer)
        delete buffer;

      if (!got404 && abnormally_failed < 5 && page <= last)
        add_job(page++);
    }

    if (running_handles)
      curl_multi_wait(curl_multi, NULL, 0, 1000, NULL);

  } while (alive && running_handles);

  alive = false;
}

std::string* Fetcher :: pop() {
  std::lock_guard<std::mutex> _(lock);
  std::string* result = NULL;
  if (results.size()) {
    result = results.front();
    results.pop();
  }
  return result;
}

/* ============================================================================
 * Updater
 * ==========================================================================*/

Updater :: Updater(Database &db)
  : db(db)
  , fetcher(NULL)
{
}

Updater :: ~Updater() {
  if (fetcher)
    delete fetcher;
}

bool Updater :: start(int pages) {
  if (fetcher) {
    if (fetcher->alive)
      return false; // There is an ongoing update
    else
      delete fetcher;
    fetcher = NULL;
  }

  // Retrieve the first page
  std::string source;
  CURL *curl = curl_easy_init();
  if (! curl) {
    std::cerr << __func__ << "E: curl_easy_init()" << std::endl;
    return false;
  }
  curl_easy_setopt(curl, CURLOPT_URL, EKTOPLAZM_BROWSE_BASE_URL);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &source);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
  CURLcode e = curl_easy_perform(curl);
  curl_easy_cleanup(curl);
  if (e != CURLE_OK) {
    std::cerr << __func__ << "E: " << curl_easy_strerror(e) << std::endl;
    return false;
  }

  BrowsePage page(source);
  insert_styles(page.getStyles(source)); // Styles needed! TODO
  insert_browsepage(page);
  int num_pages = 0; // TODO
  std::string _ = page.getBaseUrl(source, &num_pages);

  int firstPage, lastPage;

  if (pages > 0) { // Count from first page
    firstPage = pages;
    lastPage  = (num_pages ? num_pages : INT_MAX);
  }
  else if (pages < 0) { // Count from last page
    lastPage  = (num_pages ? num_pages : INT_MAX);
    firstPage = lastPage + pages;
  }
  else { // All pages
    firstPage = 2;
    lastPage  = (num_pages ? num_pages : INT_MAX);
  }

  std::cerr << __func__ << ": first=" << firstPage << " last= " << lastPage << std::endl;
  try { fetcher = new Fetcher(firstPage, lastPage, 10); } catch (...) {}
  return true;
}

bool Updater :: write_to_database() {
  if (! fetcher)
    return false;

  bool had_results = false;
  std::string *source;
  while ((source = fetcher->pop())) {
    had_results = true;
    insert_browsepage(BrowsePage(*source));
    delete source;
  }

  if (!had_results && !fetcher->alive) {
    delete fetcher;
    fetcher = NULL;
  }

  return !!fetcher;
}

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
  a.title(album.title.c_str());
  a.artist(album.artist.c_str());
  a.cover_url(album.cover_url.c_str());
  a.description(album.description.c_str());
  a.date(album.date);
  a.rating(album.rating);
  a.votes(album.votes);
  a.download_count(album.download_count);
  a.styles(styles);

  for (const auto &u : album.archive_urls)
    if (std::string::npos != u.rfind("MP3.zip"))
      a.archive_mp3_url(u.c_str());
    else if (std::string::npos != u.rfind("WAV.rar"))
      a.archive_wav_url(u.c_str());
    else if (std::string::npos != u.rfind("FLAC.zip"))
      a.archive_flac_url(u.c_str());

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

#if PERFORMANCE_TEST
#include <fstream>
#include <streambuf>
  for (int i = 1; i <= 416; ++i) { // XXX-4
    std::ifstream t(std::string("/tmp/testdata/") + std::to_string(i));
    std::string src((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());

    BrowsePage browserPage(src);
    if (i == 1)
      insert_styles(browserPage.getStyles(src));
    insert_browsepage(browserPage);
  }
#endif

#include "test.hpp"
int main() {
  unlink(TEST_DB);
  std::cout << TEST_DB << std::endl;
  size_t tracks_size;
  size_t albums_size;
  char*  track_url;
  char*  album_desc;
  {
    Database db;
    Updater u(db);
    u.start(0);
    while (u.write_to_database()) { sleep(1); }
    tracks_size = db.tracks.size();
    albums_size = db.albums.size();
    track_url   = strdup(db.tracks[tracks_size-1].url());
    album_desc  = strdup(db.albums[albums_size-1].description());
    std::cout << "Inserted " << tracks_size << " tracks." << std::endl;
    std::cout << "Inserted " << albums_size << " albums." << std::endl;
    db.save(TEST_DB); // Write the database!
  }

  Database db;
  db.load(TEST_DB); // This should succeed.
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

  //curl_global_cleanup();
}
#endif
