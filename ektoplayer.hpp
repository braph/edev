#ifndef EKTOPLAYER_HPP
#define EKTOPLAYER_HPP

#include "filesystem.hpp"

#include <string>

#define VERSION                    "0.0.0"
#define GITHUB_URL                 "https://github.com/braph/ektoplayer"
#define EKTOPLAZM_URL              "https://ektoplazm.com"
#define EKTOPLAZM_TEMP_FILE_PREFIX "~ekto-"
#define EKTOPLAZM_BROWSE_URL       EKTOPLAZM_URL "/section/free-music"
#define EKTOPLAZM_ALBUM_BASE_URL   EKTOPLAZM_URL "/free-music/"
#define EKTOPLAZM_COVER_BASE_URL   EKTOPLAZM_URL "/img/"
#define EKTOPLAZM_TRACK_BASE_URL   EKTOPLAZM_URL "/audio/"
#define EKTOPLAZM_STYLE_BASE_URL   EKTOPLAZM_URL "/style/"
#define EKTOPLAZM_ARCHIVE_BASE_URL EKTOPLAZM_URL "/files/"

// Count of online data (last updated: February 2020)
#define EKTOPLAZM_STYLE_COUNT 29
#define EKTOPLAZM_ALBUM_COUNT 2079
#define EKTOPLAZM_TRACK_COUNT 14403
#define EKTOPLAZM_META_SIZE 286645 // average lenth: 14
#define EKTOPLAZM_DESC_SIZE 1468245 // average lenth: 706
#define EKTOPLAZM_STYLE_URL_SIZE 245 // average lenth: 8
#define EKTOPLAZM_ALBUM_URL_SIZE 49777 // average lenth: 24
#define EKTOPLAZM_TRACK_URL_SIZE 391013 // average lenth: 27
#define EKTOPLAZM_COVER_URL_SIZE 67666 // average lenth: 32
#define EKTOPLAZM_ARCHIVE_URL_SIZE 77069 // average lenth: 37

namespace Ektoplayer {

static inline Filesystem::path config_dir() {
  return Filesystem::path(Filesystem::home()) / ".config" / "ektoplayer";
}

static inline Filesystem::path config_file() {
  return config_dir() / "ektoplayer.rc";
}

static inline std::string browse_url(int page = 1) {
  if (page == 1)
    return EKTOPLAZM_BROWSE_URL;
  else
    return EKTOPLAZM_BROWSE_URL "/page/" + std::to_string(page);
}

/* Shrink a ektoplazm URL
 * - Unescape it
 * - Remove prefix and suffix
 *
 * Example:
 *  url_shrink(
 *    "https://ektoplazm.com/files/Obri%20-%20Afterglow%20-%202018%20-%20FLAC.zip",
 *    EKTOPLAZM_ARCHIVE_BASE_URL, "FLAC.zip");
 *
 *  "Obri - Afterglow - 2018 - "
 */
std::string& url_shrink(std::string&, const char*, const char* suffix = NULL);

/* Unshrink a previously shrinked ektoplazm URL
 * - Add prefix and suffix
 * - Escape it
 *
 * Example:
 *  url_expand("Obri - Afterglow - 2018 - ", EKTOPLAZM_ARCHIVE_BASE_URL, "FLAC.zip");
 *
 *  "https://ektoplazm.com/files/Obri%20-%20Afterglow%20-%202018%20-%20FLAC.zip"
 */
std::string& url_expand(std::string&, const char*, const char* suffix = NULL);

} // namespace Ektoplayer

#endif
