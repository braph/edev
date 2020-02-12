#ifndef _EKTOPLAYER_HPP
#define _EKTOPLAYER_HPP

#include "filesystem.hpp"

#include <string>

#define VERSION                    "0.0.1"
#define GITHUB_URL                 "https://github.com/braph/ektoplayer"
#define EKTOPLAZM_URL              "https://ektoplazm.com"
#define EKTOPLAZM_ALBUM_BASE_URL   EKTOPLAZM_URL "/free-music/"
#define EKTOPLAZM_COVER_BASE_URL   EKTOPLAZM_URL "/img/"
#define EKTOPLAZM_TRACK_BASE_URL   EKTOPLAZM_URL "/audio/"
#define EKTOPLAZM_STYLE_BASE_URL   EKTOPLAZM_URL "/style/"
#define EKTOPLAZM_ARCHIVE_BASE_URL EKTOPLAZM_URL "/files/"
#define EKTOPLAZM_BROWSE_BASE_URL  EKTOPLAZM_URL "/section/free-music"

namespace Ektoplayer {

static inline std::string album_url(const std::string &album) {
  return std::string(EKTOPLAZM_ALBUM_BASE_URL) + album;
}

static inline std::string cover_url(const std::string &cover) {
  return std::string(EKTOPLAZM_COVER_BASE_URL) + cover;
}

static inline std::string track_url(const std::string &track) {
  return std::string(EKTOPLAZM_TRACK_BASE_URL) + track;
}

static inline std::string style_url(const std::string &style) {
  return std::string(EKTOPLAZM_STYLE_BASE_URL) + style;
}

static inline std::string archive_url(const std::string &archive) {
  return std::string(EKTOPLAZM_ARCHIVE_BASE_URL) + archive;
}

static inline std::string config_dir() {
  return Filesystem::home() + PATH_SEP ".config" PATH_SEP "ektoplayer";
}

static inline std::string config_file() {
  return config_dir() + PATH_SEP "ektoplayer.rc";
}

} // namespace Ektoplayer

#endif
