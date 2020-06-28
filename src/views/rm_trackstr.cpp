#include "../ektoplayer.hpp"

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
        for (size_t id : extract_set_bits(unsigned(f.value.i))) {
          strcat(buf, comma);
          strcat(buf, track.table->db.styles[id].name());
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
