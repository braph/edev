#ifndef _DOWNLOADS_HPP
#define _DOWNLOADS_HPP

#include <curl/curl.h>

#include <deque>
#include <vector>
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
  void cleanup();
  int httpCode();
  const char* lastURL();

  template<typename T>
  inline CURLcode setopt(CURLoption option, T value) {
    return curl_easy_setopt(curl_easy, option, value);
  }

  template<typename T>
  inline CURLcode getinfo(CURLINFO info, T& value) {
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
  std::string& buffer();
private:
  std::string _buffer;
};

/* ============================================================================
 * Download to file
 * ==========================================================================*/

class FileDownload : public Download {
public:
  FileDownload(const std::string&, const std::string&);
  const std::string& filename();
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
  int work();

private:
  CURLM *curl_multi;
  std::deque<Download*> queue;
  int parallel;
};

#endif

