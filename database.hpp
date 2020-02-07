#ifndef _DATABASE_HPP
#define _DATABASE_HPP

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include "strpool.hpp"

/* ============================================================================
 * Track Database
 * ============================================================================
 *
 * The metadata is organized in tables in a classic relational way.
 *
 * === Tables ===
 * A table consists of a fixed number of columns.
 * For retrieving a single row a proxy object is returned.
 * A column is a specialized form of an int vector. If a column shall hold a
 * string value, than it must reference an ID to the stringpool.
 *
 * === Columns ===
 * A column is a bitpacked vector of integers. If a column should hold a string
 * it has to reference the string by an ID to a string pool.
 * The bit-width of a column changes.
 *
 * === String Pool ===
 * The stringpool itself is just one big allocated buffer that holds all the
 * null-terminated strings. The stringID is simply an integer offset to the
 * starting address of that buffer.
 *
 * Since searching the stringpool is expensive there are 3 different pools:
 * 1) URLs 2) Descriptions 3) Other Metadata
 *
 * === Loading and Saving the database ===
 * Loading and Saving is practally done by reading/writing the raw memory
 * to disk. Since the database file is not meant to be shared by other
 * machines/architectures this should be fine. A minimal form of data
 * validation is performed.
 *
 */

/* === GenericIterator ===
 * Make anything iterable that provides operator[] */
template<typename TStore, typename TItem>
class GenericIterator
: public std::iterator<std::random_access_iterator_tag, TItem> {
  TStore &store;
  size_t idx;
public:
  typedef GenericIterator iterator;

  GenericIterator(TStore &store, size_t idx) : store(store), idx(idx) {}

  bool operator==(const iterator&it) const { return idx == it.idx; }
  bool operator!=(const iterator&it) const { return idx != it.idx; }
  bool operator< (const iterator&it) const { return idx <  it.idx; }
  bool operator> (const iterator&it) const { return idx >  it.idx; }
  bool operator<=(const iterator&it) const { return idx <= it.idx; }
  bool operator>=(const iterator&it) const { return idx >= it.idx; }

  TItem operator*()                  const { return store[idx]; }
  TItem operator[](ptrdiff_t n)      const { return *(*this + n); }

  iterator& operator++()                  { ++idx; return *this; }
  iterator& operator--()                  { --idx; return *this; }
  iterator  operator++(int)       { iterator old = *this; ++idx; return old; }
  iterator  operator--(int)       { iterator old = *this; --idx; return old; }
  iterator& operator+=(ptrdiff_t n)       { idx += n; return *this; }
  iterator& operator-=(ptrdiff_t n)       { idx -= n; return *this; }
  iterator  operator+ (ptrdiff_t n) const { iterator i = *this; return i += n; }
  iterator  operator- (ptrdiff_t n) const { iterator i = *this; return i -= n; }
  iterator& operator= (const iterator&it) { idx=it.idx; return *this; } 
};

class Database {
public:
  typedef const char* ccstr;

  // === Column ===============================================================
  typedef std::vector<int> Column;

  // === Base class for all tables ============================================
  struct Table {
    Database &db;
    std::vector<Column*> columns;
    Table(Database &db, std::vector<Column*> columns)
      : db(db), columns(columns) { }

    size_t  size()              { return columns[0]->size();                }
    void    resize(size_t n)    { for (auto col : columns) col->resize(n);  }
    void    reserve(size_t n)   { for (auto col : columns) col->reserve(n); }

    bool load(std::ifstream&);
    bool save(std::ofstream&);
  };

  // === Base class for all records ===========================================
  struct Record {
    Database &db;
    size_t    id;
    inline Record(Database &db, size_t id) : db(db), id(id) {}
    inline bool valid() const { return id != -1; }
    Record& operator=(const Record &rhs) { id = rhs.id; return *this; }
    void swap(Record &rhs) { size_t tmp = id; id = rhs.id; rhs.id = tmp; } // XXX TODO
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
      ccstr url()   const;
      ccstr name()  const;
      // SETTER
      void  url(ccstr);
      void  name(ccstr);
    };

    Style operator[](size_t id);
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
    Albums(Database &db)
    : Table(db, {&url,&title,&artist,&cover_url,&description,&date,&rating,&votes,&download_count,&styles}) {}

    struct Album : public Record {
      Album(Database &db, size_t id) : Record(db, id) {}
      // GETTER
      ccstr   url()               const;
      ccstr   title()             const;
      ccstr   artist()            const;
      ccstr   cover_url()         const;
      ccstr   description()       const;
      time_t  date()              const;
      float   rating()            const;
      int     votes()             const;
      int     download_count()    const;
      int     styles()            const;
      // SETTER
      void    url(ccstr);
      void    title(ccstr);
      void    artist(ccstr);
      void    cover_url(ccstr);
      void    description(ccstr);
      void    date(time_t);
      void    rating(float);
      void    votes(int);
      void    download_count(int);
      void    styles(int);
    };

    Album operator[](size_t id);
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

