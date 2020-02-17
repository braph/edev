#ifndef _DATABASE_HPP
#define _DATABASE_HPP

#include "strpool.hpp"
#include "common.hpp"

#include <array>
#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include <utility>
#include <algorithm>

#define DB_ABI_VERSION 1 // Change this each time the DB ABI breaks

/* ============================================================================
 * Metadata Database
 * ============================================================================
 *
 * The metadata is organized in tables in a classic relational way.
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
 * Records with ID = 0 (first ROW) are used for representing a NULL value.
 * The row count of a table will therefore be off by one.
 * It is easier to do a `-1` on the return value of Table::size() than applying
 * this -1 logic all over the place. this won't be fixed.
 *
 * The reason for that is also the implementation of the Album::styles(). //XXX
 */

/* Make anything iterable that provides operator[] */
template<typename TContainer>
class GenericIterator
: public std::iterator<std::random_access_iterator_tag, typename TContainer::value_type> {
public:
  typedef GenericIterator iterator;
  typedef typename TContainer::value_type value_type;

  GenericIterator(TContainer &container, size_t idx) : container(container), idx(idx) {}

  bool operator==(const iterator&it) const { return idx == it.idx; }
  bool operator!=(const iterator&it) const { return idx != it.idx; }
  bool operator< (const iterator&it) const { return idx <  it.idx; }
  bool operator> (const iterator&it) const { return idx >  it.idx; }
  bool operator<=(const iterator&it) const { return idx <= it.idx; }
  bool operator>=(const iterator&it) const { return idx >= it.idx; }

  value_type operator*()             const { return container[idx]; }
  value_type operator[](ptrdiff_t n) const { return *(*this + n);   }

  iterator& operator++()                  { ++idx; return *this; }
  iterator& operator--()                  { --idx; return *this; }
  iterator  operator++(int)         { iterator old = *this; ++idx; return old; }
  iterator  operator--(int)         { iterator old = *this; --idx; return old; }
  iterator& operator+=(ptrdiff_t n)       { idx += n; return *this;            }
  iterator& operator-=(ptrdiff_t n)       { idx -= n; return *this;            }
  iterator  operator+ (ptrdiff_t n) const { iterator i = *this; return i += n; }
  iterator  operator- (ptrdiff_t n) const { iterator i = *this; return i -= n; }
  iterator& operator= (const iterator&it) { idx = it.idx; return *this;        } 
private:
  TContainer &container;
  size_t idx;
};

class Database {
public:
  typedef const char* ccstr;

  /* ==========================================================================
   * ColumnIDs
   *
   * A single field of a record can be accessed using the index operator[].
   * Although there are multiple enum types defined all functions use the
   * `ColumnID` type as parameter.
   *
   * Not all of these IDs correspond to a real field. Some of them are just a
   * `view` of another column (e.g. `ALBUM_DAY` returns day of `ALBUM_DATE`).
   * ========================================================================*/

  enum ColumnID {
    COLUMN_NONE = 0
  };

  enum StyleColumnID {
    STYLE_URL = COLUMN_NONE + 1,
    STYLE_NAME,
  };

  enum AlbumColumnID {
    ALBUM_URL = STYLE_NAME + 1,
    ALBUM_TITLE,
    ALBUM_ARTIST,
    ALBUM_COVER_URL,
    ALBUM_DESCRIPTION,
    ALBUM_DATE,
    ALBUM_DAY,   // -.
    ALBUM_MONTH, //  +- View
    ALBUM_YEAR,  // -'
    ALBUM_RATING,
    ALBUM_VOTES,
    ALBUM_DOWNLOAD_COUNT,
  };

  enum TrackColumnID {
    TRACK_URL = ALBUM_DOWNLOAD_COUNT + 1,
    TRACK_TITLE,
    TRACK_ARTIST,
    TRACK_REMIX,
    TRACK_NUMBER,
    TRACK_BPM,
  };

  static ColumnID columnIDFromStr(const std::string &s) { //XXX make array
    /**/ if (s == "style")        return static_cast<ColumnID>(STYLE_NAME);
    else if (s == "album")        return static_cast<ColumnID>(ALBUM_TITLE);
    else if (s == "album_artist") return static_cast<ColumnID>(ALBUM_ARTIST);
    else if (s == "description")  return static_cast<ColumnID>(ALBUM_DESCRIPTION);
    else if (s == "date")         return static_cast<ColumnID>(ALBUM_DATE);
    else if (s == "rating")       return static_cast<ColumnID>(ALBUM_RATING);
    else if (s == "votes")        return static_cast<ColumnID>(ALBUM_VOTES);
    else if (s == "downloads")    return static_cast<ColumnID>(ALBUM_DOWNLOAD_COUNT);
    else if (s == "day")          return static_cast<ColumnID>(ALBUM_DAY);
    else if (s == "month")        return static_cast<ColumnID>(ALBUM_MONTH);
    else if (s == "year")         return static_cast<ColumnID>(ALBUM_YEAR);
    else if (s == "title")        return static_cast<ColumnID>(TRACK_TITLE);
    else if (s == "artist")       return static_cast<ColumnID>(TRACK_ARTIST);
    else if (s == "remix")        return static_cast<ColumnID>(TRACK_REMIX);
    else if (s == "number")       return static_cast<ColumnID>(TRACK_NUMBER);
    else if (s == "bpm")          return static_cast<ColumnID>(TRACK_BPM);
    else if (s == "styles")       return static_cast<ColumnID>(TRACK_NUMBER); // TODO
    else                          return static_cast<ColumnID>(COLUMN_NONE);
  }

