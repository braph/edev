#include "../packedvector.hpp"

static const char* trackField(const Database::Tracks::Track &track, Database::ColumnID id) {
  static char buf[128];

  Database::Field f = track[id];
  switch (f.type) {
    case Database::Field::STRING:
      return f.value.s;

    case Database::Field::INTEGER:
      if (Database::AlbumColumnID(id) == Database::ALBUM_STYLES) {
        buf[0] = '\0';
        const char* comma = "";
        Database::StylesArray styleIDs(unsigned(f.value.i));
        for (auto id : styleIDs)
          if (id) {
            strcat(buf, comma);
            strcat(buf, track.db.styles[id].name());
            comma = "|";
          }
      }
      else {
        sprintf(buf, "%02d", f.value.i);
      }

      return buf;

    case Database::Field::FLOAT:
      sprintf(buf, "%02.2f", f.value.f);
      return buf;

    case Database::Field::TIME:
      return REPORT_BUG;

    default:     
      return REPORT_BUG;
  }
}
