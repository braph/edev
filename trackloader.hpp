#ifndef TRACKLOADER_HPP
#define TRACKLOADER_HPP

class Downloads;
#include "database.hpp" // TODO: get rid of this!

#include <string>

class TrackLoader {
public:
  std::string getFileForTrack(Database::Tracks::Track, bool force_reload=false);
//void downloadAlbum(const Database::Tracks::Track&);
  TrackLoader(Downloads&);
private:
  Downloads& downloads;
};

#endif
