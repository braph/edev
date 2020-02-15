#ifndef _DOWNLOADS_HPP
#define _DOWNLOADS_HPP

#include <curl/curl.h>

#include <deque>
#include <vector>
#include <string>
#include <fstream>
#include <functional>

class Downloads;

class Download {
public:
  Download(const std::string&);
  virtual ~Download();
  std::function<void(Download&, CURLcode)> onFinished;
  CURLcode perform();
  void cleanup();
  int httpcode();
  const char* lastURL();

  template<typename T>
  CURLcode setopt(CURLoption option, T value) {
    return curl_easy_setopt(curl_easy, option, value);
  }
private:
  friend class Downloads;
  CURL *curl_easy;
};

class BufferDownload : public Download {
public:
  BufferDownload(const std::string&);
  std::string& getContent();
private:
  std::string buffer;
};

#if 0
class FileDownload : public Download {
public:
  FileDownload(const std::string&, const std::string&);
 ~FileDownload();
  const std::string& getFilename();
private:
  std::string filename;
  std::ofstream stream;
}
#endif

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

