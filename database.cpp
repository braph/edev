#include "database.hpp"

#include "ektoplayer.hpp"

#include "lib/raii/file.hpp"

#include <climits>
#include <cstring>
#include <cstdio>

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
  Dumper(FILE* fh)
    : fs(fh)
    , _error(0)
  {}

  operator bool() const noexcept {
    return !_error;
  }

  // TODO: change reinterpret_cast to memcpy + template_is_integral
  void dump(size_t value) noexcept
  { write(reinterpret_cast<char*>(&value), sizeof(value)); }

  void dump(uint16_t value) noexcept
  { write(reinterpret_cast<char*>(&value), sizeof(value)); }

  void dump(StringChunk& p) noexcept
  { dump(BITSOF(char), size_t(p.size()), reinterpret_cast<char*>(p.data())); }

  void dump(DynamicPackedVector& v) noexcept
  { dump(size_t(v.bits()), v.size(), reinterpret_cast<char*>(v.data())); }

  template<typename T>
  void dump(std::vector<T>& v) noexcept
  { dump(BITSOF(T), v.size(), reinterpret_cast<char*>(v.data())); }

  void dump(Table& t) noexcept
  { for (const auto& col : t.columns) { dump(*col); } }

private:
  FILE* fs;
  int _error;

  void write(char* buf, size_t size) noexcept {
    _error |= !::fwrite(buf, size, 1, fs);
  }

  // maybe better name
  void dump(size_t bits, size_t count, char* data) noexcept {
    dump(bits);
    dump(count);
    write(data, size_for_bits(bits*count));
    dump(bits);
    dump(count);
  }
};

struct Loader {
  enum Error {
    Success = 0,
    InvalidBitCountInHeader,
    InvalidBitCountInFooter,
    InvalidElemCountInFooter
  };

  const char* what() const noexcept {
    switch (_error) {
      default:                        return strerror(0);
      case InvalidBitCountInHeader:   return "Invalid bit count in header";
      case InvalidBitCountInFooter:   return "Invalid bit count in footer";
      case InvalidElemCountInFooter:  return "Invalid element count in footer";
    }
  }

  operator bool() const noexcept {
    return ! _error;
  }

  Loader(FILE* fs)
    : fs(fs)
    , _error(Success)
  {}

  void load(size_t& value) noexcept
  { read(reinterpret_cast<char*>(&value), sizeof(value)); }

  void load(uint16_t& value) noexcept
  { read(reinterpret_cast<char*>(&value), sizeof(value)); }

  void load(StringChunk& chunk) noexcept {
    readHeader(BITSOF(char));
    chunk.resize(elem_count);
    readData(reinterpret_cast<char*>(chunk.data()));
  }

  void load(DynamicPackedVector& vec) noexcept {
    readHeader();
    vec.reserve(elem_count, elem_bits);
    vec.resize(elem_count);
    readData(reinterpret_cast<char*>(vec.data()));
  }

  template<typename T>
  void load(std::vector<T>& v) noexcept {
    readHeader(BITSOF(T));
    v.resize(elem_count);
    readData(reinterpret_cast<char*>(v.data()));
  }

  void load(Table& t) noexcept
  { for (const auto& col : t.columns) { load(*col); } }

private:
  FILE* fs;
  Error _error;
  size_t elem_bits;
  size_t elem_count;

  void read(char* buf, size_t size) noexcept {
    fread(buf, size, 1, fs);
  }

  void readHeader(size_t expected_bits = 0) noexcept {
    elem_bits = 0; elem_count = 0;
    load(elem_bits);
    load(elem_count);
    if (elem_bits == 0 || (expected_bits && elem_bits != expected_bits))
      _error = InvalidBitCountInHeader;
  }

  void readData(char* buf) noexcept {
    size_t check_bits = 0xDEAD, check_count = 0;
    read(buf, size_for_bits(elem_bits*elem_count));
    read(reinterpret_cast<char*>(&check_bits),  sizeof(check_bits));
    read(reinterpret_cast<char*>(&check_count), sizeof(check_count));
    if (check_bits != elem_bits)
      _error = InvalidBitCountInFooter;
    if (check_count != elem_count)
      _error = InvalidElemCountInFooter;
  }
};

