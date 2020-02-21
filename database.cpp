#include "database.hpp"

#include "common.hpp"

#include <cstdlib>
#include <climits>
#include <cstring>
#include <unordered_map>

#define  streq(a,b) (!strcmp(a,b))
#define  bitsOf(T) (CHAR_BIT*sizeof(T))

typedef const char* ccstr;

// TODO: find(const char *url, bool create): $create is not respected!

/* ============================================================================
 * Dumper / Loader
 * ============================================================================
 * Helpers for saving and reading containers to/from disk.
 * Binary format is:
 *   size_t elem_bits  : Size of a containers element
 *   size_t elem_count : Element count
 *   void*  data[]     : Data
 *   size_t elem_bits  : Size of a containers element  -.
 *   size_t elem_count : Element count                 -'-> Used for validation
 */

struct Dumper {
  Dumper(std::ofstream& fs) : fs(fs) {}

  template<typename T>
  void dump(std::vector<T>& v)
  { dump(bitsOf(T), v.size(), reinterpret_cast<char*>(v.data())); }

  void dump(DynamicPackedVector& v)
  { dump(v.bits(), v.size(), reinterpret_cast<char*>(v.data())); }

  void dump(StringPool& p)
  { dump(bitsOf(char), p.size(), reinterpret_cast<char*>(p.data())); }

private:
  std::ofstream& fs;
  void dump(size_t bits, size_t count, char* data) {
    fs.write(reinterpret_cast<char*>(&bits),  sizeof(bits));
    fs.write(reinterpret_cast<char*>(&count), sizeof(count));
    fs.write(data, size_for_bits(bits*count));
    fs.write(reinterpret_cast<char*>(&bits),  sizeof(bits));
    fs.write(reinterpret_cast<char*>(&count), sizeof(count));
  }
};

struct Loader {
  Loader(std::ifstream& fs) : fs(fs) {}

  void load(DynamicPackedVector& vec) {
    readHeader();
    vec.reserve(elem_count, elem_bits);
    vec.resize(elem_count);
    readData(reinterpret_cast<char*>(vec.data()));
  }

  void load(StringPool& pool) {
    readHeader(bitsOf(char));
    pool.resize(elem_count);
    readData(reinterpret_cast<char*>(pool.data()));
  }

  template<typename T>
  void load(std::vector<T>& v) {
    readHeader(bitsOf(T));
    v.resize(elem_count);
    readData(reinterpret_cast<char*>(v.data()));
  }

private:
  std::ifstream& fs;
  size_t elem_bits;
  size_t elem_count;

  void readHeader(size_t expected_bits = 0) {
    elem_bits = elem_count = 0xDEAD;
    fs.read(reinterpret_cast<char*>(&elem_bits),  sizeof(elem_bits));
    fs.read(reinterpret_cast<char*>(&elem_count), sizeof(elem_count));
    if (elem_bits == 0 || elem_bits == 0xDEAD || (expected_bits && elem_bits != expected_bits))
      throw std::runtime_error("Invalid bit count in header");
  }

