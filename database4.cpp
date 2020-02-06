#include <vector>
#include <string>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <cstdlib>
#include "strpool.cpp"
#define  streq(a,b) (!strcmp(a,b))
typedef const char* ccstr;

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
 * string value, than it must reference an id to the stringpool.
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
 * === Loading and Saving the database ===
 * Loading and Saving is practally done by reading/writing the raw memory
 * to disk. Since the database file is not meant to be shared by other
 * machines/architectures this should be fine. A minimal form of data
 * validation is performed.
 *
 */

// === Save/Load Functions ====
template<typename T>
void saveVector(std::ofstream &fs, const std::vector<T> &v) { }

template<>
void saveVector(std::ofstream &fs, const std::vector<const char*> &v) { }

template<typename T>
void loadVector(std::ifstream &fs, std::vector<T> &v) { }

template<>
void loadVector(std::ifstream &fs, std::vector<const char*> &v) { }

/* === GenericIterator ===
 * Make anything iterable that provides operator[] */
template<typename TStore, typename TItem>
struct GenericIterator
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

// === Containers ===
#define str_assign(DEST, STR) \
  if (! streq(db.pool.get(DEST), STR)) DEST = db.pool.add(STR)

#define str_assign_long(DEST, STR) \
  if (! streq(db.pool_long.get(DEST), STR)) DEST = db.pool_long.add(STR)

struct Database {
  struct Table {
    /* Base class for all tables. */
    Database &db;
    std::vector<std::vector<int>*> columns;
    Table(Database &db, std::vector<std::vector<int>*> columns)
      : db(db), columns(columns) { }

    size_t size()           { return columns[0]->size();                }
    void resize(size_t n)   { for (auto col : columns) col->resize(n);  }
    void reserve(size_t n)  { for (auto col : columns) col->reserve(n); }

    void load(std::ifstream &fs) {
      for (auto col: columns)
        loadVector(fs, *col);
    }

    void save(std::ofstream &fs) {
      for (auto col : columns)
        saveVector(fs, *col);
    }
  };

  struct Record {
    Database &db;
    size_t    id;
    inline Record(Database &db, size_t id) : db(db), id(id) {}
    inline bool valid() const { return id != -1; }
    Record& operator=(const Record &rhs) { id = rhs.id; return *this; }
    void swap(Record &rhs) { size_t tmp = id; id = rhs.id; rhs.id = tmp; } // XXX TODO
  };

  struct Styles : public Table {
    std::vector<int>  url;
    std::vector<int>  name;
    Styles(Database &db)
    : Table(db, {&url,&name}) {}

    struct Style : public Record {
      Style(Database &db, size_t id) : Record(db, id) {}

      const char* url()     const;
      const char* name()    const;

      void        url(const char*);
      void        name(const char*);
    };

    Style operator[](size_t id);
    Style find(const char *url, bool create);
  };

  struct Albums : public Table {
    std::vector<int>   url;
    std::vector<int>   title;
    std::vector<int>   artist;
    std::vector<int>   cover_url;
    std::vector<int>   description;
    std::vector<int>   date;
    std::vector<int>   rating;
    std::vector<int>   votes;
    std::vector<int>   download_count;
    std::vector<int>   styles;
    Albums(Database &db)
    : Table(db, {&url,&title,&artist,&cover_url,&description,&date,&rating,&votes,&download_count,&styles}) {}

    struct Album : public Record {
      Album(Database &db, size_t id) : Record(db, id) {}

      const char* url()               const;
      const char* title()             const;
      const char* artist()            const;
      const char* cover_url()         const;
      const char* description()       const;
      time_t      date()              const;
      float       rating()            const;
      int         votes()             const;
      int         download_count()    const;
      int         styles()            const;

      void        url(const char*);
      void        title(const char*);
      void        artist(const char*);
      void        cover_url(const char*);
      void        description(const char*);
      void        date(time_t);
      void        rating(float);
      void        votes(int);
      void        download_count(int);
      void        styles(int);
    };

    Album operator[](size_t id);
    Album find(const char *url, bool create);
  };

  struct Tracks : public Table {
    std::vector<int>    url;
    std::vector<int>    album_id;
    std::vector<int>    title;
    std::vector<int>    artist;
    std::vector<int>    remix;
    std::vector<int>    number;
    std::vector<int>    bpm;
    Tracks(Database &db)
    : Table(db, {&url,&album_id,&title,&artist,&remix,&number,&bpm}) {}