/* ============================================================================
 * Database
 * ==========================================================================*/

Database :: Database() noexcept
: styles(*this, chunk_style_url, chunk_meta)
, albums(*this, chunk_album_url, chunk_cover_url, chunk_archive_url, chunk_desc, chunk_meta)
, tracks(*this, chunk_track_url, chunk_meta)
, tables({&styles, &albums, &tracks})
, chunks({&chunk_meta, &chunk_desc, &chunk_style_url, &chunk_album_url,
    &chunk_track_url, &chunk_cover_url, &chunk_archive_url})
{
}

const char* Database :: load(const std::string& file) noexcept {
  uint16_t check;
  RAII::FILE fh( fopen(file.c_str(), "r") );
  if (! fh)
    return strerror(errno);

  Loader l(fh);
  l.load(check);
  if (check != DB_ENDIANNESS_CHECK)
    return "Database endianess mismatch";

  l.load(check);
  if (check != DB_ABI_VERSION)
    return "Database ABI version mismatch";

  for (auto p : chunks)
    l.load(*p);

  for (auto t : tables)
    l.load(*t);

  return l ? NULL : l.what();
}

const char* Database :: save(const std::string& file) noexcept {
  RAII::FILE fh( fopen(file.c_str(), "w") );
  if (! fh)
    return strerror(errno);

  Dumper d(fh);
  d.dump(uint16_t(DB_ENDIANNESS_CHECK));
  d.dump(uint16_t(DB_ABI_VERSION));
  for (auto p : chunks)
    d.dump(*p);
  for (auto t : tables)
    d.dump(*t);

  return d ? NULL : strerror(EIO);
}

void Database :: shrink_to_fit() {
  shrink_chunk_to_fit(chunk_style_url, {&styles.url});
  shrink_chunk_to_fit(chunk_track_url, {&tracks.url});
  shrink_chunk_to_fit(chunk_album_url, {&albums.url});
  shrink_chunk_to_fit(chunk_cover_url, {&albums.cover_url});
  shrink_chunk_to_fit(chunk_desc,      {&albums.description});
  shrink_chunk_to_fit(chunk_archive_url,
    {&albums.archive_mp3, &albums.archive_wav, &albums.archive_flac});
  shrink_chunk_to_fit(chunk_meta, {&styles.name, &albums.title,
    &albums.artist, &tracks.title, &tracks.artist, &tracks.remix});

  for (auto& table : tables)
    table->shrink_to_fit();
}

void Database :: shrink_chunk_to_fit(StringChunk& chunk, std::initializer_list<Column*> columns) {
  if (chunk.is_shrinked())
    return;
  log_write("shrinking chunk ... ");

  StringChunk::Shrinker shrinker = chunk.get_shrinker();
  for (auto col : columns)
    for (auto id : *col)
      shrinker.add(id);

  shrinker.shrink();

  for (auto& column : columns)
    for (Column::iterator it = column->begin(); it != column->end(); ++it)
      *it = shrinker.get_new_id(*it);
    //for (auto& id : *column)
    //  id = idRemap[id];
}

/* ============================================================================
 * Database :: Table
 * ==========================================================================*/

