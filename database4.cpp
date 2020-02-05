#include <vector>
#include <string>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <cassert> //XXX
#include <iostream> //XXX
#include "strpool.cpp"
#define  streq(a,b) (!strcmp(a,b))

// === Save/Load Functions ====
template<typename T>
void saveVector(std::ofstream &fs, const std::vector<T> &v) {
  size_t size = v.size();
  fs.write((char*) &size, sizeof(size));     // element count
  fs.write((char*) &v[0], sizeof(T) * size); // data
}

template<>
void saveVector(std::ofstream &fs, const std::vector<const char*> &v) {
  size_t size = v.size();
  fs.write((char*) &size, sizeof(size)); // element count

  size = 0;
  for (const auto &e : v)
    size += strlen(e) + 1;
  fs.write((char*) &size, sizeof(size)); // data size

  for (const auto &e : v)
    fs.write(e, strlen(e) + 1);          // data
}

template<typename T>
void loadVector(std::ifstream &fs, std::vector<T> &v) {
  size_t size = 0;
  fs.read((char*) &size, sizeof(size));     // element count
  v.resize(size);
  fs.read((char*) &v[0], sizeof(T) * size); // data
}

template<>
void loadVector(std::ifstream &fs, std::vector<const char*> &v) {
  size_t size = 0, data_size = 0;
  fs.read((char*) &size,      sizeof(size));      // element count
  fs.read((char*) &data_size, sizeof(data_size)); // data size
  v.reserve(size);
  char *data = new char[data_size]; // XXX: This leaks memory
  fs.read(data, data_size);
  while (size--) {
    v.push_back(data);
    data += strlen(data) + 1;
  }
}

// === Iterator ===
template<typename TStore, typename TItem>
class GenericIterator {
  size_t idx;
  TStore &store;
public:
  GenericIterator(TStore &store, size_t idx) : store(store), idx(idx) {}

  typedef GenericIterator iterator;

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

#if 0
  inline ptrdiff_t
  operator-(const _Bit_iterator_base& __x, const _Bit_iterator_base& __y)
  {
    return (int(_S_word_bit) * (__x._M_p - __y._M_p)
	    + __x._M_offset - __y._M_offset);
  }
  inline _Bit_iterator
  operator+(ptrdiff_t __n, const _Bit_iterator& __x)
  { return __x + __n; }
#endif
};

// === Containers ===
#define str_assign(DEST, STR) \
  if (!DEST || !streq(DEST, STR)) DEST = strdup(STR)

// ===== FOOO NOW WE ARE DOING THE TABLE STUFF FO FOO BAR

struct Database {
  struct Table {
    /* This holds columns and functions to save them to a file */
    Database &db;
    std::vector<std::vector<int>*> columns;
    Table(Database &db, std::vector<std::vector<int>*> columns)
      : db(db), columns(columns) { }

    void load(std::ifstream &fs) {
      for (auto col: columns)
        loadVector(fs, *col);
    }

    void save(std::ofstream &fs) {
      for (auto col : columns)
        saveVector(fs, *col);
    }

    void reserve(size_t n) {
      for (auto col : columns)
        col->reserve(n);
    }

    void resize(size_t n) {
      for (auto col : columns)
        col->resize(n);
    }

    size_t size() {
      return columns[0]->size();
    }
  };

  struct Styles : public Table {
    std::vector<int>  url;
    std::vector<int>  name;
    Styles(Database &db)
    : Table(db, {&url,&name}) {}

    struct Style {
      Database &db;
      size_t    id;
      Style(Database &db, size_t id) : db(db), id(id) {}
      bool        valid()   const { return id != -1; }

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

    struct Album {
      Database &db;
      size_t id;
      Album(Database &db, size_t id) : db(db), id(id) {}
      bool        valid()   const { return id != -1; }

      const char* url()               const;
      const char* title()             const;
      const char* artist()            const;
      const char* cover_url()         const;
      const char* description()       const;
      const time_t date()             const;
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

