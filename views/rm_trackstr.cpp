

// TODO: Fix this fucking shit asshoel
static const char* trackField(Database::Tracks::Track track, int id) {
  static char buf[32];
  Database::Field f = track[(Database::TrackColumnID) id];
  switch (f.type) {
  case Database::Field::STRING:  return f.value.s;
  case Database::Field::INTEGER: sprintf(buf, "%d", f.value.i); return buf;
  case Database::Field::FLOAT:   sprintf(buf, "%f", f.value.f); return buf;
  default:                       return "REPORT A BUG";
  }
}

#if 0
static const char* trackField(Database::Tracks::Track track, Database::ColumnID id) {
#define SPRINTF(FMT, ...) (sprintf(buf, FMT, __VA_ARGS__), buf)
#define DB Database
  static char buf[32];
  switch (id) {
    case DB::STYLE_NAME:            return "TODO";
    case DB::ALBUM_TITLE:           return track.album().title();
    case DB::ALBUM_ARTIST:          return track.album().artist();
    case DB::ALBUM_DESCRIPTION:     return track.album().description();
    case DB::ALBUM_DATE:            return "TODO";
    case DB::ALBUM_RATING:          return SPRINTF("%f", track.album().rating());
    case DB::ALBUM_VOTES:           return SPRINTF("%d", track.album().votes());
    case DB::ALBUM_DOWNLOAD_COUNT:  return SPRINTF("%d", track.album().download_count());
    //case TRACK_NUMBER: // TODO
    //case TRACK_NUMBER: // TODO
    //case TRACK_NUMBER: // TODO
    case DB::TRACK_TITLE:           return track.title();
    case DB::TRACK_ARTIST:          return track.artist();
    case DB::TRACK_REMIX:           return track.remix();
    case DB::TRACK_NUMBER:          return SPRINTF("%d", track.number());
    case DB::TRACK_BPM:             return SPRINTF("%d", track.bpm());
  }
}
#endif

