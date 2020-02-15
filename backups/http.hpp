#ifndef _HTTP_HPP
#define _HTTP_HPP

#include <curl/curl.h>

// TODO: Which version shall we use?
#define USE_VERSION_WITH_CURLSH 0

#if USE_VERSION_WITH_CURLSH
class SimpleHTTP {
  private:
    CURLSH *curl_share;
  public:
    SimpleHTTP() {
      curl_share = curl_share_init();
      curl_share_setopt(curl_share, CURLSHOPT_SHARE, CURL_LOCK_DATA_PSL);
      curl_share_setopt(curl_share, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
      curl_share_setopt(curl_share, CURLSHOPT_SHARE, CURL_LOCK_DATA_CONNECT);
      curl_share_setopt(curl_share, CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION);
      curl_share_setopt(curl_share, CURLSHOPT_UNSHARE, CURL_LOCK_DATA_COOKIE);
    }

    ~SimpleHTTP() {
      curl_share_cleanup(curl_share);
    }

    static size_t curl_writefunc(char *data, size_t size, size_t nmemb, void *ss) {
      ((std::string*) ss)->append(data, size*nmemb);
      return size*nmemb;
    }

    std::string get(const std::string &url) {
      CURL *curl;
      std::string buf;

      if (! (curl = curl_easy_init()))
        throw std::runtime_error("curl_easy_init()");

      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_SHARE, curl_share);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, SimpleHTTP::curl_writefunc);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
      curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
      curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, CURL_MAX_READ_SIZE);
      CURLcode res = curl_easy_perform(curl);
      curl_easy_cleanup(curl);

      if (res != CURLE_OK)
        throw std::runtime_error(curl_easy_strerror(res));

      return buf;
    }
};
#else
class SimpleHTTP {
  private:
    CURL *curl;
  public:
    SimpleHTTP() {
      if (! (curl = curl_easy_init()))
        throw std::runtime_error("curl_easy_init()");

      curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  SimpleHTTP::curl_writefunc);
      curl_easy_setopt(curl, CURLOPT_BUFFERSIZE,     CURL_MAX_READ_SIZE);
    }

    ~SimpleHTTP() {
      curl_easy_cleanup(curl);
    }

    static size_t curl_writefunc(char *data, size_t size, size_t nmemb, void *ss) {
      ((std::string*) ss)->append(data, size*nmemb);
      return size*nmemb;
    }

    std::string get(const std::string &url) {
      std::string buf;
      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
      CURLcode res = curl_easy_perform(curl);

      if (res != CURLE_OK)
        throw std::runtime_error(curl_easy_strerror(res));

      return buf;
    }
};
#endif

#endif
