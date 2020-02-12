#include "database.hpp"
#include <cstring>
#include <cstdlib>
#define  streq(a,b) (!strcmp(a,b))
typedef const char* ccstr;

// TODO: find(const char *url, bool create): create is not respected!

/* ============================================================================
 * Dumper / Loader
 * ============================================================================
 * Helpers for saving and reading buffers to/from disk.
 * Binary format is:
 *   size_t elem_size  : Size of one element (in bits!!!)
 *   size_t elem_count : Element count
 *   char   data[]     : Buffer data, length = bytes_for(elem_size, elem_count)
 *   size_t elem_size  : Size of one element  -. 
 *   size_t elem_count : Element count        -'-> Used for validation
 */

static inline size_t bytes_for(size_t bits, size_t count) {
  bits *= count;
  return (bits % 8 ? bits/8 + 1 : bits/8);
}

struct Saver {
  static void write(std::ofstream &fs, const char* buf, size_t elem_size, size_t elem_count) {
    fs.write((char*) &elem_size,  sizeof(elem_size));
    fs.write((char*) &elem_count, sizeof(elem_count));
    if (elem_count)
      fs.write(buf, bytes_for(elem_size, elem_count));
    fs.write((char*) &elem_size,  sizeof(elem_size));
    fs.write((char*) &elem_count, sizeof(elem_count));
  }
};

struct Loader {
  size_t elem_size;
  size_t elem_count;
  void readHeader(std::ifstream &fs) {
    elem_size = elem_count = 0;
    fs.read((char*) &elem_size,  sizeof(elem_size));
    fs.read((char*) &elem_count, sizeof(elem_count));
  }
  void readData(std::ifstream &fs, char *buf) {
    if (elem_count)
      fs.read(buf, bytes_for(elem_size, elem_count));
    size_t elem_size_check  = 0;
    size_t elem_count_check = 0;
    fs.read((char*) &elem_size_check,   sizeof(elem_size_check));
    fs.read((char*) &elem_count_check,  sizeof(elem_count_check));
    if (elem_size != elem_size_check || elem_count != elem_count_check)
      throw std::runtime_error("Column size check failed");
  }
};

/* ============================================================================
 * A bit ugly, but cleans up the code a lot
 * ==========================================================================*/