  /* ==========================================================================
   * The field struct is basically a union type.
   * ========================================================================*/
  struct Field {
    enum Type {
      STRING,
      INTEGER,
      FLOAT,
      TIME,
    };

    Type type;
    union {
      ccstr s;
      int i;
      float f;
      time_t t;
    } value;

    inline Field(ccstr s)   { setString(s);  }
    inline Field(int i)     { setInteger(i); }
    inline Field(float f)   { setFloat(f);   }
    inline Field(time_t t)  { setTime(t);    }

    inline void setString(ccstr s) {
      this->value.s = s;
      this->type = STRING;
    }

    inline void setInteger(int i) {
      this->value.i = i;
      this->type = INTEGER;
    }

    inline void setFloat(float f) {
      this->value.f = f;
      this->type = FLOAT;
    }
    
    inline void setTime(time_t t) {
      this->value.t = t;
      this->type = TIME;
    }

    int compare(const Field &rhs) const {
      assert(type == rhs.type);
      switch (type) {
      case STRING:  return strcmp(this->value.s, rhs.value.s);
      case INTEGER: return this->value.i - rhs.value.i;
      case FLOAT:   return this->value.f - rhs.value.f;
      case TIME:    return this->value.t - rhs.value.t;
      }
    }
  };

  // === Column ===============================================================
  // TODO: Implement the bitpacked vector
  typedef std::vector<int> Column;

  // === Base class for all tables ============================================
  struct Table {
    Database &db;
    std::vector<Column*> columns;
    Table(Database &db, std::vector<Column*> columns)
    : db(db)
    , columns(columns) { }

    size_t  size()              { return columns[0]->size();            }
    void    resize(size_t n)    { for (auto c : columns) c->resize(n);  }
    void    reserve(size_t n)   { for (auto c : columns) c->reserve(n); }

    void load(std::ifstream&);
    void save(std::ofstream&);
  };

  // === Base class for all records ===========================================
  struct Record {
    Database &db;
    size_t    id;
    inline Record(Database &db, size_t id) : db(db), id(id) {}
    inline operator bool()              const { return id != 0;      }
    bool operator!=(const Record &rhs)  const { return id != rhs.id; }
    bool operator==(const Record &rhs)  const { return id == rhs.id; }
    Record& operator=(const Record &rhs)      { id = rhs.id; return *this; }
  };

  /* ==========================================================================
   * Table definitions begin here
   * ========================================================================*/

  struct Styles : public Table {
    Column url;
    Column name;
    Styles(Database &db)
    : Table(db, {&url,&name}) {}

    struct Style : public Record {
      Style(Database &db, size_t id) : Record(db, id) {}
      // GETTER
      Field operator[](ColumnID) const;
      ccstr url()   const;
      ccstr name()  const;
      // SETTER
      void  url(ccstr);
      void  name(ccstr);
    };

    typedef Style value_type;

    Style operator[](size_t id) { return Style(db, id); }
    Style find(const char *url, bool create);
  };

  struct Albums : public Table {
    Column url;
    Column title;
    Column artist;
    Column cover_url;
    Column description;
    Column date;
    Column rating;
    Column votes;
    Column download_count;
    Column styles;
    Column archive_mp3;
    Column archive_wav;
    Column archive_flac;
    Albums(Database &db)
    : Table(db,
      {&url,&title,&artist,&cover_url,&description,&date,&rating, &votes,
        &download_count,&styles,&archive_mp3,&archive_wav,&archive_flac}) {}

    struct Album : public Record {
      Album(Database &db, size_t id) : Record(db, id) {}
      // GETTER
      Field   operator[](ColumnID) const;
      ccstr   url()                const;
      ccstr   title()              const;
      ccstr   artist()             const;
      ccstr   cover_url()          const;
      ccstr   description()        const;
      ccstr   archive_mp3_url()    const;
      ccstr   archive_wav_url()    const;
      ccstr   archive_flac_url()   const;
      time_t  date()               const;
      float   rating()             const;
      int     votes()              const;
      int     download_count()     const;
      int     styles()             const;
      // SETTER
      void    url(ccstr);
      void    title(ccstr);
      void    artist(ccstr);
      void    cover_url(ccstr);
      void    description(ccstr);
      void    archive_mp3_url(ccstr);
      void    archive_wav_url(ccstr);
      void    archive_flac_url(ccstr);
      void    date(time_t);
      void    rating(float);
      void    votes(int);
      void    download_count(int);
      void    styles(int);
    };
    typedef Album value_type;

