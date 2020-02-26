#include "../packedvector.hpp"

static const char* trackField(const Database::Tracks::Track &track, Database::ColumnID id) {
  static char buf[128];
  char* s = buf;

  Database::Field f = track[id];
  switch (f.type) {
    case Database::Field::STRING:
      return f.value.s;

    case Database::Field::INTEGER:
      if (static_cast<Database::AlbumColumnID>(id) == Database::ALBUM_STYLES) {
        *s = '\0';
        TinyPackedArray<uint8_t, uint32_t> styleIDs(f.value.i);
        for (auto id : styleIDs) {
          if (! id)
            break;
          strcat(s, ", ");
          strcat(s, track.db.styles[id].name());
        }

        return buf + 2;
      }

      sprintf(buf, "%d", f.value.i);
      return buf;

    case Database::Field::FLOAT:
      sprintf(buf, "%f", f.value.f);
      return buf;

    case Database::Field::TIME:
      return "TIME...";

    default:     
      return REPORT_BUG;
  }
}