#define STR_SET(POOL_NAME, ID, S) \
  if (! streq(db.pool_##POOL_NAME.get(ID), S)) \
    { ID = db.pool_##POOL_NAME.add(S); }

#define STR_GET(POOL_NAME, ID) \
  db.pool_##POOL_NAME.get(ID)

/* ============================================================================
 * Database
 * ==========================================================================*/

void Database :: load(const std::string &file) {
  std::ifstream fs;
  fs.exceptions(std::ifstream::failbit|std::ifstream::badbit);
  fs.open(file, std::ios::binary);

  Loader l;
  for (auto p : pools) {
    l.readHeader(fs);
    p->reserve(l.elem_count);
    l.readData(fs, p->data());
  }

  styles.load(fs);
  albums.load(fs);
  tracks.load(fs);
}

void Database :: save(const std::string &file) {
  std::ofstream fs;
  fs.exceptions(std::ofstream::failbit|std::ofstream::badbit);
  fs.open(file, std::ios::binary);

  for (auto p : pools)
    Saver::write(fs, p->data(), 8, p->size());
  styles.save(fs);
  albums.save(fs);
  tracks.save(fs);
}

/* ============================================================================
 * Database :: Table
 * ==========================================================================*/

void Database :: Table :: load(std::ifstream &fs) {
  Loader l;
  for (auto *col: columns) {
    l.readHeader(fs);
    col->resize(l.elem_count);
    l.readData(fs, (char*) col->data());
  }
}

void Database :: Table :: save(std::ofstream &fs) {
  for (auto *col : columns)
    Saver::write(fs, (char*) col->data(), 8*sizeof(int), col->size());
}

// XXX: description
template<typename TRecord, typename TTable>
static TRecord find_or_create(TTable &table, StringPool &pool, const char *url) {
  bool newly_inserted = false;
  size_t strId = pool.add(url, &newly_inserted);
  Database::Column::iterator beg;
  Database::Column::iterator end;
  Database::Column::iterator fnd;
  if (newly_inserted)
    goto NOT_FOUND;
  beg = table.url.begin();
  end = table.url.end();
  fnd = std::find(beg, end, strId);
  if (fnd == end) {
NOT_FOUND:
    size_t pos = table.size();
    table.resize(pos+1);
    table.url[pos] = strId;
    return TRecord(table.db, pos);
  } else {
    return TRecord(table.db, fnd-beg);
  }
}

/* ============================================================================
 * Database :: Styles
 * ==========================================================================*/

Database::Styles::Style Database :: Styles :: find(const char *url, bool create) {
  return find_or_create<Database::Styles::Style>(*this, db.pool_style_url, url);
}

#define STYLE Database :: Styles :: Style
ccstr   STYLE :: url()  const   { return STR_GET(style_url, db.styles.url[id]);  }
ccstr   STYLE :: name() const   { return STR_GET(meta,      db.styles.name[id]); }

void    STYLE :: url(ccstr s)   { STR_SET(style_url, db.styles.url[id],  s);  }
void    STYLE :: name(ccstr s)  { STR_SET(meta,      db.styles.name[id], s);  }

/* ============================================================================
 * Database :: Albums
 * ==========================================================================*/

Database::Albums::Album Database::Albums::find(const char *url, bool create) {
  return find_or_create<Database::Albums::Album>(*this, db.pool_album_url, url);
}

#define ALBUM Database :: Albums :: Album
ccstr  ALBUM::url()            const { return STR_GET(album_url,  db.albums.url[id]);         }
ccstr  ALBUM::title()          const { return STR_GET(meta,       db.albums.title[id]);       }
ccstr  ALBUM::artist()         const { return STR_GET(meta,       db.albums.artist[id]);      }
ccstr  ALBUM::cover_url()      const { return STR_GET(cover_url,  db.albums.cover_url[id]);   }
ccstr  ALBUM::description()    const { return STR_GET(desc,       db.albums.description[id]); }
ccstr  ALBUM::archive_mp3_url() const { return STR_GET(mp3_url,   db.albums.archive_mp3[id]); }
ccstr  ALBUM::archive_wav_url() const { return STR_GET(wav_url,   db.albums.archive_wav[id]); }
ccstr  ALBUM::archive_flac_url() const { return STR_GET(flac_url, db.albums.archive_flac[id]); }
time_t ALBUM::date()           const { return db.albums.date[id] * 60 * 60 * 24;      }
float  ALBUM::rating()         const { return (float) db.albums.rating[id] / 100;     }
int    ALBUM::votes()          const { return db.albums.votes[id];                    }
int    ALBUM::download_count() const { return db.albums.download_count[id];           }
int    ALBUM::styles()         const { return db.albums.styles[id];                   }

void   ALBUM::url(ccstr s)          { STR_SET(album_url,    db.albums.url[id],         s); }
void   ALBUM::title(ccstr s)        { STR_SET(meta,         db.albums.title[id],       s); }
void   ALBUM::artist(ccstr s)       { STR_SET(meta,         db.albums.artist[id],      s); }
void   ALBUM::cover_url(ccstr s)    { STR_SET(cover_url,    db.albums.cover_url[id],   s); }
void   ALBUM::description(ccstr s)  { STR_SET(desc,         db.albums.description[id], s); }
void   ALBUM::archive_mp3_url(ccstr s) { STR_SET(mp3_url,   db.albums.archive_mp3[id], s); }
void   ALBUM::archive_wav_url(ccstr s) { STR_SET(wav_url,   db.albums.archive_wav[id], s); }
void   ALBUM::archive_flac_url(ccstr s) { STR_SET(flac_url, db.albums.archive_flac[id], s); }
void   ALBUM::date(time_t t)        { db.albums.date[id] = t / 60 / 60 / 24;    }
void   ALBUM::rating(float i)       { db.albums.rating[id] = i * 100;           }
void   ALBUM::votes(int i)          { db.albums.votes[id] = i;                  }
void   ALBUM::download_count(int i) { db.albums.download_count[id] = i;         }
void   ALBUM::styles(int i)         { db.albums.styles[id] = i;                 }

/* ============================================================================
 * Database :: Tracks
 * ==========================================================================*/

Database::Tracks::Track Database::Tracks::find(const char* url, bool create) {
  return find_or_create<Database::Tracks::Track>(*this, db.pool_track_url, url);
}

#define TRACK Database :: Tracks :: Track
ccstr TRACK :: url()      const { return STR_GET(track_url, db.tracks.url[id]);      }
ccstr TRACK :: title()    const { return STR_GET(meta,      db.tracks.title[id]);    }
ccstr TRACK :: artist()   const { return STR_GET(meta,      db.tracks.artist[id]);   }
ccstr TRACK :: remix()    const { return STR_GET(meta,      db.tracks.remix[id]);    }
int   TRACK :: number()   const { return db.tracks.number[id];                }
int   TRACK :: bpm()      const { return db.tracks.bpm[id];                   }
int   TRACK :: album_id() const { return db.tracks.album_id[id];              }
ALBUM TRACK :: album()    const { return db.albums[db.tracks.album_id[id]];   }

void  TRACK :: url(ccstr s)     { STR_SET(track_url, db.tracks.url[id],    s); }
void  TRACK :: title(ccstr s)   { STR_SET(meta,      db.tracks.title[id],  s); }
void  TRACK :: artist(ccstr s)  { STR_SET(meta,      db.tracks.artist[id], s); }
void  TRACK :: remix(ccstr s)   { STR_SET(meta,      db.tracks.remix[id],  s); }
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
