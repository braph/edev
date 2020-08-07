#ifndef DOWNLOADS_HPP
#define DOWNLOADS_HPP

#include <curl/curl.h>

#include <string>
#include <vector>
#include <cstdio>
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
  int http_code() const noexcept;
  const char* last_url() const noexcept;

  template<typename T>
  inline CURLcode setopt(CURLoption option, T value) noexcept {
    return curl_easy_setopt(curl_easy, option, value);
  }

  template<typename T>
  inline CURLcode getinfo(CURLINFO info, T& value) const noexcept {
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
  FileDownload(const std::string&, std::string);
 ~FileDownload();

  const std::string& filename() const noexcept { return _filename; }

private:
  std::string _filename;
  std::FILE* _fh;
};

/* ============================================================================
 * Download mangager - handle multiple Downloads
 * ==========================================================================*/

class Downloads {
public:
  Downloads();
 ~Downloads();

  void addDownload(Download*);
  int work() noexcept;

  void parallel(int)               noexcept;
  int  parallel()            const noexcept { return _parallel;                }
  std::vector<Download*>& queue()  noexcept { return _queue;                   }
  size_t running_downloads() const noexcept { return size_t(_running_handles); }
  size_t queued_downloads()  const noexcept { return _queue.size();            }

private:
  CURLM* _curl_multi;
  std::vector<Download*> _queue;
  int _parallel;
  int _running_handles;
};

#endif