    struct Track : public Record {
      Track(Database &db, size_t id) : Record(db, id) {}

      const char*   url()         const;
      const char*   title()       const;
      const char*   artist()      const;
      const char*   remix()       const;
      int           number()      const;
      int           bpm()         const;
      int           album_id()    const;
      Albums::Album album()       const;

      void          url(const char*);
      void          title(const char*);
      void          artist(const char*);
      void          remix(const char*);
      void          number(int);
      void          bpm(int);
      void          album_id(int);
    };

    Track operator[](size_t id);
    Track find(const char *url, bool create);
  };

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
    DESCENDING
  };

  enum Operator {
    EQUAL,
    UNEQUAL,
    GREATER,
    GREATER_EQUAL,
    LESSER,
    LESSER_EQUAL
  };

  struct OrderBy {
    ColumnID  column;
    SortOrder order;
    OrderBy(ColumnID column, SortOrder order) : column(column), order(order) {}

    bool operator()(const Styles::Style &a, const Styles::Style &b) {
      int c;
      switch (column) {
        case STYLE_URL:   c = strcmp(a.url(),  b.url());    break;
        case STYLE_NAME:  c = strcmp(a.name(), b.name());   break;
        default:          return false;  
      }
      return (order == ASCENDING ? c < 0 : c > 0);
    }

    bool operator()(const Albums::Album &a, const Albums::Album &b) {
      int c;
      switch (column) {
        case ALBUM_URL:         c = strcmp(a.url(),        b.url());    break;
        case ALBUM_TITLE:       c = strcmp(a.title(),      b.title());  break;
        case ALBUM_ARTIST:      c = strcmp(a.artist(),     b.artist()); break;
        case ALBUM_COVER_URL:   c = strcmp(a.cover_url(),  b.cover_url());  break;
        case ALBUM_DESCRIPTION: c = strcmp(a.description(),b.description()); break;
        case ALBUM_DATE:        c = a.date() -     b.date();            break;
        case ALBUM_RATING:      c = a.rating() -   b.rating();          break;
        case ALBUM_VOTES:       c = a.votes() -    b.votes();           break;
        case ALBUM_DOWNLOAD_COUNT: c = a.download_count() - b.download_count(); break;
        default:                return false;
      }
      return (order == ASCENDING ? c < 0 : c > 0);
    }

    bool operator()(const Tracks::Track &a, const Tracks::Track &b) {
      int c;
      switch (column) {
        case TRACK_URL:    c = strcmp(a.url(),      b.url());    break;
        case TRACK_TITLE:  c = strcmp(a.title(),    b.title());  break;
        case TRACK_ARTIST: c = strcmp(a.artist(),   b.artist()); break;
        case TRACK_REMIX:  c = strcmp(a.remix(),    b.remix());  break;
        case TRACK_NUMBER: c =        a.number() -  b.number();  break;
        case TRACK_BPM:    c =        a.bpm() -     b.bpm();     break;
        default:           return (*this)(a.album(), b.album()); break;
      }
      return (order == ASCENDING ? c < 0 : c > 0);
    }
  };

  struct Where {
    ColumnID       column;
    Operator       op;
    int            intValue;
    const char*    strValue;

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

    int compare(const Tracks::Track &t) {
      switch (column) {
        case TRACK_URL:    return strcmp(t.url(),    strValue);
        case TRACK_TITLE:  return strcmp(t.title(),  strValue);
        case TRACK_ARTIST: return strcmp(t.artist(), strValue);
        case TRACK_REMIX:  return strcmp(t.remix(),  strValue);
        case TRACK_NUMBER: return t.number() -       intValue;
        case TRACK_BPM:    return t.bpm() -          intValue;
        default:           return compare(t.album());
      }
    }

    int compare(const Albums::Album &a) {
      switch (column) {
        case ALBUM_URL:             return strcmp(a.url(),          strValue);
        case ALBUM_TITLE:           return strcmp(a.title(),        strValue);
        case ALBUM_ARTIST:          return strcmp(a.artist(),       strValue);
        case ALBUM_COVER_URL:       return strcmp(a.cover_url(),    strValue);
        case ALBUM_DESCRIPTION:     return strcmp(a.description(),  strValue);
        case ALBUM_DATE:            return a.date() -               intValue;
        case ALBUM_RATING:          return a.rating() -             intValue;
        case ALBUM_VOTES:           return a.votes() -              intValue;
        case ALBUM_DOWNLOAD_COUNT:  return a.download_count() -     intValue;
        default:                    std::abort();
      }
    }

    int compare(const Styles::Style &s) {
      switch (column) {
        case STYLE_URL:     return strcmp(s.url(),    strValue);
        case STYLE_NAME:    return strcmp(s.name(),   strValue);
        default:            std::abort();
      }
    }

    bool operator()(const Tracks::Track &t) { return check(compare(t)); }
    bool operator()(const Albums::Album &a) { return check(compare(a)); }
    bool operator()(const Styles::Style &s) { return check(compare(s)); }
    bool check(int rval) {
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

    struct OrderByProxy {
      Result<TStore, TRecord> &result;
      OrderBy orderBy;
      OrderByProxy(Result<TStore, TRecord> &result, ColumnID column, SortOrder order)
      : result(result), orderBy(column, order) {}
      bool operator()(size_t a, size_t b) {
        return orderBy(result.store[a], result.store[b]);
      }
    };

    void order_by(ColumnID column, SortOrder order) {
      OrderByProxy orderProxy(*this, column, order);
      std::sort(indices.begin(), indices.end(), orderProxy);
    }

    struct WhereProxy {
      Result<TStore, TRecord> &result;
      Where where;
      WhereProxy(Result<TStore, TRecord> &result, ColumnID column, Operator op, int value)
      : result(result), where(column, op, value) {}
      WhereProxy(Result<TStore, TRecord> &result, ColumnID column, Operator op, const char* value)
      : result(result), where(column, op, value) {}
      bool operator()(size_t i) {
        return where(result.store[i]);
      }
    };

    void where(ColumnID column, Operator op, int value) {
      WhereProxy whereProxy(*this, column, op, value);
      indices.erase(std::remove_if(indices.begin(), indices.end(), whereProxy), indices.end());
    }

    void where(ColumnID column, Operator op, const char* value) {
      WhereProxy whereProxy(*this, column, op, value);
      indices.erase(std::remove_if(indices.begin(), indices.end(), whereProxy), indices.end());
    }
  };

  Result<Styles, Styles::Style> getStyles() {
    return Result<Styles, Styles::Style>(styles);
  }

  Result<Albums, Albums::Album> getAlbums() {
    return Result<Albums, Albums::Album>(albums);
  }

  Result<Tracks, Tracks::Track> getTracks() {
    return Result<Tracks, Tracks::Track>(tracks);
  }

  std::string file;
  StringPool  pool;
  StringPool  pool_long;
  Styles      styles;
  Albums      albums;
  Tracks      tracks;
  Database(const std::string &file)
  : file(file)
  , styles(*this)
  , albums(*this)
  , tracks(*this) {}
};

