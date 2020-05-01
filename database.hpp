#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "lib/cstring.hpp"
#include "lib/genericiterator.hpp"
#include "lib/stringpool.hpp"
#include "lib/packedvector.hpp"
#include "lib/switch.hpp"
#include "ektoplayer.hpp" // REPORT_BUG

#include <array>
#include <vector>
#include <string>
#include <cstring>
#include <initializer_list>

#define DATABASE_USE_PACKED_VECTOR 1

namespace Database {

class Database;
typedef const char* ccstr;

/* ============================================================================
 * Metadata Database
 * ============================================================================
 *
 * === Tables ===
 * A table consists of a fixed number of columns holding all the data.
 * For retrieving a row from a table a proxy object (struct Record) is returned.
 * The index of a row is also its ID.
 *
 * === Columns ===
 * A column is a vector<int> like container. It stores the integers in a packed
 * way. The integers only occupy as much bits as the biggest integer contained.
 * For storing strings inside a column, a reference id to a string pool must be
 * stored.
 *
 * === String Pools ===
 * A stringpool is one big buffer containing all possible strings in the
 * database. A string is referenced using an ID, which is simply the offset
 * from the beginning of the buffer.
 *
 * Inserting into a stringpool is expensive (the whole buffer has to be
 * searched) and the [sub]string-deduplication makes only sense on similar
 * data, so we use separate pools for each column.
 * Exception is {track,album}{title,artist,remix,style} - they all share the same
 * pool ("meta") as there is a chance that we can find duplicates there.
 *
 * Splitting up the stringpool also results in lower string IDs per pool,
 * leading to smaller storage requirements in a bitpacked vector.
 *
 * === Loading and Saving the database ===
 * Loading and saving is practically done by reading/writing the raw memory
 * to disk. Since the database file is not meant to be shared by other
 * machines/architectures this should be fine. A minimal form of data
 * validation is performed, so the database may be rebuilt on errors.
 * And after all the database is more like a cache.
 *
 * XXX NOTE XXX
 * The `Album::styles` field stores at max 4 styles...
 * Records with ID = 0 (first row) are used for representing a NULL value.
 * The row count of a table will therefore be off by one.
 * It is easier to do a `-1` on the return value of Table::size() than applying
 * this -1 logic all over the place. this won't be fixed.
 */

/* ==========================================================================
 * ColumnIDs
 *
 * A single field of a record can be accessed using the index operator[].
 *
 * Although there are multiple enum types defined, all functions use the
 * `ColumnID` type as parameter.
 *
 * Not all of these IDs correspond to a real field. Some of them are just a
 * `view` of another column (e.g. `ALBUM_DAY` returns day of `ALBUM_DATE`).
 * ========================================================================*/

enum ColumnID {
  COLUMN_NONE = 0
};

enum StyleColumnID {
  STYLE_URL = 1,
  STYLE_NAME,
  STYLE_ENUM_END,
};

enum AlbumColumnID {
  ALBUM_URL = STYLE_ENUM_END,
  ALBUM_TITLE,
  ALBUM_ARTIST,
  ALBUM_STYLES,
  ALBUM_COVER_URL,
  ALBUM_DESCRIPTION,
  ALBUM_DATE,
  ALBUM_DAY,   // -.
  ALBUM_MONTH, //  +- View
  ALBUM_YEAR,  // -'
  ALBUM_RATING,
  ALBUM_VOTES,
  ALBUM_DOWNLOAD_COUNT,
  ALBUM_ENUM_END,
};

enum TrackColumnID {
  TRACK_URL = ALBUM_ENUM_END,
  TRACK_TITLE,
  TRACK_ARTIST,
  TRACK_REMIX,
  TRACK_NUMBER,
  TRACK_BPM,
  TRACK_ENUM_END,
};

static ColumnID columnIDFromStr(const std::string &s) noexcept {
  using pack = StringPack::AlphaNoCase;
  switch (pack::pack_runtime(s)) {
    case pack("style"):         return static_cast<ColumnID>(STYLE_NAME);
    case pack("album"):         return static_cast<ColumnID>(ALBUM_TITLE);
    case pack("album_artist"):  return static_cast<ColumnID>(ALBUM_ARTIST);
    case pack("description"):   return static_cast<ColumnID>(ALBUM_DESCRIPTION);
    case pack("date"):          return static_cast<ColumnID>(ALBUM_DATE);
    case pack("rating"):        return static_cast<ColumnID>(ALBUM_RATING);
    case pack("votes"):         return static_cast<ColumnID>(ALBUM_VOTES);
    case pack("downloads"):     return static_cast<ColumnID>(ALBUM_DOWNLOAD_COUNT);
    case pack("day"):           return static_cast<ColumnID>(ALBUM_DAY);
    case pack("month"):         return static_cast<ColumnID>(ALBUM_MONTH);
    case pack("year"):          return static_cast<ColumnID>(ALBUM_YEAR);
    case pack("title"):         return static_cast<ColumnID>(TRACK_TITLE);
    case pack("artist"):        return static_cast<ColumnID>(TRACK_ARTIST);
    case pack("remix"):         return static_cast<ColumnID>(TRACK_REMIX);
    case pack("number"):        return static_cast<ColumnID>(TRACK_NUMBER);
    case pack("bpm"):           return static_cast<ColumnID>(TRACK_BPM);
    case pack("styles"):        return static_cast<ColumnID>(ALBUM_STYLES);
    default:                    return static_cast<ColumnID>(COLUMN_NONE);
  }
}

/* ==========================================================================
 * The field struct is basically a union type.
 * ========================================================================*/

struct Field {
  enum  Type  { STRING,  INTEGER, FLOAT,   TIME,     };
  union Value { ccstr s; int i;   float f; time_t t; };

