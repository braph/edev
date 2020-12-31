#include "database.hpp"

#include "ektoplayer.hpp"

#include <lib/raii/file.hpp>

#include <type_traits>
#include <climits>
#include <cstdint>
#include <cstring>
#include <cstdio>

#define DB_ABI_VERSION      1
#define DB_ENDIANNESS_CHECK 0xFEFF

namespace Database {

struct Dumper {
  Dumper(FILE* fh)
    : fs(fh)
  {}

  void dump(StringChunk& p) {
    const size_t size = size_t(p.size());
    dump(size);
    write(reinterpret_cast<char*>(p.data()), size);
    dump(size);
  }

  void dump(DynamicPackedVector& v) {
    const uint8_t bits = v.bits();
    const size_t  size = v.size();
    dump(bits);
    dump(size);
    write(reinterpret_cast<char*>(v.data()), size_for_bits(bits * size));
    dump(bits);
    dump(size);
  }

  template<typename T>
  void dump(std::vector<T>& v) {
    const uint8_t bytes = sizeof(T);
    const size_t  size  = v.size();
    dump(bytes);
    dump(size);
    write(reinterpret_cast<char*>(v.data()), bytes * size);
    dump(bytes);
    dump(size);
  }

  template<typename T>
  inline void dump(T value) {
    static_assert(std::is_arithmetic<T>::value, "T not an integer");
    write(reinterpret_cast<char*>(&value), sizeof(value));
  }

  void dump(Table& t) {
    for (const auto col : t.columns)
      dump(*col);
  }

private:
  FILE* fs;

  void write(char* buf, size_t size) {
    if (::fwrite(buf, size, 1, fs) != 1)
      throw std::runtime_error(strerror(EIO));
  }
};

struct Loader {
  Loader(FILE* fs)
    : fs(fs)
  {}

#define MSG_BAD_FOOTER "bad footer"

  void load(StringChunk& chunk) {
    const size_t size = load<size_t>();
    chunk.resize(size);
    read(reinterpret_cast<char*>(chunk.data()), size);
    if (load<size_t>() != size)
      throw std::runtime_error(MSG_BAD_FOOTER);
  }

  void load(DynamicPackedVector& vec) {
    const uint8_t bits = load<uint8_t>();
    const size_t  size = load<size_t>();
    vec.reserve(size, bits);
    vec.resize(size);
    read(reinterpret_cast<char*>(vec.data()), size_for_bits(bits * size));
    if (load<uint8_t>() != bits) throw std::runtime_error(MSG_BAD_FOOTER);
    if (load<size_t>() != size)  throw std::runtime_error(MSG_BAD_FOOTER);
  }

  template<typename T>
  void load(std::vector<T>& v) {
    const uint8_t bytes = load<uint8_t>();
    const size_t  size  = load<size_t>();
    if (bytes != sizeof(T))
      throw std::runtime_error("byte count != sizeof(T)");
    v.resize(size);
    read(reinterpret_cast<char*>(v.data()), size);
    if (load<uint8_t>() != bytes) throw std::runtime_error(MSG_BAD_FOOTER);
    if (load<size_t>() != size)   throw std::runtime_error(MSG_BAD_FOOTER);
  }

  template<typename T>
  inline T load() {
    T value;
    static_assert(std::is_arithmetic<T>::value, "T not an integer");
    read(reinterpret_cast<char*>(&value), sizeof(value));
    return value;
  }

  void load(Table& t) {
    for (const auto& col : t.columns)
      load(*col);
  }

private:
  FILE* fs;

  void read(char* buf, size_t size) {
    if (::fread(buf, size, 1, fs) != 1)
      throw std::runtime_error(strerror(EIO));
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

void Database :: load(const std::string& file) {
  RAII::FILE fh( fopen(file.c_str(), "r") );
  if (! fh)
    throw std::runtime_error(strerror(errno));

  Loader l(fh);
  if (l.load<uint16_t>() != DB_ENDIANNESS_CHECK)
    throw std::runtime_error("Database endianess mismatch");

  if (l.load<uint16_t>() != DB_ABI_VERSION)
    throw std::runtime_error("Database ABI version mismatch");

  for (auto p : chunks)
    l.load(*p);

  for (auto t : tables)
    l.load(*t);
}

void Database :: save(const std::string& file) {
  RAII::FILE fh( fopen(file.c_str(), "w") );
  if (! fh)
    throw std::runtime_error(strerror(errno));

  Dumper d(fh);
  d.dump<uint16_t>(DB_ENDIANNESS_CHECK);
  d.dump<uint16_t>(DB_ABI_VERSION);
  for (auto p : chunks)
    d.dump(*p);
  for (auto t : tables)
    d.dump(*t);
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

Field Styles::Style::operator[](ColumnID id) const noexcept {
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

Field Albums::Album::operator[](ColumnID id) const noexcept {
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

Field Tracks::Track::operator[](ColumnID id) const noexcept {
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
#include <lib/test.hpp>
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

  auto styles = db.get_styles();
  auto albums = db.get_albums();
  auto tracks = db.get_tracks();

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


  /* Test: WHERE ALBUM_TITLE =============================================== */
  tracks.erase(
    remove_if(tracks.begin(), tracks.end(),
      Database::Where((Database::ColumnID) Database::ALBUM_TITLE, Database::Operator::EQUAL, "Interbeing")),
      tracks.end());

  assert(tracks.size() == 8);
  for (auto track : tracks)
    assert(streq(track.album().title(), "Interbeing"));


  /* Test: WHERE TRACK_TITLE =============================================== */
  tracks.erase(
    remove_if(tracks.begin(), tracks.end(),
      Database::Where((Database::ColumnID) Database::TRACK_TITLE, Database::Operator::EQUAL, "Satori")),
      tracks.end());

  assert(tracks.size() == 1);
  assert(streq(tracks[0].title(), "Satori"));


  /* Test: Failing to load a invalid database ============================== */
  except(db.load("/non-existent"));
  except(db.load("/bin/true"));

  TEST_END();
}
#endif
