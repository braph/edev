#include "downloads.hpp"
#include "common.hpp"
#include <stdexcept>
#include <iostream>

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

int Download :: httpcode() {
  long code = 0;
  curl_easy_getinfo(curl_easy, CURLINFO_RESPONSE_CODE, &code);
  return code;
}

const char* Download :: lastURL() {
  const char *url = "<UNKNOWN URL>";
  curl_easy_getinfo(curl_easy, CURLINFO_EFFECTIVE_URL, &url);
  return url;
}

void Download :: cleanup() {
  if (curl_easy)
    curl_easy_cleanup(curl_easy);
  curl_easy = NULL;
}

/* ============================================================================
 * BufferDownload
 * ==========================================================================*/

static size_t write_buffer_cb(char *data, size_t size, size_t nmemb, void *p) {
  static_cast<std::string*>(p)->append(data, size*nmemb);
  return size*nmemb;
}

BufferDownload :: BufferDownload(const std::string &url)
: Download(url)
{
  setopt(CURLOPT_WRITEFUNCTION, write_buffer_cb);
  setopt(CURLOPT_WRITEDATA, &buffer);
}

std::string& BufferDownload :: getContent() {
  return buffer;
}

/* ============================================================================
 * FileDownload
 * ==========================================================================*/
#if 0
static size_t write_stream_cb(char *data, size_t size, size_t nmemb, void *p) {
  static_cast<std::ofstream*>(p)->write(data, size*nmemb);
  return size*nmemb;
}

FileDownload :: FileDownload(const std::string &url, const std::string &file)
: Download(url), filename(file)
{
  setopt(CURLOPT_WRITEFUNCTION, write_stream_cb);
  setopt(CURLOPT_WRITEDATA, &stream);
}

const std::string& FileDownload :: getFilename() {
  return filename;
}
#endif

/* ============================================================================
 * Downloads
 * ==========================================================================*/

Downloads :: Downloads(int parallel)
: parallel(parallel)
{
  curl_global_init(CURL_GLOBAL_ALL);
  if (! (curl_multi = curl_multi_init()))
    throw std::runtime_error("curl_multi_init()");
  curl_multi_setopt(curl_multi, CURLMOPT_MAXCONNECTS, (long) parallel);
}

Downloads :: ~Downloads() {
  for (auto* dl : queue)
    delete dl;
  curl_multi_cleanup(curl_multi);
  curl_global_cleanup();
}

void Downloads :: addDownload(Download* dl, Priority prio) {
  if (prio == LOW)
    queue.push_back(dl);
  else
    queue.push_front(dl);
}

int Downloads :: work() {
  int running_handles;
  curl_multi_perform(curl_multi, &running_handles);
  while (running_handles < parallel && queue.size()) {
    ++running_handles;
    Download *dl = queue.front();
    queue.pop_front();
    curl_multi_add_handle(curl_multi, dl->curl_easy);
  }

  CURLMsg *msg;
  int msgs_left = -1;
  while ((msg = curl_multi_info_read(curl_multi, &msgs_left))) {
    CURL *curl = msg->easy_handle;
    assert(curl); // This should never be 

    Download *dl;
    curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &dl);
    const char *url = dl->lastURL();

    if (msg->msg == CURLMSG_DONE) {
      if (msg->data.result == CURLE_OK)
        std::cerr << url << ": " << dl->httpcode() << std::endl;
      else
        std::cerr << url << ": " << curl_easy_strerror(msg->data.result) << std::endl;

      if (dl->onFinished)
        dl->onFinished(*dl, msg->data.result);
      curl_multi_remove_handle(curl_multi, curl);
      delete dl;
    }
    else {
      std::cerr << url << ": " << msg->msg << std::endl;
    }
  }

  return running_handles;
}