/* ============================================================================
 * Styles
 * ==========================================================================*/

Database::Styles::Style Database::Styles::operator[](size_t id) {
  return Database::Styles::Style(db, id); // XXX huh.
}

Database::Styles::Style Database :: Styles :: find(const char *url, bool create) {
  size_t i = this->size();
  while (i--)
    if (streq(db.pool.get(this->url[i]), url))
      return Database::Styles::Style(db, i);

  if (! create)
    return Database::Styles::Style(db, -1);

  i = this->size();
  this->resize(i+1);
  auto r = Database::Styles::Style(db, i);
  r.url(url);
  return r;
}

#define STYLE Database :: Styles :: Style
ccstr   STYLE :: url()  const   { return db.pool.get(db.styles.url[id]);      }
ccstr   STYLE :: name() const   { return db.pool.get(db.styles.name[id]);     }

void    STYLE :: url(ccstr s)   { str_assign(db.styles.url[id], s);           }
void    STYLE :: name(ccstr s)  { str_assign(db.styles.name[id], s);          }

/* ============================================================================
 * Albums
 * ==========================================================================*/

Database::Albums::Album Database::Albums::operator[](size_t id) {
  return Database::Albums::Album(db, id); // XXX huh.
}

Database::Albums::Album Database::Albums::find(const char *url, bool create) {
  size_t i = this->size();
  while (i--)
    if (streq(db.pool.get(this->url[i]), url))
      return Database::Albums::Album(db, i);

  if (! create)
    return Database::Albums::Album(db, -1);

  i = this->size();
  this->resize(i+1);
  auto r = Database::Albums::Album(db, i);
  r.url(url);
  return r;
}

