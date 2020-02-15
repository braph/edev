#ifndef _EKTOPLAYER_HPP
#define _EKTOPLAYER_HPP

#include "filesystem.hpp"

#include <string>

#define VERSION                    "0.0.1"
#define GITHUB_URL                 "https://github.com/braph/ektoplayer"
#define EKTOPLAZM_URL              "https://ektoplazm.com"
#define EKTOPLAZM_BROWSE_URL       EKTOPLAZM_URL "/section/free-music"
#define EKTOPLAZM_ALBUM_BASE_URL   EKTOPLAZM_URL "/free-music/"
#define EKTOPLAZM_COVER_BASE_URL   EKTOPLAZM_URL "/img/"
#define EKTOPLAZM_TRACK_BASE_URL   EKTOPLAZM_URL "/audio/"
#define EKTOPLAZM_STYLE_BASE_URL   EKTOPLAZM_URL "/style/"
#define EKTOPLAZM_ARCHIVE_BASE_URL EKTOPLAZM_URL "/files/"

// Count of online data (last updated: February 2020)
#define EKTOPLAZM_STYLE_COUNT      25
#define EKTOPLAZM_ALBUM_COUNT      2078
#define EKTOPLAZM_TRACK_COUNT      14402

namespace Ektoplayer {

static inline std::string config_dir() {
  return Filesystem::home() + PATH_SEP ".config" PATH_SEP "ektoplayer";
}

static inline std::string config_file() {
  return config_dir() + PATH_SEP "ektoplayer.rc";
}

namespace URL {

std::string style(std::string);
std::string style_pack(std::string);

std::string track(std::string);
std::string track_pack(std::string);

std::string album(std::string);
std::string album_pack(std::string);

std::string cover(std::string);
std::string cover_pack(std::string);

std::string archive_mp3(std::string);
std::string archive_mp3_pack(std::string);

std::string archive_wav(std::string);
std::string archive_wav_pack(std::string);

std::string archive_flac(std::string);
std::string archive_flac_pack(std::string);

static inline std::string browse_url(int page = 1) {
  if (page == 1)
    return EKTOPLAZM_BROWSE_URL;
  else
    return EKTOPLAZM_BROWSE_URL "/page/" + std::to_string(page);
}

} // namespace URL
} // namespace Ektoplayer

#endif
