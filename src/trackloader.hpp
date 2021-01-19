#ifndef TRACKLOADER_HPP
#define TRACKLOADER_HPP

#include "database.hpp"

#include <lib/downloads.hpp>

#include <string>

class TrackLoader {
public:
  TrackLoader() {}

  std::string get_file_for_track(Database::Tracks::Track, bool force_reload=false); /* throws */
  void download_album(const Database::Tracks::Track&); /* throws */
  Downloads& downloads() noexcept { return _downloads; }

private:
  Downloads _downloads;
};

#endif