  void readData(char* buf) {
    size_t check_bits, check_count;
    fs.read(buf, size_for_bits(elem_bits*elem_count));
    fs.read(reinterpret_cast<char*>(&check_bits),  sizeof(check_bits));
    fs.read(reinterpret_cast<char*>(&check_count), sizeof(check_count));
    if (check_bits != elem_bits || check_count != elem_count)
      throw std::runtime_error("Validation failed: invalid bit/element count in footer");
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
    Loader(fs).load(*pool);

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
    Dumper(fs).dump(*p);

  styles.save(fs);
  albums.save(fs);
  tracks.save(fs);
}

void Database :: shrink_to_fit() {
  shrink_pool_to_fit(pool_meta, {&styles.name, &albums.title,
      &albums.artist, &tracks.title, &tracks.artist, &tracks.remix});
}

void Database :: shrink_pool_to_fit(StringPool& pool, std::initializer_list<Column*> columns) {
  const char* data = pool.data(); // Avoid StringPool::get()

  size_t nIDs = 0; // Average ID count
  for (auto col : columns) { nIDs += col->size(); }
  nIDs /= columns.size();

  // Build the map used for remapping IDs, storing all IDs used in columns
  std::unordered_map<int, int> idRemap;
  idRemap.reserve(nIDs);
  for (auto col : columns)
    for (auto id : *col)
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
    for (Column::iterator it = column->begin(); it != column->end(); ++it)
      *it = idRemap[*it];
    //for (auto& id : *column)
    //  id = idRemap[id];

  // Transfer the pool
  pool = newPool;
}

/* ============================================================================
 * Database :: Table
 * ==========================================================================*/

void Database :: Table :: load(std::ifstream& fs) {
  for (auto* col : columns)
    Loader(fs).load(*col);
}

void Database :: Table :: save(std::ofstream& fs) {
  for (auto* col : columns)
    Dumper(fs).dump(*col);
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


// ============================================================================
#define _ Database :: Styles :: Style // ======================================
// ============================================================================

STYLE Database::Styles::find(const char *url, bool create) {
  return find_or_create(*this, db.pool_style_url, url);
}

// GETTER
ccstr   _::url()  const   { return STR_GET(style_url, db.styles.url[id]);  }
ccstr   _::name() const   { return STR_GET(meta,      db.styles.name[id]); }
// SETTER
void    _::url(ccstr s)   { STR_SET(style_url, db.styles.url[id],  s);     }
void    _::name(ccstr s)  { STR_SET(meta,      db.styles.name[id], s);     }
// INDEX
DB::Field _::operator[](DB::ColumnID id) const {
  switch (static_cast<DB::StyleColumnID>(id)) {
  case DB::STYLE_URL:   return DB::Field(url());
  case DB::STYLE_NAME:  return DB::Field(name());
  default:              return DB::Field(REPORT_BUG);
  }
}

#undef _


// ============================================================================
#define _ Database :: Albums :: Album // ======================================
// ============================================================================

ALBUM Database::Albums::find(const char *url, bool create) {
  return find_or_create(*this, db.pool_album_url, url);
}

#define SHRINK_DATE(T) (T / 60 / 60 / 24 - 10000)
#define EXPAND_DATE(T) ((T + 10000) * 60 * 60 * 24)

// GETTER
ccstr  _::url()              const { return STR_GET(album_url,   db.albums.url[id]);         }
ccstr  _::title()            const { return STR_GET(meta,        db.albums.title[id]);       }
ccstr  _::artist()           const { return STR_GET(meta,        db.albums.artist[id]);      }
ccstr  _::cover_url()        const { return STR_GET(cover_url,   db.albums.cover_url[id]);   }
ccstr  _::description()      const { return STR_GET(desc,        db.albums.description[id]); }
ccstr  _::archive_mp3_url()  const { return STR_GET(archive_url, db.albums.archive_mp3[id]); }
ccstr  _::archive_wav_url()  const { return STR_GET(archive_url, db.albums.archive_wav[id]); }
ccstr  _::archive_flac_url() const { return STR_GET(archive_url, db.albums.archive_flac[id]);}
time_t _::date()             const { return EXPAND_DATE(db.albums.date[id]);                 }
float  _::rating()           const { return static_cast<float>(db.albums.rating[id]) / 100;  }
int    _::votes()            const { return db.albums.votes[id];                             }
int    _::download_count()   const { return db.albums.download_count[id];                    }
int    _::styles()           const { return db.albums.styles[id];                            }
// SETTER
void   _::url(ccstr s)              { STR_SET(album_url,    db.albums.url[id],         s); }
void   _::title(ccstr s)            { STR_SET(meta,         db.albums.title[id],       s); }
void   _::artist(ccstr s)           { STR_SET(meta,         db.albums.artist[id],      s); }
void   _::cover_url(ccstr s)        { STR_SET(cover_url,    db.albums.cover_url[id],   s); }
void   _::description(ccstr s)      { STR_SET(desc,         db.albums.description[id], s); }
void   _::archive_mp3_url(ccstr s)  { STR_SET(archive_url,  db.albums.archive_mp3[id], s); }
void   _::archive_wav_url(ccstr s)  { STR_SET(archive_url,  db.albums.archive_wav[id], s); }
void   _::archive_flac_url(ccstr s) { STR_SET(archive_url, db.albums.archive_flac[id], s); }
void   _::date(time_t t)            { db.albums.date[id]   = SHRINK_DATE(t);               }
void   _::rating(float i)           { db.albums.rating[id] = i * 100;                      }
void   _::votes(int i)              { db.albums.votes[id]  = i;                            }
void   _::download_count(int i)     { db.albums.download_count[id] = i;                    }
void   _::styles(int i)             { db.albums.styles[id] = i;                            }
// INDEX
DB::Field _::operator[](DB::ColumnID id) const {
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

#undef _


// ============================================================================
#define _ Database :: Tracks :: Track // ======================================
// ============================================================================

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
void  TRACK::bpm(int i)       { db.tracks.bpm[id] = (i & 0xFF);               }
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

#undef _


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