/* Find a record by its URL or create one if it could not be found */
template<typename TTable>
static typename TTable::value_type find_by_url(TTable& table, StringChunk& chunk, CString url, bool create) {
  if (url.empty())
    return typename TTable::value_type(NULL, 0);

  int strId = chunk.find(url);
  if (strId) {
    auto pos = std::find(table.url.begin(), table.url.end(), strId);
    if (pos != table.url.end())
      return typename TTable::value_type(&table, size_t(pos - table.url.begin()));
  }

  if (create) {
    if (! strId)
      strId = chunk.add_unchecked(url);

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

Styles::Style Styles::find(CString url, bool create) {
  return find_by_url(*this, db.chunk_style_url, url, create);
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

Albums::Album Albums::find(CString url, bool create) {
  return find_by_url(*this, db.chunk_album_url, url, create);
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
    std::time_t stamp = date();
    std::tm* t = std::localtime(&stamp); // not thread safe
    switch (static_cast<AlbumColumnID>(id)) {
    case ALBUM_DAY:           return Field(t->tm_mday);
    case ALBUM_MONTH:         return Field(t->tm_mon + 1);
    case ALBUM_YEAR:          return Field(t->tm_year + 1900);
    default:                  return Field(REPORT_BUG);
    }
  }
}

// ============================================================================
// Tracks :: Track ============================================================
// ============================================================================

Tracks::Track Tracks::find(CString url, bool create) {
  return find_by_url(*this, db.chunk_track_url, url, create);
}

Albums::Album Tracks::Track::album() const noexcept {
  return table->db.albums[size_t(table->album_id[id])];
}

Field Tracks::Track::operator[](ColumnID id) const { // XXX: noexcept
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

using namespace std;

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
  assert (strlen(db.styles[0].url()) == 0);
  assert (strlen(db.albums[0].url()) == 0);
  assert (strlen(db.tracks[0].url()) == 0);

  auto styles = db.getStyles();
  auto albums = db.getAlbums();
  auto tracks = db.getTracks();

  if (db.tracks.size() < 100)
    throw runtime_error("Sorry, I need a database with some data, please run the updater ...");

  /* Test: Count of result set equals the number of table entries */
  assert (styles.size() == db.styles.size() - 1);
  assert (albums.size() == db.albums.size() - 1);
  assert (tracks.size() == db.tracks.size() - 1);
  /* Test: First row of database contains valid data */
  assert (strlen(db.styles[1].url()));
  assert (strlen(db.albums[1].url()));
  assert (strlen(db.tracks[1].url()));
  /* Test: First row of result set contains valid data */
  assert (strlen(styles[0].url()));
  assert (strlen(albums[0].url()));
  assert (strlen(tracks[0].url()));
  /* Test: The first row of a result set equals the first record of a table */
  assert (styles[0].url() == db.styles[1].url());
  assert (albums[0].url() == db.albums[1].url());
  assert (tracks[0].url() == db.tracks[1].url());
  /* Test: Last row of database contains valid data */
  assert (strlen(db.styles[db.styles.size() - 1].url()));
  assert (strlen(db.albums[db.albums.size() - 1].url()));
  assert (strlen(db.tracks[db.tracks.size() - 1].url()));
  /* Test: Last row of result set */
  assert (strlen(styles[styles.size() - 1].url()));
  assert (strlen(albums[albums.size() - 1].url()));
  assert (strlen(tracks[tracks.size() - 1].url()));

  /* Test: shrink_to_fit() ================================================= */
  {
    Database::Database db2;
    db2.load(TEST_DB);
    db2.shrink_to_fit();
    for (size_t i = 0; i < db.tracks.size(); ++i)
      assert(streq(db.tracks[i].title(), db2.tracks[i].title()));
    for (size_t i = 0; i < db.albums.size(); ++i)
      assert(streq(db.albums[i].title(), db2.albums[i].title()));
    for (const auto& chunk : db2.chunks)
      assert(chunk->is_shrinked());
    db2.save(TEST_DB ".shrinked");
  }

  /* Test: ORDER BY TRACK_TITLE ============================================ */
  vector<const char*> track_titles;
  for (auto track : tracks)
    track_titles.push_back(track.title());

  // Duplication succeded
  assert(equals(tracks, (Database::ColumnID) Database::TRACK_TITLE, track_titles));

  sort(track_titles.begin(), track_titles.end(), [](const char* a, const char* b) { return strcmp(a, b) < 0; });
  sort(tracks.begin(), tracks.end(), Database::OrderBy(Database::TRACK_TITLE));

  // Sorting succeeded
  assert(equals(tracks, (Database::ColumnID) Database::TRACK_TITLE, track_titles));


  /* Test: ORDER BY ALBUM_TITLE ============================================ */
  vector<const char*> album_titles;
  for (auto track : tracks)
    album_titles.push_back(track.album().title());

  // Duplication succeded
  assert(equals(tracks, (Database::ColumnID) Database::ALBUM_TITLE, album_titles));

  sort(album_titles.begin(), album_titles.end(), [](const char* a, const char* b) { return strcmp(a, b) < 0; });
  sort(tracks.begin(), tracks.end(), Database::OrderBy(Database::ALBUM_TITLE));

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
