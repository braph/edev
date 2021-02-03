#ifndef DOWNLOADS_HPP
#define DOWNLOADS_HPP

#include <curl/curl.h>

#include <string>
#include <vector>
#include <cstdio>
#include <functional>
#include <memory>

class Downloads;

/* ============================================================================
 * Download base class
 * ==========================================================================*/

class Download {
public:
  Download(const std::string&);
  virtual ~Download();

  int http_code()             const noexcept;
  const char* effective_url() const noexcept;

  void reset() noexcept {
    curl_easy_reset(curl_easy);
  }

  CURLcode perform() noexcept {
    return curl_easy_perform(curl_easy);
  }

  inline void url(const char* URL) noexcept {
    setopt(CURLOPT_URL, URL);
  }

  template<typename T>
  inline CURLcode setopt(CURLoption option, T value) noexcept {
    return curl_easy_setopt(curl_easy, option, value);
  }

  template<typename T>
  inline CURLcode getinfo(CURLINFO info, T& value) const noexcept {
    return curl_easy_getinfo(curl_easy, info, &value);
  }

protected:
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

protected:
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

protected:
  std::string _filename;
  std::FILE* _fh;
};

/* ============================================================================
 * Download mangager - handle multiple Downloads
 * ==========================================================================*/

class Downloads {
public:
  enum Action { Keep, Remove };
  using onFinished_t = std::function<Action(Download&, CURLcode)>;

private:
  struct DL {
    enum State : char { Waiting, Loading, Finished };

    std::unique_ptr<Download> download;
    onFinished_t onFinished;
    State state;
  };

public:
  Downloads();
 ~Downloads();

  void add_download(Download*, onFinished_t);
  int  work()                        noexcept;
  void parallel(int)                 noexcept;
  int  parallel()              const noexcept { return _parallel;        }
  int  running_downloads()     const noexcept { return _running_handles; }
  int  queued_downloads()      const noexcept { return _queued_handles;  }
  std::vector<DL>& downloads()       noexcept { return _downloads;       }

private:
  CURLM* _curl_multi;
  std::vector<DL> _downloads;
  int _running_handles;
  int _queued_handles;
  int _parallel;
};

#endif