    Album operator[](size_t id) { return Album(db, id); }
    Album find(const char *url, bool create);
  };

  struct Tracks : public Table {
    Column url;
    Column album_id;
    Column title;
    Column artist;
    Column remix;
    Column number;
    Column bpm;
    Tracks(Database &db)
    : Table(db, {&url,&album_id,&title,&artist,&remix,&number,&bpm}) {}

    struct Track : public Record {
      Track(Database &db, size_t id) : Record(db, id) {}
      // GETTER
      Field   operator[](ColumnID) const;
      ccstr   url()         const;
      ccstr   title()       const;
      ccstr   artist()      const;
      ccstr   remix()       const;
      int     number()      const;
      int     bpm()         const;
      int     album_id()    const;
      Albums::Album album() const;
      // SETTER
      void    url(ccstr);
      void    title(ccstr);
      void    artist(ccstr);
      void    remix(ccstr);
      void    number(int);
      void    bpm(int);
      void    album_id(int);
    };
    typedef Track value_type;

    Track operator[](size_t id) { return Track(db, id); }
    Track find(const char *url, bool create);
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
    OrderBy(ColumnID column, SortOrder order) : column(column), order(order) {}

    template<typename T>
    bool operator()(const T& a, const T& b) {
      int ret = a[column].compare(b[column]);
      return (order == ASCENDING ? ret < 0 : ret > 0);
    }
  };

  class Where {
    ColumnID     column;
    Operator     op;
    Field        field;
  public:
    Where(ColumnID column, Operator op, int value)
    : column(column), op(op), field(value) {}

    Where(ColumnID column, Operator op, float value)
    : column(column), op(op), field(value) {}

    Where(ColumnID column, Operator op, const char* value)
    : column(column), op(op), field(value) {}

    template<typename T>
    bool operator()(const T& t) {
      int ret = t.compare(field);
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

  template<typename TStore>
  class Result {
    TStore &store;
    std::vector<int> indices;
  public:
    Result(TStore &store) : store(store) {
      indices.reserve(store.size());
      for (size_t i = 1 /* Skip NULL Record */; i < store.size(); ++i)
        indices.push_back(i);
    }

    typedef typename TStore::value_type value_type;

    inline size_t size() {
      return indices.size();
    }

    value_type operator[](size_t id) {
      return store[indices[id]];
    }

    GenericIterator<Result> begin() {
      return GenericIterator<Result>(*this, 0);
    }

    GenericIterator<Result> end() {
      return GenericIterator<Result>(*this, size());
    }

    /* ========================================================================
     * Proxy objects
     * ======================================================================*/

    template<typename TValue>
    struct WhereProxy {
      Result<TStore> &result;
      Where where;
      WhereProxy(Result<TStore> &result, ColumnID column, Operator op, TValue value)
      : result(result), where(column, op, value) {}
      bool operator()(size_t i) {
        return where(result.store[i]);
      }
    };

    void order_by(ColumnID column, SortOrder order) {
      OrderBy orderBy(column, order);
      std::sort(indices.begin(), indices.end(), [&](size_t a, size_t b) {
        return orderBy(this->store[a], this->store[b]);
      });
    }

    template<typename TValue>
    void where(ColumnID column, Operator op, TValue value) {
      WhereProxy<TValue> whereProxy(*this, column, op, value);
      indices.erase(std::remove_if(indices.begin(), indices.end(), whereProxy), indices.end());
    }
  };

  /* ==========================================================================
   * Database members and methods - finally!
   * ========================================================================*/

  StringPool  pool_meta;
  StringPool  pool_desc;
  StringPool  pool_style_url;
  StringPool  pool_album_url;
  StringPool  pool_track_url;
  StringPool  pool_cover_url;
  StringPool  pool_archive_url;
  Styles      styles;
  Albums      albums;
  Tracks      tracks;
  std::array<StringPool*, 7> pools;

  Database();

  Result<Styles> getStyles() {
    return Result<Styles>(styles);
  }

  Result<Albums> getAlbums() {
    return Result<Albums>(albums);
  }

  Result<Tracks> getTracks() {
    return Result<Tracks>(tracks);
  }

  void load(const std::string&);
  void save(const std::string&);
  void shrink_to_fit();
private:
  void shrink_pool_to_fit(StringPool&, std::initializer_list<Column*>);
};

#endif
