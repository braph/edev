#include "database.hpp"
#include <cstring>
#include <cstdlib>
#define  streq(a,b) (!strcmp(a,b))
typedef const char* ccstr;

/* ============================================================================
 * Dumper / Loader
 * ============================================================================
 * Helpers for saving and reading buffers to/from disk.
 * Binary format is:
 *   size_t elem_size  : Size of one element (in bits!)
 *   size_t elem_count : Element count
 *   char   data       : Buffer data, length = bytes_for(elem_size, elem_count)
 *   size_t elem_size  : Size of one element  -. 
 *   size_t elem_count : Element count        -'-> Used for validation
 */

static inline size_t bytes_for(size_t bits, size_t count) {
  bits *= count;
  return (bits % 8 ? bits/8 + 1 : bits/8);
}

struct Saver {
  static bool write(std::ofstream &fs, const char* buf, size_t elem_size, size_t elem_count) {
    fs.write((char*) &elem_size,  sizeof(elem_size));
    fs.write((char*) &elem_count, sizeof(elem_count));
    if (elem_count)
      fs.write(buf, bytes_for(elem_size, elem_count));
    fs.write((char*) &elem_size,  sizeof(elem_size));
    fs.write((char*) &elem_count, sizeof(elem_count));
    return fs.good();
  }
};

struct Loader {
  size_t elem_size;
  size_t elem_count;
  bool readHeader(std::ifstream &fs) {
    elem_size = elem_count = 0;
    fs.read((char*) &elem_size,  sizeof(elem_size));
    fs.read((char*) &elem_count, sizeof(elem_count));
    return fs.good();
  }
  bool readData(std::ifstream &fs, char *buf) {
    if (elem_count)
      fs.read(buf, bytes_for(elem_size, elem_count));
    size_t elem_size_check  = 0;
    size_t elem_count_check = 0;
    fs.read((char*) &elem_size_check,   sizeof(elem_size_check));
    fs.read((char*) &elem_count_check,  sizeof(elem_count_check));
    return (elem_size == elem_size_check && elem_count == elem_count_check);
  }
};

/* ============================================================================
 * Some macros (TODO: remove them?)
 * ==========================================================================*/
#define str_assign(DEST, STR) \
  if (! streq(db.pool.get(DEST), STR)) DEST = db.pool.add(STR)

#define str_assign_url(DEST, STR) \
  if (! streq(db.pool_url.get(DEST), STR)) DEST = db.pool_url.add(STR)

#define str_assign_desc(DEST, STR) \
  if (! streq(db.pool_desc.get(DEST), STR)) DEST = db.pool_desc.add(STR)

/* ============================================================================
 * Database
 * ==========================================================================*/

bool Database :: load() {
  std::ifstream fs(file);
  if (fs.good()) {
    Loader l;
    for (auto p : {&pool, &pool_url, &pool_desc}) {
      l.readHeader(fs);
      p->reserve(l.elem_count);
      if (! l.readData(fs, p->data()))
        return false;
    }

    return
      styles.load(fs) &&
      albums.load(fs) &&
      tracks.load(fs);
  }
  return false;
}

bool Database :: save() {
  std::ofstream fs(file);
  if (fs.good()) {
    for (auto p : {&pool, &pool_url, &pool_desc})
      if (! Saver::write(fs, p->data(), 8, p->size()))
        return false;
    return
      styles.save(fs) &&
      albums.save(fs) &&
      tracks.save(fs);
  }
  return false;
}

/* ============================================================================
 * Database :: Table
 * ==========================================================================*/

bool Database :: Table :: load(std::ifstream &fs) {
  Loader l;
  for (auto col: columns) {
    if (! l.readHeader(fs))
      return false;
    col->resize(l.elem_count);
    if (! l.readData(fs, (char*) col->data()))
      return false;
  }
  return true;
}

bool Database :: Table :: save(std::ofstream &fs) {
  for (auto col : columns)
    if (! Saver::write(fs, (char*) col->data(), 8*sizeof(int), col->size()))
      return false;
  return true;
}


/* ============================================================================
 * Database :: Styles
 * ==========================================================================*/

Database::Styles::Style Database::Styles::operator[](size_t id) {
  return Database::Styles::Style(db, id); // XXX huh.
}

Database::Styles::Style Database :: Styles :: find(const char *url, bool create) {
#if 0 // XXX this version, db.pool.add() + std::find() may be faster
  auto beg = std::begin(this->url);
  auto end = std::end(this->url);
  auto fnd = std::find(beg, end, db.pool.add(url));
  size_t pos;
  if (fnd == end) {
    pos = this->size();
    this->resize(pos+1);
    auto r = Database::Styles::Style(db, pos);
    r.url(url);
    return r;
  } else {
    return Database::Styles::Style(db, fnd-beg);
  }
#else
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
#endif
}

#define STYLE Database :: Styles :: Style
ccstr   STYLE :: url()  const   { return db.pool_url.get(db.styles.url[id]);  }
ccstr   STYLE :: name() const   { return db.pool.get(db.styles.name[id]);     }

void    STYLE :: url(ccstr s)   { str_assign_url(db.styles.url[id], s);       }
void    STYLE :: name(ccstr s)  { str_assign(db.styles.name[id], s);          }

