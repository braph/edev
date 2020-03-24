#include "database.hpp"

#include <fstream>
#include <climits>
#include <cstring>
#include <iostream>
#include <unordered_map>

#define DB_ABI_VERSION      1
#define DB_ENDIANNESS_CHECK 0xFEFF

namespace Database {

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

  void dump(size_t value)
  { fs.write(reinterpret_cast<char*>(&value), sizeof(value)); }

  void dump(uint16_t value)
  { fs.write(reinterpret_cast<char*>(&value), sizeof(value)); }

  void dump(StringPool& p)
  { dump(BITSOF(char), size_t(p.size()), reinterpret_cast<char*>(p.data())); }

  void dump(DynamicPackedVector& v)
  { dump(size_t(v.bits()), v.size(), reinterpret_cast<char*>(v.data())); }

  template<typename T>
  void dump(std::vector<T>& v)
  { dump(BITSOF(T), v.size(), reinterpret_cast<char*>(v.data())); }

  void dump(Table& t)
  { for (const auto& col : t.columns) { dump(*col); } }

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

  void load(size_t& value)
  { fs.read(reinterpret_cast<char*>(&value), sizeof(value)); }

  void load(uint16_t& value)
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

  void load(Table& t)
  { for (const auto& col : t.columns) { load(*col); } }

private:
  std::ifstream& fs;
  size_t elem_bits;
  size_t elem_count;