    struct Track {
      Database &db;
      size_t id;
      Track(Database &db, size_t id) : db(db), id(id) {}
      bool        valid()   const { return id != -1; }

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
      return store[id];
    }
    GenericIterator<Result, TRecord> begin() {
      return GenericIterator<Result, TRecord>(*this, 0);
    }
    GenericIterator<Result, TRecord> end() {
      return GenericIterator<Result, TRecord>(*this, indices.size());
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

  struct OrderByStyle {
    ColumnID  column;
    SortOrder order;
    OrderByStyle(ColumnID column, SortOrder order) : column(column), order(order) {}
    bool operator()(const Styles::Style &a, const Styles::Style &b) {
      int c;
      switch (column) {
        case STYLE_URL:   c = strcmp(a.url(),      b.url());    break;
        case STYLE_NAME:  c = strcmp(a.name(),     b.name());   break;
        default:          return false;  
      }
      return (order == ASCENDING ? c < 0 : c > 0);
    }
  };

  struct OrderByAlbum {
    ColumnID  column;
    SortOrder order;
    OrderByAlbum(ColumnID column, SortOrder order) : column(column), order(order) {}
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
  };

  struct OrderByTrack {
    ColumnID      column;
    SortOrder     order;
    OrderByAlbum  orderByAlbum;
    OrderByTrack(ColumnID column, SortOrder order)
    : column(column)
    , order(order)
    , orderByAlbum(column, order) {}
    bool operator()(const Tracks::Track &a, const Tracks::Track &b) {
      int c;
      switch (column) {
        case TRACK_URL:    c = strcmp(a.url(),      b.url());    break;
        case TRACK_TITLE:  c = strcmp(a.title(),    b.title());  break;
        case TRACK_ARTIST: c = strcmp(a.artist(),   b.artist()); break;
        case TRACK_REMIX:  c = strcmp(a.remix(),    b.remix());  break;
        case TRACK_NUMBER: c =        a.number() -  b.number();  break;
        case TRACK_BPM:    c =        a.bpm() -     b.bpm();     break;
        default:           return orderByAlbum(a.album(), b.album()); break;
      }
      return (order == ASCENDING ? c < 0 : c > 0);
    }
  };

#if sdfsdf
  // =========================================================================


  template<typename TRecord>
  struct Result {
    std::vector<int> indices;
    TRecord operator[](size_t id);
    GenericIterator<Result, TRecord> begin();
    GenericIterator<Result, TRecord> end();

    void order_by(ColumnID column, SortOrder order) {
      std::sort(begin(), end(), OrderBy(db, column, order));
    }
  };
  // =========================================================================
#endif

  std::string file;
  StringPool  pool;
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

#define str_assign_2(DEST, STR) \
  if (/*!DEST ||*/ !streq(db.pool.get(DEST), STR)) DEST = db.pool.add(STR)

#define     STYLE Database :: Styles :: Style ::
const char* STYLE url()  const { return db.pool.get(db.styles.url[id]);  }
const char* STYLE name() const { return db.pool.get(db.styles.name[id]); }
void        STYLE url(const char *s)  { str_assign_2(db.styles.url[id], s); }
void        STYLE name(const char *s) { str_assign_2(db.styles.name[id], s); }

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

#define      ALBUM Database :: Albums :: Album ::
const char*  ALBUM url()            const { return db.pool.get(db.albums.url[id]);         }
const char*  ALBUM title()          const { return db.pool.get(db.albums.title[id]);       }
const char*  ALBUM artist()         const { return db.pool.get(db.albums.artist[id]);      }
const char*  ALBUM cover_url()      const { return db.pool.get(db.albums.cover_url[id]);   }
const char*  ALBUM description()    const { return db.pool.get(db.albums.description[id]); }
const time_t ALBUM date()           const { return db.albums.date[id] * 60 * 60 * 24;      }
float        ALBUM rating()         const { return (float) db.albums.rating[id] / 100;     }
int          ALBUM votes()          const { return db.albums.votes[id];                    }
int          ALBUM download_count() const { return db.albums.download_count[id];           }
int          ALBUM styles()         const { return db.albums.styles[id];                   }

void         ALBUM url(const char*s)         { str_assign_2(db.albums.url[id], s);         }
void         ALBUM title(const char*s)       { str_assign_2(db.albums.title[id], s);       }
void         ALBUM artist(const char*s)      { str_assign_2(db.albums.artist[id], s);      }
void         ALBUM cover_url(const char*s)   { str_assign_2(db.albums.cover_url[id], s);   }
void         ALBUM description(const char*s) { str_assign_2(db.albums.description[id], s); }
void         ALBUM date(time_t t)            { db.albums.date[id] = t / 60 / 60 / 24;    }
void         ALBUM rating(float i)           { db.albums.rating[id] = i * 100;           }
void         ALBUM votes(int i)              { db.albums.votes[id] = i;                  }
void         ALBUM download_count(int i)     { db.albums.download_count[id] = i;         }
void         ALBUM styles(int i)             { db.albums.styles[id] = i;                 }

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

#define     TRACK Database :: Tracks :: Track ::
const char* TRACK url()       const { return db.pool.get(db.tracks.url[id]);      }
const char* TRACK title()     const { return db.pool.get(db.tracks.title[id]);    }
const char* TRACK artist()    const { return db.pool.get(db.tracks.artist[id]);   }
const char* TRACK remix()     const { return db.pool.get(db.tracks.remix[id]);    }
int         TRACK number()    const { return db.tracks.number[id];   }
int         TRACK bpm()       const { return db.tracks.bpm[id];      }
int         TRACK album_id()  const { return db.tracks.album_id[id]; }
Database::Albums::Album TRACK album() const { return db.albums[db.tracks.album_id[id]]; }

void        TRACK url(const char*s)     { str_assign_2(db.tracks.url[id],    s); }
void        TRACK title(const char*s)   { str_assign_2(db.tracks.title[id],  s); }
void        TRACK artist(const char*s)  { str_assign_2(db.tracks.artist[id], s); }
void        TRACK remix(const char*s)   { str_assign_2(db.tracks.remix[id],  s); }
void        TRACK number(int i)         { db.tracks.number[id] =           i;  }
void        TRACK bpm(int i)            { db.tracks.bpm[id] =              i;  }
void        TRACK album_id(int i)       { db.tracks.album_id[id] =         i;  }
// ============================================================================

#if TEST_DATABASE
int main () {
  std::vector<int> v = {1,2,3,4,5};

  GenericIterator<std::vector<int>, int> beg(v, 0);
  GenericIterator<std::vector<int>, int> end(v, v.size());

  for (; beg != end; ++beg) {
    std::cout << *beg << std::endl;
  }

  Database db;
  auto style = db.styles.find("foo", true);
  style.name("Foo-Style");
  auto s1 = db.styles.find("bar", true);
  s1.name("Bar-Style");

  for (auto s : db.getStyles()) {
    std::cout << s.url() << ": " << s.name() << std::endl;
  }
}
#endif