  Type type;
  Value value;

  inline Field(ccstr s)   noexcept { setString(s);  }
  inline Field(int i)     noexcept { setInteger(i); }
  inline Field(float f)   noexcept { setFloat(f);   }
  inline Field(time_t t)  noexcept { setTime(t);    }

  void setString(ccstr s) noexcept { type = STRING;  value.s = s; }
  void setInteger(int i)  noexcept { type = INTEGER; value.i = i; }
  void setFloat(float f)  noexcept { type = FLOAT;   value.f = f; }
  void setTime(time_t t)  noexcept { type = TIME;    value.t = t; }

  int compare(const Field &rhs) const noexcept {
    assert(type == rhs.type);
    switch (type) {
    case STRING:  return std::strcmp(value.s, rhs.value.s);
    case INTEGER: return this->value.i - rhs.value.i;
    case FLOAT:   return this->value.f - rhs.value.f;
    case TIME:    return this->value.t - rhs.value.t;
    }
  }
};

// === Column ===============================================================
#if DATABASE_USE_PACKED_VECTOR
using Column = DynamicPackedVector;
#else
using Column = std::vector<int>;
#endif

using StylesArray = TinyPackedArray<5, uint32_t>;

// === Base class for all tables ============================================
struct Table {
  ccstr name;
  Database &db;
  std::vector<Column*> columns;

  Table(ccstr name, Database &db, std::vector<Column*> columns)
  : name(name)
  , db(db)
  , columns(columns)
  {}

  size_t size() const noexcept { return columns[0]->size();                 }
  void   resize(size_t n)      { for (auto c : columns) c->resize(n);       }
  void   reserve(size_t n)     { for (auto c : columns) c->reserve(n);      }
  void   shrink_to_fit()       { for (auto c : columns) c->shrink_to_fit(); }
};

// === Base class for all records ===========================================
template<typename TablePointer>
struct Record {
  TablePointer table;
  size_t id;
  Record() : table(NULL), id(0) {}
  Record(TablePointer t, size_t id) : table(t), id(id) {}
  operator bool()                   const noexcept { return id != 0;    }
  bool operator!=(const Record& r)  const noexcept { return id != r.id; }
  bool operator==(const Record& r)  const noexcept { return id == r.id; }
  Record& operator=(const Record& r)      noexcept { id = r.id; table = r.table; return *this; }
};

/* ==========================================================================
 * Table definitions begin here
 * ========================================================================*/

class StringColumn : public Column {
  StringPool& pool;
public:
  StringColumn(StringPool& pool) : pool(pool) {}

  const char* get(size_t i) noexcept { // TODO: this should be const!
    return pool.get((*this)[i]);
  }

  void set(size_t i, CString s) {
    int string_id = (*this)[i];
    if (!string_id || std::strcmp(pool.get(string_id), s))
      (*this)[i] = pool.add(s);
  }
};

struct Styles : public Table {
  StringColumn url;
  StringColumn name;

  Styles(Database& db, StringPool& pool_url, StringPool& pool_name)
  : Table("styles", db, {&url,&name})
  , url(pool_url)
  , name(pool_name)
  {
    resize(1); // Records with ID 0 represent a NULL value. Create them here.
  }

