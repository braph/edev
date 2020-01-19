//#include events
#include <string>
#include <curl/curl.h>
#include <cstdio>

class Download {
  public:
    //events
    std::string url;
    std::string filename;
    std::string error;
  private:
    CURL *curl;
    FILE *fp;
    unsigned int bytes_read;
    unsigned int total;
    unsigned int tries;

  public:
    Download(std::string _url, std::string _filename)
    //events: :completed, :failed
    : url(_url) // TODO: uri-parse
    , filename(_filename)
    , error("")
    , bytes_read(0)
    , total(0)
    , tries(3)
    { }

    void start() {
      pthread_t thr;
      pthread_create(&thr, NULL, do_download_thread_func, this);
    }

    static size_t curl_write_callback_static(char *data, size_t size, size_t nmemb, void *download_instance) {
      Download *d = (Download*) download_instance;
      return d->curl_write_callback(data, size, nmemb);
    }

    size_t curl_write_callback(char *data, size_t size, size_t nmemb) {
      bytes_read += size * nmemb;
      size_t written = fwrite(data, size, nmemb, fp);
      // @total = res.header['Content-Length'].to_i
      return written;
    }

    static void* do_download_thread_func(void *download_instance) {
      Download *d = (Download*) download_instance;
      for (unsigned int i = 0; i < d->tries; ++i)
        if (d->do_download()) {
          // trigger event :completed
          return NULL;
        }
        // TODO: on error sleep 3
      // trigger event :failed
      return NULL;
    }

    /*
       def progress
       (@bytes_read.to_f / @total * 100) rescue 0).clamp(0, 100).to_f
       end
       */

    bool do_download() {
      printf("do download\n");
      CURLcode res;
      bool ret = false;

      bytes_read = 0;
      total = 0;
      error = "";

      if (! (curl = curl_easy_init()))
        goto END_CURL_EASY;

      if (! (fp = fopen(filename.c_str(), "wb"))) {
        goto END_FOPEN; // TODO ERROR
      }

      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Download::curl_write_callback_static);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
      curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
      curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, CURL_MAX_READ_SIZE);

      res = curl_easy_perform(curl);
      //fail res.body unless res.code == '200'

      if (res != CURLE_OK) {
        error = curl_easy_strerror(res);
        goto END_FOPEN;
      }

      // fail 'filesize mismatch' if @bytes_read != @total

END_NORMAL:
      ret = true;
END_FOPEN:
      fclose(fp);
END_CURL_EASY:
      curl_easy_cleanup(curl);

      printf("returning %u\n", ret);
      return ret;
    }
};

#include <unistd.h>

int main() {
  Download d("https://ektoplazm.com/audio/octogoat-change-the-world.mp3", "/tmp/penis.mp3");
  d.start();
  pause();
}
