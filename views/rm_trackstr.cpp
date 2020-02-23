
static const char* trackField(const Database::Tracks::Track &track, Database::ColumnID id) {
  static char buf[128];
  int v;
  char* s = buf;

  Database::Field f = track[id];
  switch (f.type) {
  case Database::Field::STRING:  return f.value.s;
  case Database::Field::INTEGER:
                                 if ((Database::AlbumColumnID) id == Database::ALBUM_STYLES) {
                                   *s = '\0';
                                   v = f.value.i;
                                   while (v) {
                                     strcat(s, track.db.styles[v & 0xFF].name());
                                     v >>= 8;
                                     if (v)
                                       strcat(s, ", ");
                                   }

                                   return buf;
                                 }

                                 sprintf(buf, "%d", f.value.i); return buf;
  case Database::Field::FLOAT:   sprintf(buf, "%f", f.value.f); return buf;
  case Database::Field::TIME:    return "TIME...";
  default:                       return REPORT_BUG;
  }
}