  struct Style : public Record<Styles*> {
    using Record::Record;

    // GETTER
    Field operator[](ColumnID) const;
    ccstr url()  const noexcept { return table->url.get(id);  }
    ccstr name() const noexcept { return table->name.get(id); }
    // SETTER
    void  url(CString s)        { table->url.set(id, s);      }
    void  name(CString s)       { table->name.set(id, s);     }
  };

  using value_type = Style;
  using reference  = Style;
  using iterator   = GenericIterator<Styles>;

  value_type operator[](size_t id) { return value_type(this, id);   }
  iterator begin()                 { return iterator(this, 1);      }
  iterator end()                   { return iterator(this, size()); }
  value_type find(CString url, bool create);
};

struct Albums : public Table {
  StringColumn url;
  StringColumn title;
  StringColumn artist;
  StringColumn cover_url;
  StringColumn description;
  Column       date;
  Column       rating;
  Column       votes;
  Column       download_count;
  Column       styles;
  StringColumn archive_mp3;
  StringColumn archive_wav;
  StringColumn archive_flac;

  Albums(Database& db, StringPool& pool_album_url, StringPool& pool_cover_url, StringPool& pool_archive_url, StringPool& pool_desc, StringPool& pool_meta)
  : Table("albums", db,
    {&url,&title,&artist,&cover_url,&description,&date,&rating,&votes,
      &download_count,&styles,&archive_mp3,&archive_wav,&archive_flac})
  , url(pool_album_url)
  , title(pool_meta)
  , artist(pool_meta)
  , cover_url(pool_cover_url)
  , description(pool_desc)
  , archive_mp3(pool_archive_url)
  , archive_wav(pool_archive_url)
  , archive_flac(pool_archive_url)
  {
    resize(1); // Records with ID 0 represent a NULL value. Create them here.
  }

  struct Album : public Record<Albums*> {
    using Record::Record;

    static int shrinkDate(time_t T) { return (T / 60 / 60 / 24 - 10000);   }
    static time_t expandDate(int T) { return ((T + 10000) * 60 * 60 * 24); }

    // GETTER
    Field  operator[](ColumnID) const;
    ccstr  url()               const noexcept { return table->url.get(id);             }
    ccstr  title()             const noexcept { return table->title.get(id);           }
    ccstr  artist()            const noexcept { return table->artist.get(id);          }
    ccstr  cover_url()         const noexcept { return table->cover_url.get(id);       }
    ccstr  description()       const noexcept { return table->description.get(id);     }
    ccstr  archive_mp3_url()   const noexcept { return table->archive_mp3.get(id);     }
    ccstr  archive_wav_url()   const noexcept { return table->archive_wav.get(id);     }
    ccstr  archive_flac_url()  const noexcept { return table->archive_flac.get(id);    }
    time_t date()              const noexcept { return expandDate(table->date[id]);    }
    float  rating()            const noexcept { return float(table->rating[id]) / 100; }
    int    votes()             const noexcept { return table->votes[id];               }
    int    download_count()    const noexcept { return table->download_count[id];      }
    int    styles()            const noexcept { return table->styles[id];              }
    // SETTER
    void   url(CString s)              { table->url.set(id, s);               }
    void   title(CString s)            { table->title.set(id, s);             }
    void   artist(CString s)           { table->artist.set(id, s);            }
    void   cover_url(CString s)        { table->cover_url.set(id, s);         }
    void   description(CString s)      { table->description.set(id, s);       }
    void   archive_mp3_url(CString s)  { table->archive_mp3.set(id, s);       }
    void   archive_wav_url(CString s)  { table->archive_wav.set(id, s);       }
    void   archive_flac_url(CString s) { table->archive_flac.set(id, s);      }
    void   date(time_t t)              { table->date[id] = shrinkDate(t);     }
    void   rating(float i)             { table->rating[id] = i * 100;         }
    void   votes(int i)                { table->votes[id] = i;                }
    void   download_count(int i)       { table->download_count[id] = i;       }
    void   styles(int i)               { table->styles[id] = i;               }
  };

  using value_type = Album;
  using reference  = Album;
  using iterator   = GenericIterator<Albums>;

  value_type operator[](size_t id) { return value_type(this, id);   }
  iterator begin()                 { return iterator(this, 1);      }
  iterator end()                   { return iterator(this, size()); }
  value_type find(CString url, bool create);
};

struct Tracks : public Table {
  StringColumn  url;
  Column        album_id;
  StringColumn  title;
  StringColumn  artist;
  StringColumn  remix;
  Column        number;
  Column        bpm;

