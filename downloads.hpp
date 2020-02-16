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
  CURLcode setopt(CURLoption option, T value) {
    return curl_easy_setopt(curl_easy, option, value);
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
  std::string& getContent();
private:
  std::string buffer;
  std::ofstream stream;
};

/* ============================================================================
 * Download to file
 * ==========================================================================*/

class FileDownload : public Download {
public:
  FileDownload(const std::string&, const std::string&);
  const std::string& getFilename();
private:
  std::string filename;
  std::ofstream stream;
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
  int work(struct curl_waitfd*, unsigned int extra_nfds);

private:
  CURLM *curl_multi;
  std::deque<Download*> queue;
  int parallel;
};

#endif

