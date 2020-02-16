
static const char* trackField(const Database::Tracks::Track &track, Database::ColumnID id) {
  static char buf[32];
  Database::Field f = track[id];
  switch (f.type) {
  case Database::Field::STRING:  return f.value.s;
  case Database::Field::INTEGER: sprintf(buf, "%d", f.value.i); return buf;
  case Database::Field::FLOAT:   sprintf(buf, "%f", f.value.f); return buf;
  case Database::Field::TIME:    return "TIME...";
  default:                       return "REPORT A BUG";
  }
}
