#ifndef _DATABASE_HPP
#define _DATABASE_HPP

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include "strpool.hpp"

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
 * Since inserting into a stringpool is expensive (the whole buffer has to be
 * searched) and the [sub]string-deduplication makes only sense on similar
 * data, we use separate pools for each column.
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
 */

/* === GenericIterator ===
 * Make anything iterable that provides an index operator[] */
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
      ccstr   url()               const;
      ccstr   title()             const;
      ccstr   artist()            const;
      ccstr   cover_url()         const;
      ccstr   description()       const;
      ccstr   archive_mp3_url()   const;
      ccstr   archive_wav_url()   const;
      ccstr   archive_flac_url()  const;
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
      void    archive_mp3_url(ccstr);
      void    archive_wav_url(ccstr);
      void    archive_flac_url(ccstr);
      void    date(time_t);
      void    rating(float);
      void    votes(int);
      void    download_count(int);
      void    styles(int);
    };

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

    Track operator[](size_t id) { return Track(db, id); }
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
      int ret = compare(a, b);
      return (order == ASCENDING ? ret < 0 : ret > 0);
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
      int ret = compare(t);
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
   * Database members and methods - finally!
   * ========================================================================*/

  StringPool  pool_meta;
  StringPool  pool_desc;
  StringPool  pool_style_url;
  StringPool  pool_album_url;
  StringPool  pool_track_url;
  StringPool  pool_cover_url;
  StringPool  pool_mp3_url;
  StringPool  pool_wav_url;
  StringPool  pool_flac_url;
  Styles      styles;
  Albums      albums;
  Tracks      tracks;
  std::vector<StringPool*> pools;
  Database()
  : styles(*this)
  , albums(*this)
  , tracks(*this)
  , pools({&pool_meta, &pool_desc, &pool_style_url, &pool_album_url, &pool_track_url,
      &pool_cover_url, &pool_mp3_url, &pool_wav_url, &pool_flac_url})
  {
    // We need a dummy style on ID 0, see implementation of Album::styles()
    styles.find("NULL", true).name("NULL");

    // We know that the database will *at least* hold this amount of data,
    // so pre allocate the buffers. (January, 2020)
#define STYLES 25
#define ALBUMS 2078
#define TRACKS 14402
    styles.reserve(STYLES);
    albums.reserve(ALBUMS);
    tracks.reserve(TRACKS);
    // These are the average string lengths.
    pool_desc.reserve(ALBUMS * 720);
    pool_mp3_url.reserve(ALBUMS * 39);
    pool_wav_url.reserve(ALBUMS * 39);
    pool_flac_url.reserve(ALBUMS * 39);
    pool_cover_url.reserve(ALBUMS * 35);
    pool_album_url.reserve(ALBUMS * 22);
    pool_track_url.reserve(TRACKS * 30);
    pool_style_url.reserve(STYLES * 7);
    pool_meta.reserve(ALBUMS * 15 + TRACKS * 25);
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

  bool load(const std::string&);
  bool save(const std::string&);
};

#endif
