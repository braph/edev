#include "ektoplayer.hpp"

#include <lib/downloads.hpp>

#include <curl/curl.h>

#include <cstring>

namespace {

// https://foo.com/bar/ -> bar
std::string& url_basename(std::string &url) {
  if (url.size()) {
    if (url.back() == '/')
      url.pop_back();

    size_t pos = url.rfind('/');
    if (pos != std::string::npos)
      url.erase(0, pos+1);
  }

  return url;
}

// TODO: templated on size
bool strip_extension(std::string &s, const char* ext) {
  size_t ext_len = std::strlen(ext);
  if (std::string::npos != s.find(ext, s.size() - ext_len)) {
    s.erase(s.size() - ext_len);
    return true;
  }
  return false;
}

std::string& unescape(std::string& url) {
  for (size_t pos = 0; (pos = url.find("%20", pos)) != std::string::npos;)
    url.replace(pos, 3, 1, ' ');
  return url;
}

std::string& escape(std::string& url) {
  for (size_t pos = 0; (pos = url.find(' ', pos)) != std::string::npos;)
    url.replace(pos, 1, "%20");
  return url;
}

} // namespace (anonymous)

namespace Ektoplayer {

std::string& url_shrink(std::string& url, const char* prefix, const char* suffix) {
  (void) prefix; // UNUSED for now
  unescape(url);
  url_basename(url);
  if (suffix)
    strip_extension(url, suffix);
  return url;
}

std::string& url_expand(std::string& url, const char* prefix, const char* suffix) {
  escape(url);
  url = std::string(prefix) + url;
  if (suffix) {
    if (std::string::npos == url.find('.', url.size() - std::strlen(suffix)))
      url.append(suffix);
  }
  return url;
}

} // namespace Ektoplayer

#ifdef TEST_EKTOPLAYER
#include <lib/test.hpp>

using namespace Ektoplayer;

int main() {
  TEST_BEGIN();

  std::cout << "config_dir():  " << Ektoplayer::config_dir()  << std::endl;
  std::cout << "config_file(): " << Ektoplayer::config_file() << std::endl;

  // Basic tests
  for (const char* _ : {
      "https://ektoplazm.com/style/psy-dub/",
      "https://ektoplazm.com/style/psy-dub",
      "psy-dub/",
      "psy-dub"}) {

    std::string url = _;
    url_shrink(url, EKTOPLAZM_STYLE_BASE_URL, NULL);
    assert(url == "psy-dub");
    url_expand(url, EKTOPLAZM_STYLE_BASE_URL, NULL);
    assert(url == "https://ektoplazm.com/style/psy-dub");
  }

  // Test escaping / unescaping + extension stripping
  {
    const char* _ = "https://ektoplazm.com/files/Globular%20-%20Entangled%20Everything%20-%202018%20-%20MP3.zip";
    std::string url = _;
    url_shrink(url, EKTOPLAZM_ARCHIVE_BASE_URL, "MP3.zip");
    assert(url == "Globular - Entangled Everything - 2018 - ");
    url_expand(url, EKTOPLAZM_ARCHIVE_BASE_URL, "MP3.zip");
    assert(url == _);
  }

  // Cover URL:
  // Not all cover URLs end in .jpg ( woodsworthy-call-of-the-ancestors-300x300.png )
  // Test if url_shrink/url_expand do not create faulty urls
  {
    const char* _ = "https://ektoplazm.com/img/woodsworthy-call-of-the-ancestors-300x300.png";
    std::string url = _;
    url_shrink(url, EKTOPLAZM_COVER_BASE_URL, ".jpg");
    assert(url == "woodsworthy-call-of-the-ancestors-300x300.png");
    url_expand(url, EKTOPLAZM_COVER_BASE_URL, ".jpg");
    assert(url == _);
  }

  TEST_END();
}
#endif