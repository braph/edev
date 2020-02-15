#include "ektoplayer.hpp"

#include <curl/curl.h>

#include <cstring>

// TODO:
// Not all images end with .jpg: woodsworthy-call-of-the-ancestors-300x300.png
// Make the call simpler.

// https://foo.com/bar/ -> bar
static std::string& url_basename(std::string &url) {
  if (url.size()) {
    if (url.back() == '/')
      url.pop_back();

    size_t pos = url.rfind('/');
    if (pos != std::string::npos)
      url.erase(0, pos+1);
  }

  return url;
}

static std::string& strip_extension(std::string &s, const char* ext) {
  size_t ext_len = std::strlen(ext);
  if (std::string::npos != s.find(ext, s.size() - ext_len))
    s.erase(s.size() - ext_len);
  return s;
}

static std::string& unescape(std::string& url) {
  for (size_t pos; (pos = url.find("%20")) != std::string::npos;)
    url.replace(pos, 3, " ");
  return url;
}

static std::string& escape(std::string& url) {
  for (size_t pos; (pos = url.find(' ')) != std::string::npos;)
    url.replace(pos, 1, "%20");
  return url;
}

namespace Ektoplayer { namespace URL {

std::string track(std::string track) {
  return std::string(EKTOPLAZM_TRACK_BASE_URL) + escape(track) + ".mp3";
}

std::string track_pack(std::string url) {
  return strip_extension(url_basename(unescape(url)), ".mp3");
}

std::string style(std::string style) {
  return std::string(EKTOPLAZM_STYLE_BASE_URL) + escape(style);
}

std::string style_pack(std::string url) {
  return url_basename(unescape(url));
}

std::string album(std::string album) {
  return std::string(EKTOPLAZM_ALBUM_BASE_URL) + escape(album);
}

std::string album_pack(std::string url) {
  return url_basename(unescape(url));
}

std::string cover(std::string cover) {
  return std::string(EKTOPLAZM_COVER_BASE_URL) + escape(cover) + ".jpg";
}

std::string cover_pack(std::string url) {
  return strip_extension(url_basename(unescape(url)), ".jpg");
}

// === Archives ===============================================================
std::string archive_mp3(std::string packed) {
  return std::string(EKTOPLAZM_ARCHIVE_BASE_URL) + escape(packed) + "MP3.zip";
}

std::string archive_mp3_pack(std::string url) {
  return strip_extension(url_basename(unescape(url)), "MP3.zip");
}

std::string archive_wav(std::string packed) {
  return std::string(EKTOPLAZM_ARCHIVE_BASE_URL) + escape(packed) + "WAV.rar";
}

std::string archive_wav_pack(std::string url) {
  return strip_extension(url_basename(unescape(url)), "WAV.rar");
}

std::string archive_flac(std::string packed) {
  return std::string(EKTOPLAZM_ARCHIVE_BASE_URL) + escape(packed) + "FLAC.zip";
}

std::string archive_flac_pack(std::string url) {
  return strip_extension(url_basename(unescape(url)), "FLAC.zip");
}

}} // namespace URL, namespace Ektoplayer

#if TEST_EKTOPLAYER
#include "test.hpp"
#include <functional>
using namespace Ektoplayer::URL;

void test(
    std::string url,
    std::function<std::string(std::string)> packer,
    std::string packed_expect,
    std::function<std::string(std::string)> unpacker,
    std::string unpacked_expect) {

  std::string packed = packer(url);
  if (packed != packed_expect)
    throw std::runtime_error(std::string("packed != packed_expect: ") +
        packed + " != " + packed_expect);

  std::string unpacked = unpacker(packed);
  if (unpacked != unpacked_expect)
    throw std::runtime_error(std::string("unpacked != unpacked_expect: ") +
        unpacked + " != " + unpacked_expect);
}

int main() {
  TEST_BEGIN();

  std::cout
    << "config_dir():  " << Ektoplayer::config_dir()  << std::endl
    << "config_file(): " << Ektoplayer::config_file() << std::endl;

  // Style URLs
  for (const char* _ : {
      "https://ektoplazm.com/style/psy-dub/",
      "https://ektoplazm.com/style/psy-dub",
      "psy-dub/",
      "psy-dub"}) {
    test(_, style_pack, "psy-dub", style, "https://ektoplazm.com/style/psy-dub");
  }

  // Track URLs
  {
    const char* _ = "https://ektoplazm.com/audio/aerodromme-crop-circle.mp3";
    test(_, track_pack, "aerodromme-crop-circle", track, _);
  }

  // Albums URLs
  {
    const char* _ = "https://ektoplazm.com/free-music/globular-entangled-everything";
    test(_, album_pack, "globular-entangled-everything", album, _);
  }

  // Cover URLs
  {
    const char* _ = "https://ektoplazm.com/img/globular-entangled-everything-300x300.jpg";
    test(_, cover_pack, "globular-entangled-everything-300x300", cover, _);
  }

  // Archive URLs
  {
    const char* _ = "https://ektoplazm.com/files/Globular%20-%20Entangled%20Everything%20-%202018%20-%20MP3.zip";
    test(_, archive_mp3_pack, "Globular - Entangled Everything - 2018 - ", archive_mp3, _);
  }

  {
    const char* _ = "https://ektoplazm.com/files/Globular%20-%20Entangled%20Everything%20-%202018%20-%20WAV.rar";
    test(_, archive_wav_pack, "Globular - Entangled Everything - 2018 - ", archive_wav, _);
  }

  {
    const char* _ = "https://ektoplazm.com/files/Globular%20-%20Entangled%20Everything%20-%202018%20-%20FLAC.zip";
    test(_, archive_flac_pack, "Globular - Entangled Everything - 2018 - ", archive_flac, _);
  }

  TEST_END();
}
#endif
