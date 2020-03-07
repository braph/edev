#ifndef _DOWNLOADS_HPP
#define _DOWNLOADS_HPP

#include <curl/curl.h>

#include <deque>
#include <string>
#include <fstream>
#include <functional>

class Downloads;

/* ============================================================================
 * Download base class
 * ==========================================================================*/

class Download {
public:
  Download(const std::string&);
  virtual ~Download();

  std::function<void(Download&, CURLcode)> onFinished;

  CURLcode perform();
  void cleanup() noexcept;
  int httpCode() const noexcept;
  const char* lastURL() const noexcept;

  template<typename T>
  CURLcode setopt(CURLoption option, T value) noexcept {
    return curl_easy_setopt(curl_easy, option, value);
  }

  template<typename T>
  CURLcode getinfo(CURLINFO info, T& value) const noexcept {
    return curl_easy_getinfo(curl_easy, info, &value);
  }

private:
  friend class Downloads;
  CURL *curl_easy;
};

/* ============================================================================
 * Download to string buffer
 * ==========================================================================*/

class BufferDownload : public Download {
public:
  BufferDownload(const std::string&);

  std::string& buffer() noexcept { return _buffer; }

private:
  std::string _buffer;
};

/* ============================================================================
 * Download to file
 * ==========================================================================*/

class FileDownload : public Download {
public:
  FileDownload(const std::string&, const std::string&);

  const std::string& filename() const noexcept { return _filename; }

private:
  std::string _filename;
  std::ofstream _stream;
};

/* ============================================================================
 * Download mangager - handle multiple Downloads
 * ==========================================================================*/

class Downloads {
public:
  enum Priority { LOW, HIGH };

  Downloads(int);
 ~Downloads();
  void addDownload(Download*, Priority);
  int work() noexcept;

  std::deque<Download*>& queue() noexcept { return _queue; }
  int runningHandles() const noexcept     { return _running_handles; }

private:
  CURLM* _curl_multi;
  std::deque<Download*> _queue;
  int _parallel;
  int _running_handles;
};

#endif

