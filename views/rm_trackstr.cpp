
static const char* trackField(Database::Tracks::Track track, int id) {
  static char buf[32];
  Database::Field f = track[(Database::ColumnID) id];
  switch (f.type) {
  case Database::Field::STRING:  return f.value.s;
  case Database::Field::INTEGER: sprintf(buf, "%d", f.value.i); return buf;
  case Database::Field::FLOAT:   sprintf(buf, "%f", f.value.f); return buf;
  default:                       return "REPORT A BUG";
  }
}
