#ifndef EKTOPLAYER_HPP
#define EKTOPLAYER_HPP

#include "../third_party/pprintpp/include/pprintpp/pprintpp.hpp"

#include <lib/filesystem.hpp>

#include <string>
#include <cstdio>
#include <cstdarg>
#include <exception>

#define VERSION                     "0.0.0"
#define GITHUB_URL                  "https://github.com/braph/ektoplayer"
#define EKTOPLAZM_URL               "https://ektoplazm.com"
#define EKTOPLAZM_BROWSE_URL        EKTOPLAZM_URL "/section/free-music"
#define EKTOPLAZM_ALBUM_BASE_URL    EKTOPLAZM_URL "/free-music/"
#define EKTOPLAZM_COVER_BASE_URL    EKTOPLAZM_URL "/img/"
#define EKTOPLAZM_TRACK_BASE_URL    EKTOPLAZM_URL "/audio/"
#define EKTOPLAZM_STYLE_BASE_URL    EKTOPLAZM_URL "/style/"
#define EKTOPLAZM_ARCHIVE_BASE_URL  EKTOPLAZM_URL "/files/"
#define EKTOPLAZM_TEMP_FILE_PREFIX  "~ekto~-"
#define REPORT_BUG                  "REPORT A BUG, PLEASE!"
#define TEST_DB                     "/tmp/ektoplayer-test.db"

// Count of online data (last updated: February 2020)
#define EKTOPLAZM_STYLE_COUNT       30
#define EKTOPLAZM_ALBUM_COUNT       2090
#define EKTOPLAZM_TRACK_COUNT       14410
#define EKTOPLAZM_META_SIZE         286650    // average lenth: 14
#define EKTOPLAZM_DESC_SIZE         1468250   // average lenth: 706
#define EKTOPLAZM_STYLE_URL_SIZE    250       // average lenth: 8
#define EKTOPLAZM_ALBUM_URL_SIZE    49780     // average lenth: 24
#define EKTOPLAZM_TRACK_URL_SIZE    391020    // average lenth: 27
#define EKTOPLAZM_COVER_URL_SIZE    67670     // average lenth: 32
#define EKTOPLAZM_ARCHIVE_URL_SIZE  77070     // average lenth: 37

namespace Ektoplayer {

/// Return ektoplayer's configuration directory
static inline Filesystem::path config_dir() {
  return Filesystem::path(Filesystem::home()) / ".config" / "ektoplayer";
}

/// Return ektoplayer's configuration file
static inline Filesystem::path config_file() {
  return config_dir() / "ektoplayer.rc";
}

/// Return an ektoplazm browse url
static inline std::string browse_url(int page = 1) {
  char buf[sizeof(EKTOPLAZM_BROWSE_URL "/page/") + 10];
  std::sprintf(buf, EKTOPLAZM_BROWSE_URL "/page/%d", page);

  if (page <= 1)
    buf[sizeof(EKTOPLAZM_BROWSE_URL) - 1] = '\0';

  return buf;
}

/**
 * Shrink a ektoplazm URL
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

/**
 * Unshrink a previously shrinked ektoplazm URL
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

namespace pprintpp {
template<> const char* to_s<const std::string&>       (const std::string& s)        { return s.c_str(); }
template<> const char* to_s<const Filesystem::path&>  (const Filesystem::path& s)   { return s.c_str(); }
template<> const char* to_s<const std::exception&>    (const std::exception& e)     { return e.what();  }
}

template<class strprov, typename ... Ts>
void logwrite_helper(const Ts& ... args) {
  using paramtypes = decltype(pprintpp::tie_types(pprintpp::to_s(args)...));
  using af = pprintpp::autoformat_t<strprov, paramtypes>;
  fprintf(stderr, af::str(), pprintpp::to_s(args)...);
}

#define log_write(...) AUTOFORMAT(logwrite_helper, __VA_ARGS__);

#endif
