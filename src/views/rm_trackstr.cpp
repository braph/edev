#ifndef VIEWS_RM_TRACKSTR // TODO ...
#define VIEWS_RM_TRACKSTR

#include "../ektoplayer.hpp"
#include "../database.hpp"

#include <lib/bit_tools.hpp>

static const char* trackField(const Database::Tracks::Track &track, Database::ColumnID id) {
  static char buf[256];

  Database::Field f = track[id];
  switch (f.type) {
    case Database::Field::STRING:
      return f.value.s;

    case Database::Field::INTEGER:
      if (Database::AlbumColumnID(id) == Database::ALBUM_STYLES) {
        buf[0] = '\0';
        const char* comma = "";
        for (auto id : extract_set_bits(f.value.i)) {
          std::strcat(buf, comma);
          std::strcat(buf, track.table->db.styles[size_t(id)].name());
          comma = "|";
        }
      }
      else {
        std::sprintf(buf, "%02d", f.value.i);
      }

      return buf;

    case Database::Field::FLOAT:
      std::sprintf(buf, "%02.2f", f.value.f);
      return buf;

    case Database::Field::TIME:
      return REPORT_BUG;

    default:
      return REPORT_BUG;
  }
}

#endif