/* ============================================================================
 * Database :: Albums
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
ccstr  ALBUM::url()            const { return db.pool_url.get(db.albums.url[id]);     }
ccstr  ALBUM::title()          const { return db.pool.get(db.albums.title[id]);       }
ccstr  ALBUM::artist()         const { return db.pool.get(db.albums.artist[id]);      }
ccstr  ALBUM::cover_url()      const { return db.pool.get(db.albums.cover_url[id]);   }
ccstr  ALBUM::description()    const { return db.pool_desc.get(db.albums.description[id]); }
time_t ALBUM::date()           const { return db.albums.date[id] * 60 * 60 * 24;      }
float  ALBUM::rating()         const { return (float) db.albums.rating[id] / 100;     }
int    ALBUM::votes()          const { return db.albums.votes[id];                    }
int    ALBUM::download_count() const { return db.albums.download_count[id];           }
int    ALBUM::styles()         const { return db.albums.styles[id];                   }

void   ALBUM::url(ccstr s)          { str_assign_url(db.albums.url[id], s);     }
void   ALBUM::title(ccstr s)        { str_assign(db.albums.title[id], s);       }
void   ALBUM::artist(ccstr s)       { str_assign(db.albums.artist[id], s);      }
void   ALBUM::cover_url(ccstr s)    { str_assign(db.albums.cover_url[id], s);   }
void   ALBUM::description(ccstr s)  { str_assign_desc(db.albums.description[id], s); }
void   ALBUM::date(time_t t)        { db.albums.date[id] = t / 60 / 60 / 24;    }
void   ALBUM::rating(float i)       { db.albums.rating[id] = i * 100;           }
void   ALBUM::votes(int i)          { db.albums.votes[id] = i;                  }
void   ALBUM::download_count(int i) { db.albums.download_count[id] = i;         }
void   ALBUM::styles(int i)         { db.albums.styles[id] = i;                 }

/* ============================================================================
 * Database :: Tracks
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
ccstr TRACK :: url()      const { return db.pool_url.get(db.tracks.url[id]);  }
ccstr TRACK :: title()    const { return db.pool.get(db.tracks.title[id]);    }
ccstr TRACK :: artist()   const { return db.pool.get(db.tracks.artist[id]);   }
ccstr TRACK :: remix()    const { return db.pool.get(db.tracks.remix[id]);    }
int   TRACK :: number()   const { return db.tracks.number[id];                }
int   TRACK :: bpm()      const { return db.tracks.bpm[id];                   }
int   TRACK :: album_id() const { return db.tracks.album_id[id];              }
ALBUM TRACK :: album()    const { return db.albums[db.tracks.album_id[id]];   }

void  TRACK :: url(ccstr s)     { str_assign_url(db.tracks.url[id],    s);    }
void  TRACK :: title(ccstr s)   { str_assign(db.tracks.title[id],  s);        }
void  TRACK :: artist(ccstr s)  { str_assign(db.tracks.artist[id], s);        }
void  TRACK :: remix(ccstr s)   { str_assign(db.tracks.remix[id],  s);        }
void  TRACK :: number(int i)    { db.tracks.number[id] =           i;         }
void  TRACK :: bpm(int i)       { db.tracks.bpm[id] =              i;         }
void  TRACK :: album_id(int i)  { db.tracks.album_id[id] =         i;         }

/* ============================================================================
 * Database :: OrderBy
 * ==========================================================================*/

int Database :: OrderBy :: compare(const Styles::Style &a, const Styles::Style &b) {
  switch (column) {
    case STYLE_URL:   return strcmp(a.url(),  b.url());
    case STYLE_NAME:  return strcmp(a.name(), b.name());
    default:          return false;  
  }
}

int Database :: OrderBy :: compare(const Albums::Album &a, const Albums::Album &b) {
  switch (column) {
    case ALBUM_URL:             return strcmp(a.url(),          b.url());
    case ALBUM_TITLE:           return strcmp(a.title(),        b.title());
    case ALBUM_ARTIST:          return strcmp(a.artist(),       b.artist());
    case ALBUM_COVER_URL:       return strcmp(a.cover_url(),    b.cover_url());
    case ALBUM_DESCRIPTION:     return strcmp(a.description(),  b.description());
    case ALBUM_DATE:            return a.date() -           b.date();
    case ALBUM_RATING:          return a.rating() -         b.rating();
    case ALBUM_VOTES:           return a.votes() -          b.votes();
    case ALBUM_DOWNLOAD_COUNT:  return a.download_count() - b.download_count();
    default:                    return false;
  }
}

int Database :: OrderBy :: compare(const Tracks::Track &a, const Tracks::Track &b) {
  switch (column) {
    case TRACK_URL:    return strcmp(a.url(),      b.url());
    case TRACK_TITLE:  return strcmp(a.title(),    b.title());
    case TRACK_ARTIST: return strcmp(a.artist(),   b.artist());
    case TRACK_REMIX:  return strcmp(a.remix(),    b.remix());
    case TRACK_NUMBER: return        a.number() -  b.number();
    case TRACK_BPM:    return        a.bpm() -     b.bpm();
    default:           return compare(a.album(), b.album());
  }
}

/* ============================================================================
 * Database :: Where
 * ==========================================================================*/

int Database :: Where :: compare(const Tracks::Track &t) {
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

int Database :: Where :: compare(const Albums::Album &a) {
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

int Database :: Where :: compare(const Styles::Style &s) {
  switch (column) {
    case STYLE_URL:     return strcmp(s.url(),    strValue);
    case STYLE_NAME:    return strcmp(s.name(),   strValue);
    default:            std::abort();
  }
}


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