  void readHeader(size_t expected_bits = 0) {
    elem_bits = 0; elem_count = 0;
    load(elem_bits);
    load(elem_count);
    if (elem_bits == 0 || (expected_bits && elem_bits != expected_bits))
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
 * Database
 * ==========================================================================*/

Database :: Database()
: styles(*this, pool_style_url, pool_meta)
, albums(*this, pool_album_url, pool_cover_url, pool_archive_url, pool_desc, pool_meta)
, tracks(*this, pool_track_url, pool_meta)
, tables({&styles, &albums, &tracks})
, pools({&pool_meta, &pool_desc, &pool_style_url, &pool_album_url,
    &pool_track_url, &pool_cover_url, &pool_archive_url})
{
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

  for (auto& p : pools)  l.load(*p);
  for (auto& t : tables) l.load(*t);
}

void Database :: save(const std::string& file) {
  std::ofstream fs;
  fs.exceptions(std::ofstream::failbit|std::ofstream::badbit);
  fs.open(file, std::ios::binary);
  Dumper d(fs);
  d.dump(static_cast<uint16_t>(DB_ENDIANNESS_CHECK));
  d.dump(static_cast<uint16_t>(DB_ABI_VERSION));
  for (const auto& p : pools)  d.dump(*p);
  for (const auto& t : tables) d.dump(*t);
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

  for (auto& table : tables)
    table->shrink_to_fit();
}

void Database :: shrink_pool_to_fit(StringPool& pool, std::initializer_list<Column*> columns) {
  if (pool.is_shrinked())
    return;
  std::cerr << " shrinking pool ...";

  std::unordered_map<int, int> idRemap;
  for (auto& col : columns)
    idRemap.reserve(col->size());

  for (auto col : columns)
    for (auto id : *col)
      idRemap[id] = 0;

  pool.shrink_to_fit(idRemap);

  for (auto& column : columns)
    for (Column::iterator it = column->begin(); it != column->end(); ++it)
      *it = idRemap[*it];
    //for (auto& id : *column)
    //  id = idRemap[id];
}

/* ============================================================================
 * Database :: Table
 * ==========================================================================*/

/* Find a record by its URL or create one if it could not be found */
template<typename TTable>
static typename TTable::value_type find_by_url(TTable& table, StringPool& pool, InStr url, bool create) {
  if (!*url)
    return typename TTable::value_type(NULL, 0);

  int strId = pool.find(url);
  if (strId) {
    auto pos = std::find(table.url.begin(), table.url.end(), strId);
    if (pos != table.url.end())
      return typename TTable::value_type(&table, size_t(pos - table.url.begin()));
  }

  if (create) {
    if (! strId)
      strId = pool.add(url, true);

    size_t pos = table.size();
    table.resize(pos+1);
    table.url[pos] = strId;
    return typename TTable::value_type(&table, pos);
  }

  return typename TTable::value_type(NULL, 0);
}

// ============================================================================
// Styles :: Style ============================================================
// ============================================================================

Styles::Style Styles::find(InStr url, bool create) {
  return find_by_url(*this, db.pool_style_url, url, create);
}

Field Styles::Style::operator[](ColumnID id) const {
  switch (static_cast<StyleColumnID>(id)) {
  case STYLE_URL:   return Field(url());
  case STYLE_NAME:  return Field(name());
  default:          return Field(REPORT_BUG);
  }
}

// ============================================================================
// Albums :: Album ============================================================
// ============================================================================

Albums::Album Albums::find(InStr url, bool create) {
  return find_by_url(*this, db.pool_album_url, url, create);
}

Field Albums::Album::operator[](ColumnID id) const {
  switch (static_cast<AlbumColumnID>(id)) {
  case ALBUM_URL:             return Field(url());
  case ALBUM_COVER_URL:       return Field(cover_url());
  case ALBUM_TITLE:           return Field(title());
  case ALBUM_ARTIST:          return Field(artist());
  case ALBUM_STYLES:          return Field(styles());
  case ALBUM_DESCRIPTION:     return Field(description());
  case ALBUM_DATE:            return Field(date());
  case ALBUM_RATING:          return Field(rating());
  case ALBUM_VOTES:           return Field(votes());
  case ALBUM_DOWNLOAD_COUNT:  return Field(download_count());
  default:
    time_t stamp = date();
    struct tm t;
    localtime_r(&stamp, &t);
    switch (static_cast<AlbumColumnID>(id)) {
    case ALBUM_DAY:           return Field(t.tm_mday);
    case ALBUM_MONTH:         return Field(t.tm_mon + 1);
    case ALBUM_YEAR:          return Field(t.tm_year + 1900);
    default:                  return Field(REPORT_BUG);
    }
  }
}

// ============================================================================
// Tracks :: Track ============================================================
// ============================================================================

Tracks::Track Tracks::find(InStr url, bool create) {
  return find_by_url(*this, db.pool_track_url, url, create);
}

Albums::Album Tracks::Track::album() const noexcept {
  return table->db.albums[size_t(table->album_id[id])];
}

Field Tracks::Track::operator[](ColumnID id) const {
  switch (static_cast<TrackColumnID>(id)) {
  case TRACK_TITLE:     return Field(title());
  case TRACK_ARTIST:    return Field(artist());
  case TRACK_REMIX:     return Field(remix());
  case TRACK_NUMBER:    return Field(number());
  case TRACK_BPM:       return Field(bpm());
  default:              return album()[id];
  }
}

} // namespace Database

// ============================================================================
#ifdef TEST_DATABASE
#include "lib/test.hpp"
#include <algorithm>

template<typename T1, typename T2>
bool equals(T1& result, Database::ColumnID id, T2& vec) {
  if (result.size() != vec.size())
    return false;

  for (size_t i = 0; i < result.size(); ++i)
    if (result[i][id].value.s != vec[i])
      return false;

  return true;
}

/* Tests should be done on a newly created database from update.cpp.
 * Tests won't modify the original database! */
int main () {
  TEST_BEGIN();
  Database::Database db;
  db.load(TEST_DB);

  /* Test: ROW with ID 0 is actually empty */
  assert (std::strlen(db.styles[0].url()) == 0);
  assert (std::strlen(db.albums[0].url()) == 0);
  assert (std::strlen(db.tracks[0].url()) == 0);

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

  /* Test: shrink_to_fit() ================================================= */
  {
    Database::Database db2;
    db2.load(TEST_DB);
    db2.shrink_to_fit();
    for (size_t i = 0; i < db.tracks.size(); ++i)
      assert(streq(db.tracks[i].title(), db2.tracks[i].title()));
    for (size_t i = 0; i < db.albums.size(); ++i)
      assert(streq(db.albums[i].title(), db2.albums[i].title()));
  }

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
