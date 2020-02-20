#include "database.hpp"

#include "common.hpp"

#include <cstring>
#include <cstdlib>
#include <unordered_map>
#define  streq(a,b) (!strcmp(a,b))
typedef const char* ccstr;

// TODO: find(const char *url, bool create): $create is not respected!

/* ============================================================================
 * Dumper / Loader
 * ============================================================================
 * Helpers for saving and reading containers to/from disk.
 * Binary format is:
 *   size_t elem_size  : Size of containers element
 *   size_t elem_count : Element count
 *   char   data[]     : Data (elem_size * elem_count)
 *   size_t elem_size  : Size of containers element  -.
 *   size_t elem_count : Element count               -'-> Used for validation
 */

struct Dumper {
  // Special
  static void dump(std::ofstream& fs, DynamicPackedVector& container) {
    size_t elem_size = container.bits();
    size_t elem_count = container.size();
    writeAtomic(fs, elem_size);
    writeAtomic(fs, elem_count);
    fs.write(reinterpret_cast<char*>(container.data()), container.data_size());
    writeAtomic(fs, elem_size);
    writeAtomic(fs, elem_count);
  }

  static void dump(std::ofstream& fs, StringPool& pool)    { _dump(fs, pool); }
  static void dump(std::ofstream& fs, std::vector<int>& v) { _dump(fs, v);    }

private:
  template<typename T> // vector, array, string, etc...
  static void _dump(std::ofstream& fs, T& container) {
    size_t elem_size = sizeof(*container.data());
    size_t elem_count = container.size();
    writeAtomic(fs, elem_size);
    writeAtomic(fs, elem_count);
    fs.write(reinterpret_cast<char*>(container.data()), elem_size * elem_count);
    writeAtomic(fs, elem_size);
    writeAtomic(fs, elem_count);
  }

  template<typename T>
  static inline void writeAtomic(std::ofstream& fs, T& value) {
    fs.write(reinterpret_cast<char*>(&value), sizeof(value));
  }
};

struct Loader {
  // Special
  void load(std::ifstream& fs, DynamicPackedVector& container) {
    readHeader(fs);
    container.push_back(1<<(elem_size-1)); // This adjusts bitwidth... TODO
    container.resize(elem_count);
    fs.read(reinterpret_cast<char*>(container.data()), container.data_size());
    size_t result;
    if (elem_size != readAtomic(fs, result))
      throw std::runtime_error("Validation failed: Element size in header != Element size in footer");
    if (elem_count != readAtomic(fs, result))
      throw std::runtime_error("Validation failed: Element count in header != Element count in footer");
  }

  void load(std::ifstream& fs, StringPool& pool)    { _load(fs, pool);  }
  void load(std::ifstream& fs, std::vector<int>& v) { _load(fs, v);     }

private:
  template<typename TContainer> // vector, array, string, etc...
  void _load(std::ifstream&fs, TContainer& container) {
    readHeader(fs);
    if (elem_size != sizeof(*container.data()))
      throw std::runtime_error("Element size in file != Element size of container");
    container.resize(elem_count);
    readData(fs, reinterpret_cast<char*>(container.data()));
  }

  size_t elem_size;
  size_t elem_count;

  void readHeader(std::ifstream& fs) {
    elem_size = elem_count = 0;
    readAtomic(fs, elem_size);
    readAtomic(fs, elem_count);
    if (elem_size == 0)
      throw std::runtime_error("Invalid element size (0) in header");
  }

  void readData(std::ifstream& fs, char* buf) {
    fs.read(buf, elem_size * elem_count);
    size_t result;
    if (elem_size != readAtomic(fs, result))
      throw std::runtime_error("Validation failed: Element size in header != Element size in footer");
    if (elem_count != readAtomic(fs, result))
      throw std::runtime_error("Validation failed: Element count in header != Element count in footer");
  }

  template<typename T>
  inline T readAtomic(std::ifstream& fs, T& var) {
    fs.read(reinterpret_cast<char*>(&var), sizeof(var));
    return var;
  }
};

/* ============================================================================
 * A bit ugly, but cleans up the code a lot
 * ==========================================================================*/