#define ALBUM Database :: Albums :: Album
ccstr  ALBUM::url()            const { return db.pool.get(db.albums.url[id]);         }
ccstr  ALBUM::title()          const { return db.pool.get(db.albums.title[id]);       }
ccstr  ALBUM::artist()         const { return db.pool.get(db.albums.artist[id]);      }
ccstr  ALBUM::cover_url()      const { return db.pool.get(db.albums.cover_url[id]);   }
ccstr  ALBUM::description()    const { return db.pool_long.get(db.albums.description[id]); }
time_t ALBUM::date()           const { return db.albums.date[id] * 60 * 60 * 24;      }
float  ALBUM::rating()         const { return (float) db.albums.rating[id] / 100;     }
int    ALBUM::votes()          const { return db.albums.votes[id];                    }
int    ALBUM::download_count() const { return db.albums.download_count[id];           }
int    ALBUM::styles()         const { return db.albums.styles[id];                   }

void   ALBUM::url(ccstr s)          { str_assign(db.albums.url[id], s);         }
void   ALBUM::title(ccstr s)        { str_assign(db.albums.title[id], s);       }
void   ALBUM::artist(ccstr s)       { str_assign(db.albums.artist[id], s);      }
void   ALBUM::cover_url(ccstr s)    { str_assign(db.albums.cover_url[id], s);   }
void   ALBUM::description(ccstr s)  { str_assign_long(db.albums.description[id], s); }
void   ALBUM::date(time_t t)        { db.albums.date[id] = t / 60 / 60 / 24;    }
void   ALBUM::rating(float i)       { db.albums.rating[id] = i * 100;           }
void   ALBUM::votes(int i)          { db.albums.votes[id] = i;                  }
void   ALBUM::download_count(int i) { db.albums.download_count[id] = i;         }
void   ALBUM::styles(int i)         { db.albums.styles[id] = i;                 }

/* ============================================================================
 * Tracks
 * ==========================================================================*/

Database::Tracks::Track Database::Tracks::operator[](size_t id) {
  return Database::Tracks::Track(db, id); // XXX huh.
}

Database::Tracks::Track Database::Tracks::find(const char* url, bool create) {
  size_t i = this->size();
  while (i--)
    if (streq(db.pool.get(this->url[i]), url))
      return Database::Tracks::Track(db, i);

  if (! create)
    return Database::Tracks::Track(db, -1);

  i = this->size();
  this->resize(i+1);
  auto r = Database::Tracks::Track(db, i);
  r.url(url);
  return r;
}

#define TRACK Database :: Tracks :: Track
ccstr TRACK :: url()      const { return db.pool.get(db.tracks.url[id]);      }
ccstr TRACK :: title()    const { return db.pool.get(db.tracks.title[id]);    }
ccstr TRACK :: artist()   const { return db.pool.get(db.tracks.artist[id]);   }
ccstr TRACK :: remix()    const { return db.pool.get(db.tracks.remix[id]);    }
int   TRACK :: number()   const { return db.tracks.number[id];                }
int   TRACK :: bpm()      const { return db.tracks.bpm[id];                   }
int   TRACK :: album_id() const { return db.tracks.album_id[id];              }
ALBUM TRACK :: album()    const { return db.albums[db.tracks.album_id[id]];   }

void  TRACK :: url(ccstr s)     { str_assign(db.tracks.url[id],    s);        }
void  TRACK :: title(ccstr s)   { str_assign(db.tracks.title[id],  s);        }
void  TRACK :: artist(ccstr s)  { str_assign(db.tracks.artist[id], s);        }
void  TRACK :: remix(ccstr s)   { str_assign(db.tracks.remix[id],  s);        }
void  TRACK :: number(int i)    { db.tracks.number[id] =           i;         }
void  TRACK :: bpm(int i)       { db.tracks.bpm[id] =              i;         }
void  TRACK :: album_id(int i)  { db.tracks.album_id[id] =         i;         }
// ============================================================================

#if TEST_DATABASE
#include <iostream>
int main () {
  std::vector<int> v = {1,2,3,4,5};

  GenericIterator<std::vector<int>, int> beg(v, 0);
  GenericIterator<std::vector<int>, int> end(v, v.size());

  for (; beg != end; ++beg) {
    std::cout << *beg << std::endl;
  }

  Database db("/tmp/remove.db");
  auto style = db.styles.find("foo", true);
  style.name("Foo-Style");
  auto s1 = db.styles.find("bar", true);
  s1.name("Bar-Style");

  for (auto s : db.getStyles()) {
    std::cout << s.url() << ": " << s.name() << std::endl;
  }
}
#endif
