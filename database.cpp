#include "database.hpp"

#include "common.hpp"

#include <cstdlib>
#include <climits>
#include <cstring>
#include <iostream>
#include <unordered_map>

typedef const char* ccstr;

/* ============================================================================
 * Dumper / Loader
 * ============================================================================
 * Helpers for saving and reading containers to/from disk.
 *
 * Binary format for containers:
 *   size_t elem_bits  : Bit width of one element
 *   size_t elem_count : Element count
 *   void*  data[]     : Data
 *   size_t elem_bits  : Bit width of one element -.
 *   size_t elem_count : Element count            -'-> Used for validation
 */

struct Dumper {
  Dumper(std::ofstream& fs) : fs(fs) {}

  inline void dump(size_t value)
  { fs.write(reinterpret_cast<char*>(&value), sizeof(value)); }

  inline void dump(uint16_t value)
  { fs.write(reinterpret_cast<char*>(&value), sizeof(value)); }

  inline void dump(StringPool& p)
  { dump(BITSOF(char), size_t(p.size()), reinterpret_cast<char*>(p.data())); }

  inline void dump(DynamicPackedVector& v)
  { dump(size_t(v.bits()), v.size(), reinterpret_cast<char*>(v.data())); }

  template<typename T>
  inline void dump(std::vector<T>& v)
  { dump(BITSOF(T), v.size(), reinterpret_cast<char*>(v.data())); }

  inline void dump(Database::Table& t)
  { for (auto col : t.columns) { dump(*col); } }

private:
  std::ofstream& fs;
  void dump(size_t bits, size_t count, char* data) {
    dump(bits);
    dump(count);
    fs.write(data, std::streamsize(size_for_bits(bits*count)));
    dump(bits);
    dump(count);
  }
};

struct Loader {
  Loader(std::ifstream& fs) : fs(fs) {}

  inline void load(size_t& value)
  { fs.read(reinterpret_cast<char*>(&value), sizeof(value)); }

  inline void load(uint16_t& value)
  { fs.read(reinterpret_cast<char*>(&value), sizeof(value)); }

  void load(StringPool& pool) {
    readHeader(BITSOF(char));
    pool.resize(elem_count);
    readData(reinterpret_cast<char*>(pool.data()));
  }

  void load(DynamicPackedVector& vec) {
    readHeader();
    vec.reserve(elem_count, elem_bits);
    vec.resize(elem_count);
    readData(reinterpret_cast<char*>(vec.data()));
  }

  template<typename T>
  void load(std::vector<T>& v) {
    readHeader(BITSOF(T));
    v.resize(elem_count);
    readData(reinterpret_cast<char*>(v.data()));
  }

  void load(Database::Table& t)
  { for (auto col : t.columns) { load(*col); } }

private:
  std::ifstream& fs;
  size_t elem_bits;
  size_t elem_count;

  void readHeader(size_t expected_bits = 0) {
    elem_bits = 0xDEAD; elem_count = 0;
    load(elem_bits);
    load(elem_count);
    if (elem_bits == 0 || elem_bits == 0xDEAD || (expected_bits && elem_bits != expected_bits))
      throw std::runtime_error("Invalid bit count in header");
  }

  void readData(char* buf) {
    size_t check_bits = 0xDEAD, check_count = 0;
    fs.read(buf, std::streamsize(size_for_bits(elem_bits*elem_count)));
    fs.read(reinterpret_cast<char*>(&check_bits),  sizeof(check_bits));
    fs.read(reinterpret_cast<char*>(&check_count), sizeof(check_count));
    if (check_bits != elem_bits || check_count != elem_count)
      throw std::runtime_error("Validation failed: invalid bit/element count in footer");
  }
};

/* ============================================================================
 * A bit ugly, but it shortens up the code a lot
 * ==========================================================================*/

#define STR_GET(POOL_NAME, STRING_ID) \
  db.pool_##POOL_NAME.get(STRING_ID)

