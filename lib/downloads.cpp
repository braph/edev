#include "downloads.hpp"
#include <climits>
#include <cstring>
#include <stdexcept>

/* ============================================================================
 * Download
 * ==========================================================================*/

Download :: Download(const std::string &url_) {
  if ((curl_easy = curl_easy_init())) {
    url(url_.c_str());
    setopt(CURLOPT_PRIVATE, this);
    setopt(CURLOPT_FOLLOWLOCATION, 1);
    return;
  }
#ifdef __cpp_exceptions
  throw std::runtime_error("curl_easy_init()");
#endif
}

Download :: ~Download() {
  cleanup();
}

int Download :: http_code() const noexcept {
  long code = 0;
  getinfo(CURLINFO_RESPONSE_CODE, code);
  return code;
}

const char* Download :: effective_url() const noexcept {
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
  static_cast<std::string*>(buffer)->append(data, size * nmemb);
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
    throw std::runtime_error(strerror(errno));
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
  : _running_handles(0)
  , _queued_handles(0)
  , _parallel(INT_MAX)
{
  curl_global_init(CURL_GLOBAL_ALL);
  if ((_curl_multi = curl_multi_init()))
    return;

#ifdef __cpp_exceptions
  throw std::runtime_error("curl_multi_init()");
#endif
}

Downloads :: ~Downloads() {
  curl_multi_cleanup(_curl_multi);
  curl_global_cleanup();
}

void Downloads :: parallel(int parallel) noexcept {
  _parallel = parallel;
  curl_multi_setopt(_curl_multi, CURLMOPT_MAXCONNECTS, long(parallel));
}

void Downloads :: add_download(Download* dl, onFinished_t cb) {
  _downloads.push_back(DL{std::unique_ptr<Download>(dl), cb, DL::State::Waiting});
  _queued_handles++;
}

int Downloads :: work() noexcept {
  curl_multi_perform(_curl_multi, &_running_handles);

  for (auto& dl : _downloads) {
    if (_queued_handles && _running_handles < _parallel) {
      if (dl.state == DL::State::Waiting) {
        if (CURLM_OK == curl_multi_add_handle(_curl_multi, dl.download->curl_easy)) {
          dl.state = DL::State::Loading;
          ++_running_handles;
          --_queued_handles;
        }
      }
    }
    else break;
  }

  CURLMsg *msg;
  int msgs_left;
  while ((msg = curl_multi_info_read(_curl_multi, &msgs_left))) {
    if (msg->msg == CURLMSG_DONE) {
      CURL *curl_easy = msg->easy_handle;
      curl_multi_remove_handle(_curl_multi, curl_easy);

      for (auto it = _downloads.begin(), end = _downloads.end(); it != end; ++it) {
        if (curl_easy == it->download->curl_easy) {
          it->state = DL::Finished;
          Action action = Action::Remove;

          if (it->onFinished)
            action = it->onFinished(*(it->download), msg->data.result);

          if (action == Action::Remove)
            _downloads.erase(it);

          break;
        }
      }
    }
  }

  int ready_filedescriptors;
  curl_multi_wait(_curl_multi, NULL, 0, 0, &ready_filedescriptors);
  return ready_filedescriptors;
}
