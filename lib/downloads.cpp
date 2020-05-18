#include "downloads.hpp"
#include <cstdlib>
#include <climits>
#include <cstring>
#include <stdexcept>

/* ============================================================================
 * Download
 * ==========================================================================*/

Download :: Download(const std::string &url) {
  if ((curl_easy = curl_easy_init())) {
    setopt(CURLOPT_URL, url.c_str());
    setopt(CURLOPT_PRIVATE, this);
    setopt(CURLOPT_FOLLOWLOCATION, 1);
    setopt(CURLOPT_TIMEOUT, 60);
    return;
  }
#ifdef __cpp_exceptions
  throw std::runtime_error("curl_easy_init()");
#endif
}

Download :: ~Download() {
  cleanup();
}

CURLcode Download :: perform() {
  CURLcode e = curl_easy_perform(curl_easy);
  if (onFinished)
    onFinished(*this, e);
  return e;
}

int Download :: httpCode() const noexcept {
  long code = 0;
  getinfo(CURLINFO_RESPONSE_CODE, code);
  return code;
}

const char* Download :: lastURL() const noexcept {
  char *url = NULL;
  if (CURLE_OK != getinfo(CURLINFO_EFFECTIVE_URL, url))
    return "<UNKNOWN URL>";
  return url;
}

void Download :: cleanup() noexcept {
  if (curl_easy)
    curl_easy_cleanup(curl_easy);
  curl_easy = NULL;
}

/* ============================================================================
 * BufferDownload
 * ==========================================================================*/

static size_t write_buffer_cb(char *data, size_t size, size_t nmemb, void *buffer) {
  static_cast<std::string*>(buffer)->append(data, size*nmemb);
  return size*nmemb;
}

BufferDownload :: BufferDownload(const std::string &url)
: Download(url)
{
  setopt(CURLOPT_WRITEFUNCTION, write_buffer_cb);
  setopt(CURLOPT_WRITEDATA, &_buffer);
}

/* ============================================================================
 * FileDownload
 * ==========================================================================*/

FileDownload :: FileDownload(const std::string &url, std::string file)
: Download(url), _filename(std::move(file)), _fh(fopen(_filename.c_str(), "w"))
{
  if (! _fh) {
#ifdef __cpp_exceptions
    throw std::runtime_error(strerror(errno))
#endif
  }
  setopt(CURLOPT_WRITEDATA, _fh);
}

FileDownload :: ~FileDownload() {
  if (_fh)
    fclose(_fh);
}

/* ============================================================================
 * Downloads
 * ==========================================================================*/

Downloads :: Downloads()
: _parallel(INT_MAX), _running_handles(0)
{
  curl_global_init(CURL_GLOBAL_ALL);
  if ((_curl_multi = curl_multi_init()))
    return;

#ifdef __cpp_exceptions
  throw std::runtime_error("curl_multi_init()");
#endif
}

Downloads :: ~Downloads() {
  for (auto* dl : _queue)
    delete dl;
  curl_multi_cleanup(_curl_multi);
  curl_global_cleanup();
}

void Downloads :: parallel(int parallel) noexcept {
  _parallel = parallel;
  curl_multi_setopt(_curl_multi, CURLMOPT_MAXCONNECTS, long(parallel));
}

void Downloads :: addDownload(Download* dl) {
  _queue.push_back(dl);
}

int Downloads :: work() noexcept {
  curl_multi_perform(_curl_multi, &_running_handles);
  while (_running_handles < _parallel && _queue.size()) {
    ++_running_handles;
    Download *dl = _queue.back();
    _queue.pop_back();
    curl_multi_add_handle(_curl_multi, dl->curl_easy);
  }

  CURLMsg *msg;
  int msgs_left;
  while ((msg = curl_multi_info_read(_curl_multi, &msgs_left))) {
    CURL *curl_easy = msg->easy_handle;
    Download *dl;
    curl_easy_getinfo(curl_easy, CURLINFO_PRIVATE, &dl);
    const char *url = dl->lastURL();

    if (msg->msg == CURLMSG_DONE) {
      if (dl->onFinished)
        dl->onFinished(*dl, msg->data.result);
      curl_multi_remove_handle(_curl_multi, curl_easy);
      delete dl;
    }
#ifndef NDEBUG
    else abort();
#endif
  }

  int ready_filedescriptors;
  curl_multi_wait(_curl_multi, NULL, 0, 0, &ready_filedescriptors);
  return ready_filedescriptors;
}