    Track operator[](size_t id);
    Track find(const char *url, bool create);
  };

  /* ==========================================================================
   * Order-By + Where
   * ========================================================================*/

  enum ColumnID {
    STYLE_URL,
    STYLE_NAME,

    ALBUM_URL,
    ALBUM_TITLE,
    ALBUM_ARTIST,
    ALBUM_COVER_URL,
    ALBUM_DESCRIPTION,
    ALBUM_DATE,
    ALBUM_RATING,
    ALBUM_VOTES,
    ALBUM_DOWNLOAD_COUNT,

    TRACK_URL,
    TRACK_TITLE,
    TRACK_ARTIST,
    TRACK_REMIX,
    TRACK_NUMBER,
    TRACK_BPM,
  };

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
    int compare(const Styles::Style&, const Styles::Style&);
    int compare(const Albums::Album&, const Albums::Album&);
    int compare(const Tracks::Track&, const Tracks::Track&);
  public:
    OrderBy(ColumnID column, SortOrder order) : column(column), order(order) {}

    template<typename T>
    bool operator()(const T& a, const T& b) {
      int result = compare(a, b);
      return (order == ASCENDING ? result < 0 : result > 0);
    }
  };

  class Where {
    ColumnID     column;
    Operator     op;
    int          intValue;
    const char*  strValue;
    int compare(const Tracks::Track&);
    int compare(const Albums::Album&);
    int compare(const Styles::Style&);
  public:
    Where(ColumnID column, Operator op, int value)
    : column(column)
    , op(op)
    , intValue(value)
    , strValue("") { /* TODO: check if column is really an int */ }

    Where(ColumnID column, Operator op, const char* value)
    : column(column)
    , op(op)
    , intValue(0)
    , strValue(value) { /* TODO: check if column is really a string */ }

    template<typename T>
    bool operator()(const T& t) {
      int rval = compare(t);
      switch (op) {
        case EQUAL:         return ! (rval == 0);
        case UNEQUAL:       return ! (rval != 0);
        case GREATER:       return ! (rval >  0);
        case GREATER_EQUAL: return ! (rval >= 0);
        case LESSER:        return ! (rval <  0);
        case LESSER_EQUAL:  return ! (rval <= 0);
        default:            std::abort();
      }
    }
  };

  template<typename TStore, typename TRecord>
  struct Result {
    TStore &store;
    std::vector<int> indices;
    Result(TStore &store) : store(store) {
      indices.reserve(store.size());
      for (size_t i = 0; i < store.size(); ++i)
        indices.push_back(i);
    }

    TRecord operator[](size_t id) {
      return store[indices[id]];
    }
    GenericIterator<Result, TRecord> begin() {
      return GenericIterator<Result, TRecord>(*this, 0);
    }
    GenericIterator<Result, TRecord> end() {
      return GenericIterator<Result, TRecord>(*this, indices.size());
    }

    /* ========================================================================
     * Proxy objects
     * ======================================================================*/

    struct OrderByProxy {
      Result<TStore, TRecord> &result;
      OrderBy orderBy;
      OrderByProxy(Result<TStore, TRecord> &result, ColumnID column, SortOrder order)
      : result(result), orderBy(column, order) {}
      bool operator()(size_t a, size_t b) {
        return orderBy(result.store[a], result.store[b]);
      }
    };

    template<typename TValue>
    struct WhereProxy {
      Result<TStore, TRecord> &result;
      Where where;
      WhereProxy(Result<TStore, TRecord> &result, ColumnID column, Operator op, TValue value)
      : result(result), where(column, op, value) {}
      bool operator()(size_t i) {
        return where(result.store[i]);
      }
    };

    void order_by(ColumnID column, SortOrder order) {
      OrderByProxy orderProxy(*this, column, order);
      std::sort(indices.begin(), indices.end(), orderProxy);
    }

    template<typename TValue>
    void where(ColumnID column, Operator op, TValue value) {
      WhereProxy<TValue> whereProxy(*this, column, op, value);
      indices.erase(std::remove_if(indices.begin(), indices.end(), whereProxy), indices.end());
    }
  };

  /* ==========================================================================
   * Database members and methods - finally
   * ========================================================================*/

  std::string file;
  StringPool  pool;
  StringPool  pool_url;
  StringPool  pool_desc;
  Styles      styles;
  Albums      albums;
  Tracks      tracks;
  Database(const std::string &file)
  : file(file)
  , styles(*this)
  , albums(*this)
  , tracks(*this) {
    // We need a dummy style on ID 0, see implementation of Album::styles()
    styles.find("NULL", true).name("NULL");
  }

  Result<Styles, Styles::Style> getStyles() {
    return Result<Styles, Styles::Style>(styles);
  }

  Result<Albums, Albums::Album> getAlbums() {
    return Result<Albums, Albums::Album>(albums);
  }

  Result<Tracks, Tracks::Track> getTracks() {
    return Result<Tracks, Tracks::Track>(tracks);
  }

  bool load();
  bool save();
};

#endif