  Tracks(Database &db, StringPool& pool_track_url, StringPool& pool_meta)
  : Table("tracks", db, {&url,&album_id,&title,&artist,&remix,&number,&bpm})
  , url(pool_track_url)
  , title(pool_meta)
  , artist(pool_meta)
  , remix(pool_meta)
  {
    resize(1); // Records with ID 0 represent a NULL value. Create them here.
  }

  struct Track : public Record<Tracks*> {
    using Record::Record;

    // GETTER
    Field operator[](ColumnID) const;
    ccstr url()      const noexcept { return table->url.get(id);        }
    ccstr title()    const noexcept { return table->title.get(id);      }
    ccstr artist()   const noexcept { return table->artist.get(id);     }
    ccstr remix()    const noexcept { return table->remix.get(id);      }
    int   number()   const noexcept { return table->number[id];         }
    int   bpm()      const noexcept { return table->bpm[id];            }
    int   album_id() const noexcept { return table->album_id[id];       }
    Albums::Album album() const noexcept;
    // SETTER
    void  url(CString s)    { table->url.set(id, s);                     }
    void  title(CString s)  { table->title.set(id, s);                   }
    void  artist(CString s) { table->artist.set(id, s);                  }
    void  remix(CString s)  { table->remix.set(id, s);                   }
    void  number(int i)     { table->number[id] = i;                     }
    void  bpm(int i)        { table->bpm[id] = (i & 0xFF /* max 255 */); }
    void  album_id(int i)   { table->album_id[id] = i;                   }
  };

  using value_type = Track;
  using reference  = Track;
  using iterator   = GenericIterator<Tracks>;

  value_type operator[](size_t id) { return value_type(this, id);   }
  iterator begin()                 { return iterator(this, 1);      }
  iterator end()                   { return iterator(this, size()); }
  value_type find(CString url, bool create);
};

/* ==========================================================================
 * Order-By + Where
 * ========================================================================*/

enum SortOrder {
  ASCENDING,
  DESCENDING,
};

enum Operator {
  EQUAL,
  UNEQUAL,
  GREATER,
  GREATER_EQUAL,
  LESSER,
  LESSER_EQUAL,
};

class OrderBy {
  ColumnID  column;
  SortOrder order;
public:
  template<typename TColumn>
  OrderBy(TColumn column, SortOrder order = ASCENDING)
  : column(static_cast<ColumnID>(column))
  , order(order) {}

  template<typename T>
  bool operator()(const T& a, const T& b) const noexcept {
    int ret = a[column].compare(b[column]);
    return (order == ASCENDING ? ret < 0 : ret > 0);
  }
};

class Where {
  ColumnID column;
  Operator op;
  Field    field;
public:
  template<typename T>
  Where(ColumnID column, Operator op, T value)
  : column(column), op(op), field(value) {}

  template<typename T>
  bool operator()(const T& t) const noexcept {
    int ret = t[column].compare(field);
    switch (op) {
    case EQUAL:         return ! (ret == 0);
    case UNEQUAL:       return ! (ret != 0);
    case GREATER:       return ! (ret >  0);
    case GREATER_EQUAL: return ! (ret >= 0);
    case LESSER:        return ! (ret <  0);
    case LESSER_EQUAL:  return ! (ret <= 0);
    }
  }
};

class Database {
public:
  Styles styles;
  Albums albums;
  Tracks tracks;
  std::array<Table*, 3> tables;

  StringPool pool_meta;
  StringPool pool_desc;
  StringPool pool_style_url;
  StringPool pool_album_url;
  StringPool pool_track_url;
  StringPool pool_cover_url;
  StringPool pool_archive_url;
  std::array<StringPool*, 7> pools;

  Database();

  std::vector<Styles::Style> getStyles()
  { return std::vector<Styles::Style>(styles.begin(), styles.end()); }
  
  std::vector<Albums::Album> getAlbums()
  { return std::vector<Albums::Album>(albums.begin(), albums.end()); }

  std::vector<Tracks::Track> getTracks()
  { return std::vector<Tracks::Track>(tracks.begin(), tracks.end()); }

  const char* load(const std::string&) noexcept;
  const char* save(const std::string&) noexcept;
  void shrink_to_fit();

private:
  void shrink_pool_to_fit(StringPool&, std::initializer_list<Column*>);
};

} // namespace Database

#endif
