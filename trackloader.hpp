#ifndef _TRACKLOADER_HPP
#define _TRACKLOADER_HPP

#include "config.hpp"
#include "database.hpp"
#include "downloads.hpp"
#include "ektoplayer.hpp"

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>

#include <string>

class TrackLoader {
public:
  std::string getFileForTrack(Database::Tracks::Track, bool);
//void downloadAlbum(const Database::Tracks::Track&);
  TrackLoader(Downloads&);
private:
  Downloads& downloads;
};

#endif
