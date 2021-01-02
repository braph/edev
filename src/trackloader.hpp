#ifndef TRACKLOADER_HPP
#define TRACKLOADER_HPP

#include "database.hpp"

#include <lib/downloads.hpp>

#include <string>

class TrackLoader {
public:
  TrackLoader();

  Downloads& downloads() noexcept { return _downloads; }

  std::string get_file_for_track(Database::Tracks::Track, bool force_reload=false);
  void download_album(const Database::Tracks::Track&);

private:
  Downloads _downloads;
};

#endif