#define STR_SET(POOL_NAME, ID, S) \
  if (! streq(db.pool_##POOL_NAME.get(ID), S)) \
    { ID = db.pool_##POOL_NAME.add(S); }

#define STR_GET(POOL_NAME, ID) \
  db.pool_##POOL_NAME.get(ID)

/* ============================================================================
 * Database
 * ==========================================================================*/

Database :: Database()
: styles(*this)
, albums(*this)
, tracks(*this)
, pools({&pool_meta, &pool_desc, &pool_style_url, &pool_album_url,
    &pool_track_url, &pool_cover_url, &pool_archive_url})
{
  // Records with ID 0 represent a NULL value. Create them here.
  styles.find("", true);
  albums.find("", true);
  tracks.find("", true);
  assert(0 == styles.find("", true).id);
  assert(0 == albums.find("", true).id);
  assert(0 == tracks.find("", true).id);
}

void Database :: load(const std::string& file) {
  std::ifstream fs;
  fs.exceptions(std::ifstream::failbit|std::ifstream::badbit);
  fs.open(file, std::ios::binary);

  size_t abi_version = 0;
  fs.read(reinterpret_cast<char*>(&abi_version), sizeof(abi_version));
  if (abi_version != DB_ABI_VERSION)
    throw std::runtime_error("Database ABI version mismatch");

  for (auto pool : pools)
    Loader().load(fs, *pool);

  styles.load(fs);
  albums.load(fs);
  tracks.load(fs);
}

void Database :: save(const std::string& file) {
  std::ofstream fs;
  fs.exceptions(std::ofstream::failbit|std::ofstream::badbit);
  fs.open(file, std::ios::binary);

  size_t abi_version = DB_ABI_VERSION;
  fs.write(reinterpret_cast<char*>(&abi_version), sizeof(abi_version));

  for (auto p : pools)
    Dumper::dump(fs, *p);

  styles.save(fs);
  albums.save(fs);
  tracks.save(fs);
}

void Database :: shrink_to_fit() {
  shrink_pool_to_fit(pool_meta,
    {&styles.name, &albums.title, &albums.artist, &tracks.title, &tracks.artist, &tracks.remix});
}

void Database :: shrink_pool_to_fit(StringPool& pool, std::initializer_list<Column*> columns) {
  // Speed up a bit by avoiding StringPool::get()
  const char* data = pool.data();

  size_t nIDs = 0; // Average ID count
  for (auto c : columns) { nIDs += c->size(); }
  nIDs /= columns.size();

  // Build the map used for remapping IDs, storing all IDs used in columns
  std::unordered_map<int, int> idRemap;
  idRemap.reserve(nIDs);
  for (auto column : columns)
    for (auto id : *column)
      idRemap[id] = 0;

  // Sort the IDs, longest strings first
  std::vector<int> idSortedByLength;
  idSortedByLength.reserve(idRemap.size());
  for (auto& pair : idRemap)
    idSortedByLength.push_back(pair.first);

  std::sort(idSortedByLength.begin(), idSortedByLength.end(), [&](int a, int b)
      { return a != b && strlen(data + a) > strlen(data + b); });

  // Add strings in the right order to the stringpool and store the new ID
  StringPool newPool;
  newPool.reserve(pool.size());
  for (auto id : idSortedByLength)
    idRemap[id] = newPool.add(data + id);

  // Replace the IDs from the old pool by the IDs from the new pool
  for (auto column : columns)
    for (auto& id : *column)
      id = idRemap[id];

  // Transfer the pool
  pool = newPool;
}

/* ============================================================================
 * Database :: Table
 * ==========================================================================*/

void Database :: Table :: load(std::ifstream& fs) {
  for (auto* col : columns)
    Loader().load(fs, *col);
}

void Database :: Table :: save(std::ofstream& fs) {
  for (auto* col : columns)
    Dumper::dump(fs, *col);
}

/* Find a record by its URL or create one if it could not be found */
template<typename TTable>
static typename TTable::value_type find_or_create(TTable& table, StringPool& pool, const char* url) {
  bool newly_inserted = false;
  size_t strId = pool.add(url, &newly_inserted);
  Database::Column::iterator beg;
  Database::Column::iterator end;
  Database::Column::iterator fnd;
  if (newly_inserted)
    goto NOT_FOUND;
  beg = table.url.begin();
  end = table.url.end();
  fnd = std::find(beg, end, strId);
  if (fnd == end) {
NOT_FOUND:
    size_t pos = table.size();
    table.resize(pos+1);
    table.url[pos] = strId;
    return typename TTable::value_type(table.db, pos);
  } else {
    return typename TTable::value_type(table.db, fnd-beg);
  }
}

#define DB    Database
#define STYLE Database :: Styles :: Style
#define ALBUM Database :: Albums :: Album
#define TRACK Database :: Tracks :: Track

/* ============================================================================
 * Database :: Styles
 * ==========================================================================*/

STYLE Database::Styles::find(const char *url, bool create) {
  return find_or_create(*this, db.pool_style_url, url);
}

// GETTER
ccstr   STYLE::url()  const   { return STR_GET(style_url, db.styles.url[id]);  }
ccstr   STYLE::name() const   { return STR_GET(meta,      db.styles.name[id]); }
// SETTER
void    STYLE::url(ccstr s)   { STR_SET(style_url, db.styles.url[id],  s);     }
void    STYLE::name(ccstr s)  { STR_SET(meta,      db.styles.name[id], s);     }
// INDEX
DB::Field STYLE::operator[](DB::ColumnID id) const {
  switch (static_cast<DB::StyleColumnID>(id)) {
  case DB::STYLE_URL:   return DB::Field(url());
  case DB::STYLE_NAME:  return DB::Field(name());
  default:              return DB::Field(REPORT_BUG);
  }
}

/* ============================================================================
 * Database :: Albums
 * ==========================================================================*/

ALBUM Database::Albums::find(const char *url, bool create) {
  return find_or_create(*this, db.pool_album_url, url);
}

// GETTER
ccstr  ALBUM::url()            const { return STR_GET(album_url,  db.albums.url[id]);         }
ccstr  ALBUM::title()          const { return STR_GET(meta,       db.albums.title[id]);       }
ccstr  ALBUM::artist()         const { return STR_GET(meta,       db.albums.artist[id]);      }
ccstr  ALBUM::cover_url()      const { return STR_GET(cover_url,  db.albums.cover_url[id]);   }
ccstr  ALBUM::description()    const { return STR_GET(desc,       db.albums.description[id]); }
ccstr  ALBUM::archive_mp3_url()  const { return STR_GET(archive_url,  db.albums.archive_mp3[id]); }
ccstr  ALBUM::archive_wav_url()  const { return STR_GET(archive_url,  db.albums.archive_wav[id]); }
ccstr  ALBUM::archive_flac_url() const { return STR_GET(archive_url, db.albums.archive_flac[id]); }
time_t ALBUM::date()           const { return db.albums.date[id] * 60 * 60 * 24;      }
float  ALBUM::rating()         const { return static_cast<float>(db.albums.rating[id]) / 100; }
int    ALBUM::votes()          const { return db.albums.votes[id];                    }
int    ALBUM::download_count() const { return db.albums.download_count[id];           }
int    ALBUM::styles()         const { return db.albums.styles[id];                   }
// SETTER
void   ALBUM::url(ccstr s)          { STR_SET(album_url,    db.albums.url[id],         s); }
void   ALBUM::title(ccstr s)        { STR_SET(meta,         db.albums.title[id],       s); }
void   ALBUM::artist(ccstr s)       { STR_SET(meta,         db.albums.artist[id],      s); }
void   ALBUM::cover_url(ccstr s)    { STR_SET(cover_url,    db.albums.cover_url[id],   s); }
void   ALBUM::description(ccstr s)  { STR_SET(desc,         db.albums.description[id], s); }
void   ALBUM::archive_mp3_url(ccstr s)  { STR_SET(archive_url,  db.albums.archive_mp3[id], s); }
void   ALBUM::archive_wav_url(ccstr s)  { STR_SET(archive_url,  db.albums.archive_wav[id], s); }
void   ALBUM::archive_flac_url(ccstr s) { STR_SET(archive_url, db.albums.archive_flac[id], s); }
void   ALBUM::date(time_t t)        { db.albums.date[id] = t / 60 / 60 / 24;    }
void   ALBUM::rating(float i)       { db.albums.rating[id] = i * 100;           }
void   ALBUM::votes(int i)          { db.albums.votes[id] = i;                  }
void   ALBUM::download_count(int i) { db.albums.download_count[id] = i;         }
void   ALBUM::styles(int i)         { db.albums.styles[id] = i;                 }
// INDEX
DB::Field ALBUM::operator[](DB::ColumnID id) const {
  switch (static_cast<DB::AlbumColumnID>(id)) {
  case DB::ALBUM_URL:             return DB::Field(url());
  case DB::ALBUM_COVER_URL:       return DB::Field(cover_url());
  case DB::ALBUM_TITLE:           return DB::Field(title());
  case DB::ALBUM_ARTIST:          return DB::Field(artist());
  case DB::ALBUM_DESCRIPTION:     return DB::Field(description());
  case DB::ALBUM_DATE:            return DB::Field(date());
  case DB::ALBUM_RATING:          return DB::Field(rating());
  case DB::ALBUM_VOTES:           return DB::Field(votes());
  case DB::ALBUM_DOWNLOAD_COUNT:  return DB::Field(download_count());
  default:
    time_t stamp = date();
    struct tm t;
    localtime_r(&stamp, &t);
    switch (static_cast<DB::AlbumColumnID>(id)) {
    case DB::ALBUM_DAY:           return DB::Field(t.tm_mday);
    case DB::ALBUM_MONTH:         return DB::Field(t.tm_mon + 1);
    case DB::ALBUM_YEAR:          return DB::Field(t.tm_year + 1900);
    default:                      return DB::Field(REPORT_BUG);
    }
  }
}

/* ============================================================================
 * Database :: Tracks
 * ==========================================================================*/

TRACK Database::Tracks::find(const char* url, bool create) {
  return find_or_create(*this, db.pool_track_url, url);
}

// GETTER
ccstr TRACK::url()      const { return STR_GET(track_url, db.tracks.url[id]);    }
ccstr TRACK::title()    const { return STR_GET(meta,      db.tracks.title[id]);  }
ccstr TRACK::artist()   const { return STR_GET(meta,      db.tracks.artist[id]); }
ccstr TRACK::remix()    const { return STR_GET(meta,      db.tracks.remix[id]);  }
int   TRACK::number()   const { return db.tracks.number[id];                  }
int   TRACK::bpm()      const { return db.tracks.bpm[id];                     }
int   TRACK::album_id() const { return db.tracks.album_id[id];                }
ALBUM TRACK::album()    const { return db.albums[db.tracks.album_id[id]];     }
// SETTER
void  TRACK::url(ccstr s)     { STR_SET(track_url, db.tracks.url[id],    s);  }
void  TRACK::title(ccstr s)   { STR_SET(meta,      db.tracks.title[id],  s);  }
void  TRACK::artist(ccstr s)  { STR_SET(meta,      db.tracks.artist[id], s);  }
void  TRACK::remix(ccstr s)   { STR_SET(meta,      db.tracks.remix[id],  s);  }
void  TRACK::number(int i)    { db.tracks.number[id] = i;                     }
void  TRACK::bpm(int i)       { db.tracks.bpm[id] = i;                        }
void  TRACK::album_id(int i)  { db.tracks.album_id[id] = i;                   }
// INDEX
DB::Field TRACK::operator[](DB::ColumnID id) const {
  switch (static_cast<DB::TrackColumnID>(id)) {
  case DB::TRACK_TITLE:     return DB::Field(title());
  case DB::TRACK_ARTIST:    return DB::Field(artist());
  case DB::TRACK_REMIX:     return DB::Field(remix());
  case DB::TRACK_NUMBER:    return DB::Field(number());
  case DB::TRACK_BPM:       return DB::Field(bpm());
  default:                  return album()[id];
  }
}

// ============================================================================
#if TEST_DATABASE
#include "test.hpp"

template<typename T1, typename T2>
bool equals(T1& result, DB::ColumnID id, T2& vec) {
  if (result.size() != vec.size())
    return false;

  for (size_t i = 0; i < result.size(); ++i)
    if (result[i][id].value.s != vec[i])
      return false;

  return true;
}

int main () {
  TEST_BEGIN();
  Database db;
  db.load(TEST_DB);

  /* Test: ROW with ID 0 is actually empty */
  assert (! std::strlen(db.styles[0].url()));
  assert (! std::strlen(db.albums[0].url()));
  assert (! std::strlen(db.tracks[0].url()));

  auto styles = db.getStyles();
  auto albums = db.getAlbums();
  auto tracks = db.getTracks();

  if (db.tracks.size() < 100)
    throw std::runtime_error("Sorry, I need a database with some data, please run the updater ...");

  /* Test: Count of result set equals the number of table entries */
  assert (styles.size() == db.styles.size() - 1);
  assert (albums.size() == db.albums.size() - 1);
  assert (tracks.size() == db.tracks.size() - 1);
  /* Test: First row of database contains valid data */
  assert (std::strlen(db.styles[1].url()));
  assert (std::strlen(db.albums[1].url()));
  assert (std::strlen(db.tracks[1].url()));
  /* Test: First row of result set contains valid data */
  assert (std::strlen(styles[0].url()));
  assert (std::strlen(albums[0].url()));
  assert (std::strlen(tracks[0].url()));
  /* Test: The first row of a result set equals the first record of a table */
  assert (styles[0].url() == db.styles[1].url());
  assert (albums[0].url() == db.albums[1].url());
  assert (tracks[0].url() == db.tracks[1].url());
  /* Test: Last row of database contains valid data */
  assert (std::strlen(db.styles[db.styles.size() - 1].url()));
  assert (std::strlen(db.albums[db.albums.size() - 1].url()));
  assert (std::strlen(db.tracks[db.tracks.size() - 1].url()));
  /* Test: Last row of result set */
  assert (std::strlen(styles[styles.size() - 1].url()));
  assert (std::strlen(albums[albums.size() - 1].url()));
  assert (std::strlen(tracks[tracks.size() - 1].url()));

  /* Test: ORDER BY TRACK_TITLE ============================================ */
  std::vector<const char*> track_titles;
  for (auto track : tracks)
    track_titles.push_back(track.title());

  // Duplication succeded
  assert(equals(tracks, (Database::ColumnID) Database::TRACK_TITLE, track_titles));

  // Sorting succeeded
  std::sort(track_titles.begin(), track_titles.end(), [](const char* a, const char* b) { return strcmp(a, b) < 0; });
  tracks.order_by((Database::ColumnID) Database::TRACK_TITLE, Database::ASCENDING);
  assert(equals(tracks, (Database::ColumnID) Database::TRACK_TITLE, track_titles));


  /* Test: ORDER BY ALBUM_TITLE ============================================ */
  std::vector<const char*> album_titles;
  for (auto track : tracks)
    album_titles.push_back(track.album().title());

  // Duplication succeded
  assert(equals(tracks, (Database::ColumnID) Database::ALBUM_TITLE, album_titles));

  // Sorting succeeded
  std::sort(album_titles.begin(), album_titles.end(), [](const char* a, const char* b) { return strcmp(a, b) < 0; });
  tracks.order_by((Database::ColumnID) Database::ALBUM_TITLE, Database::ASCENDING);
  assert(equals(tracks, (Database::ColumnID) Database::ALBUM_TITLE, album_titles));


  /* Test: WHERE ALBUM_TITLE =============================================== */
  tracks.where((Database::ColumnID) Database::ALBUM_TITLE, Database::EQUAL, "Interbeing");
  assert(tracks.size());
  for (auto track : tracks)
    assert(streq(track.album().title(), "Interbeing"));

  /* Test: WHERE TRACK_TITLE =============================================== */
  tracks.where((Database::ColumnID) Database::TRACK_TITLE, Database::EQUAL, "Satori");
  assert(tracks.size() == 1);
  assert(streq(tracks[0].title(), "Satori"));

  /* Test: Failing to load a invalid database ============================== */
  except(db.load("/non-existent"));
  except(db.load("/bin/true"));

  TEST_END();
}
#endif