#define STR_SET(POOL_NAME, STRING_ID, STR) \
  if (0 != std::strcmp(STR_GET(POOL_NAME, STRING_ID), STR)) \
    { STRING_ID = db.pool_##POOL_NAME.add(STR); }

/* ============================================================================
 * Database
 * ==========================================================================*/

Database :: Database()
: styles(*this)
, albums(*this)
, tracks(*this)
, tables({&styles, &albums, &tracks})
, pools({&pool_meta, &pool_desc, &pool_style_url, &pool_album_url,
    &pool_track_url, &pool_cover_url, &pool_archive_url})
{
  // Records with ID 0 represent a NULL value. Create them here.
  for (auto table : tables)
    table->resize(1);

  assert(0 == styles.find("", true).id);
  assert(0 == albums.find("", true).id);
  assert(0 == tracks.find("", true).id);
}

void Database :: load(const std::string& file) {
  uint16_t check;
  std::ifstream fs;
  fs.exceptions(std::ifstream::failbit|std::ifstream::badbit|std::ifstream::eofbit);
  fs.open(file, std::ios::binary);

  Loader l(fs);
  l.load(check);
  if (check != DB_ENDIANNESS_CHECK)
    throw std::runtime_error("Database endianess mismatch");

  l.load(check);
  if (check != DB_ABI_VERSION)
    throw std::runtime_error("Database ABI version mismatch");

  for (auto p : pools)  l.load(*p);
  for (auto t : tables) l.load(*t);
}

void Database :: save(const std::string& file) {
  std::ofstream fs;
  fs.exceptions(std::ofstream::failbit|std::ofstream::badbit);
  fs.open(file, std::ios::binary);
  Dumper d(fs);
  d.dump(static_cast<uint16_t>(DB_ENDIANNESS_CHECK));
  d.dump(static_cast<uint16_t>(DB_ABI_VERSION));
  for (auto p : pools)  d.dump(*p);
  for (auto t : tables) d.dump(*t);
}

void Database :: shrink_to_fit() {
  shrink_pool_to_fit(pool_style_url, {&styles.url});
  shrink_pool_to_fit(pool_track_url, {&tracks.url});
  shrink_pool_to_fit(pool_album_url, {&albums.url});
  shrink_pool_to_fit(pool_cover_url, {&albums.cover_url});
  shrink_pool_to_fit(pool_desc,      {&albums.description});
  shrink_pool_to_fit(pool_archive_url,
    {&albums.archive_mp3, &albums.archive_wav, &albums.archive_flac});
  shrink_pool_to_fit(pool_meta, {&styles.name, &albums.title,
    &albums.artist, &tracks.title, &tracks.artist, &tracks.remix});

  for (auto table : tables)
    table->shrink_to_fit();
}

void Database :: shrink_pool_to_fit(StringPool& pool, std::initializer_list<Column*> columns) {
  std::cerr << " shrinking pool ...";
  if (pool.isOptimized()) {
    std::cerr << " already shrinked";
    return;
  }

  StringPool newPool;
  newPool.reserve(size_t(pool.size()));

  // Build the map used for remapping IDs, storing all IDs used in columns
  std::unordered_map<int, int> idRemap;
  for (auto col : columns)
    idRemap.reserve(col->size());

  for (auto col : columns)
    for (auto id : *col)
      idRemap[id] = 0;

  struct IDAndLength { int id; size_t length; };

  // Sort the IDs, longest strings first
  std::vector<IDAndLength> idSortedByLength;
  idSortedByLength.reserve(idRemap.size());
  for (auto& pair : idRemap)
    idSortedByLength.push_back({pair.first, std::strlen(pool.get(pair.first))});

  std::sort(idSortedByLength.begin(), idSortedByLength.end(),
      [](const IDAndLength& a, const IDAndLength& b){ return a.length > b.length; });

  // Add strings in the right order to the stringpool and store the new ID
  for (auto IDAndLength : idSortedByLength)
    idRemap[IDAndLength.id] = newPool.add(pool.get(IDAndLength.id));

  // Replace the IDs from the old pool by the IDs from the new pool
  for (auto column : columns)
    for (Column::iterator it = column->begin(); it != column->end(); ++it)
      *it = idRemap[*it];
    //for (auto& id : *column)
    //  id = idRemap[id];

  pool = newPool;
}

/* ============================================================================
 * Database :: Table
 * ==========================================================================*/

/* Find a record by its URL or create one if it could not be found */
template<typename TTable>
static typename TTable::value_type find_by_url(TTable& table, StringPool& pool, const char* url, bool create) {
  if (!*url)
    return typename TTable::value_type(table.db, 0);

  int strId = pool.find(url);
  if (strId) {
    auto pos = std::find(table.url.begin(), table.url.end(), strId);
    if (pos != table.url.end())
      return typename TTable::value_type(table.db, size_t(pos - table.url.begin()));
  }

  if (create) {
    if (! strId)
      strId = pool.add(url, true);

    size_t pos = table.size();
    table.resize(pos+1);
    table.url[pos] = strId;
    return typename TTable::value_type(table.db, pos);
  }

  return typename TTable::value_type(table.db, 0);
}


#define DB    Database
#define STYLE Database :: Styles :: Style
#define ALBUM Database :: Albums :: Album
#define TRACK Database :: Tracks :: Track


// ============================================================================
#define _ Database :: Styles :: Style // ======================================
// ============================================================================

STYLE Database::Styles::find(const char *url, bool create) {
  return find_by_url(*this, db.pool_style_url, url, create);
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
  return find_by_url(*this, db.pool_album_url, url, create);
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
float  _::rating()           const { return float(db.albums.rating[id]) / 100;               }
int    _::votes()            const { return db.albums.votes[id];                             }
int    _::download_count()   const { return db.albums.download_count[id];                    }
int    _::styles()           const { return db.albums.styles[id];                            }
// SETTER
void   _::url(ccstr s)              { STR_SET(album_url,    db.albums.url[id],         s);  }
void   _::title(ccstr s)            { STR_SET(meta,         db.albums.title[id],       s);  }
void   _::artist(ccstr s)           { STR_SET(meta,         db.albums.artist[id],      s);  }
void   _::cover_url(ccstr s)        { STR_SET(cover_url,    db.albums.cover_url[id],   s);  }
void   _::description(ccstr s)      { STR_SET(desc,         db.albums.description[id], s);  }
void   _::archive_mp3_url(ccstr s)  { STR_SET(archive_url,  db.albums.archive_mp3[id], s);  }
void   _::archive_wav_url(ccstr s)  { STR_SET(archive_url,  db.albums.archive_wav[id], s);  }
void   _::archive_flac_url(ccstr s) { STR_SET(archive_url,  db.albums.archive_flac[id], s); }
void   _::date(time_t t)            { db.albums.date[id]   = SHRINK_DATE(t);                }
void   _::rating(float i)           { db.albums.rating[id] = i * 100;                       }
void   _::votes(int i)              { db.albums.votes[id]  = i;                             }
void   _::download_count(int i)     { db.albums.download_count[id] = i;                     }
void   _::styles(int i)             { db.albums.styles[id] = i;                             }
// INDEX
DB::Field _::operator[](DB::ColumnID id) const {
  switch (static_cast<DB::AlbumColumnID>(id)) {
  case DB::ALBUM_URL:             return DB::Field(url());
  case DB::ALBUM_COVER_URL:       return DB::Field(cover_url());
  case DB::ALBUM_TITLE:           return DB::Field(title());
  case DB::ALBUM_ARTIST:          return DB::Field(artist());
  case DB::ALBUM_STYLES:          return DB::Field(styles());
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
  return find_by_url(*this, db.pool_track_url, url, create);
}

// GETTER
ccstr _::url()      const { return STR_GET(track_url, db.tracks.url[id]);     }
ccstr _::title()    const { return STR_GET(meta,      db.tracks.title[id]);   }
ccstr _::artist()   const { return STR_GET(meta,      db.tracks.artist[id]);  }
ccstr _::remix()    const { return STR_GET(meta,      db.tracks.remix[id]);   }
int   _::number()   const { return db.tracks.number[id];                      }
int   _::bpm()      const { return db.tracks.bpm[id];                         }
int   _::album_id() const { return db.tracks.album_id[id];                    }
ALBUM _::album()    const { return db.albums[size_t(db.tracks.album_id[id])]; }
// SETTER
void  _::url(ccstr s)     { STR_SET(track_url, db.tracks.url[id],    s);  }
void  _::title(ccstr s)   { STR_SET(meta,      db.tracks.title[id],  s);  }
void  _::artist(ccstr s)  { STR_SET(meta,      db.tracks.artist[id], s);  }
void  _::remix(ccstr s)   { STR_SET(meta,      db.tracks.remix[id],  s);  }
void  _::number(int i)    { db.tracks.number[id] = i;                     }
void  _::bpm(int i)       { db.tracks.bpm[id] = (i & 0xFF /* max 255 */); }
void  _::album_id(int i)  { db.tracks.album_id[id] = i;                   }
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
#ifdef TEST_DATABASE
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

  std::sort(track_titles.begin(), track_titles.end(), [](const char* a, const char* b) { return strcmp(a, b) < 0; });
  std::sort(tracks.begin(), tracks.end(), Database::OrderBy(Database::TRACK_TITLE));

  // Sorting succeeded
  assert(equals(tracks, (Database::ColumnID) Database::TRACK_TITLE, track_titles));


  /* Test: ORDER BY ALBUM_TITLE ============================================ */
  std::vector<const char*> album_titles;
  for (auto track : tracks)
    album_titles.push_back(track.album().title());

  // Duplication succeded
  assert(equals(tracks, (Database::ColumnID) Database::ALBUM_TITLE, album_titles));

  std::sort(album_titles.begin(), album_titles.end(), [](const char* a, const char* b) { return strcmp(a, b) < 0; });
  std::sort(tracks.begin(), tracks.end(), Database::OrderBy(Database::ALBUM_TITLE));

  // Sorting succeeded
  assert(equals(tracks, (Database::ColumnID) Database::ALBUM_TITLE, album_titles));


#if 0 // TODO
  /* Test: WHERE ALBUM_TITLE =============================================== */
  tracks.where((Database::ColumnID) Database::ALBUM_TITLE, Database::EQUAL, "Interbeing");
  assert(tracks.size());
  for (auto track : tracks)
    assert(streq(track.album().title(), "Interbeing"));

  /* Test: WHERE TRACK_TITLE =============================================== */
  tracks.where((Database::ColumnID) Database::TRACK_TITLE, Database::EQUAL, "Satori");
  assert(tracks.size() == 1);
  assert(streq(tracks[0].title(), "Satori"));
#endif

  /* Test: Failing to load a invalid database ============================== */
  except(db.load("/non-existent"));
  except(db.load("/bin/true"));

  TEST_END();
}
#endif
