#include "downloads.hpp"
#include "common.hpp"
#include <stdexcept>

/* ============================================================================
 * Download
 * ==========================================================================*/

Download :: Download(const std::string &url) {
  if (! (curl_easy = curl_easy_init()))
    throw std::runtime_error("curl_easy_init()");

  setopt(CURLOPT_URL, url.c_str());
  setopt(CURLOPT_PRIVATE, this);
  setopt(CURLOPT_FOLLOWLOCATION, 1);
  setopt(CURLOPT_TIMEOUT, 60);
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

static size_t write_stream_cb(char *data, size_t size, size_t nmemb, void *stream) {
  static_cast<std::ofstream*>(stream)->write(data, std::streamsize(size*nmemb));
  return size*nmemb;
}

FileDownload :: FileDownload(const std::string &url, const std::string &file)
: Download(url), _filename(file)
{
  _stream.exceptions(std::ofstream::failbit|std::ofstream::badbit);
  _stream.open(file, std::ios::binary);
  setopt(CURLOPT_WRITEFUNCTION, write_stream_cb);
  setopt(CURLOPT_WRITEDATA, &_stream);
}

/* ============================================================================
 * Downloads
 * ==========================================================================*/

Downloads :: Downloads(int parallel)
: _parallel(parallel), _running_handles(0)
{
  curl_global_init(CURL_GLOBAL_ALL);
  if (! (_curl_multi = curl_multi_init()))
    throw std::runtime_error("curl_multi_init()");
  curl_multi_setopt(_curl_multi, CURLMOPT_MAXCONNECTS, long(parallel));
}

Downloads :: ~Downloads() {
  for (auto* dl : _queue)
    delete dl;
  curl_multi_cleanup(_curl_multi);
  curl_global_cleanup();
}

void Downloads :: addDownload(Download* dl, Priority priority) {
  if (priority == LOW)
    _queue.push_back(dl);
  else
    _queue.push_front(dl);
}

int Downloads :: work() noexcept {
  curl_multi_perform(_curl_multi, &_running_handles);
  while (_running_handles < _parallel && _queue.size()) {
    ++_running_handles;
    Download *dl = _queue.front();
    _queue.pop_front();
    curl_multi_add_handle(_curl_multi, dl->curl_easy);
  }

  CURLMsg *msg;
  int msgs_left;
  while ((msg = curl_multi_info_read(_curl_multi, &msgs_left))) {
    CURL *curl_easy = msg->easy_handle;
    assert(curl_easy);

    Download *dl;
    curl_easy_getinfo(curl_easy, CURLINFO_PRIVATE, &dl);
    const char *url = dl->lastURL();

    if (msg->msg == CURLMSG_DONE) {
      if (dl->onFinished)
        dl->onFinished(*dl, msg->data.result);
      curl_multi_remove_handle(_curl_multi, curl_easy);
      delete dl;
    }
    else assert_not_reached();
  }

  int ready_filedescriptors = 0;
  curl_multi_wait(_curl_multi, NULL, 0, 10, &ready_filedescriptors);
  return ready_filedescriptors;
}
