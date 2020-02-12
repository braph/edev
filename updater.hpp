#ifndef _UPDATER_HPP
#define _UPDATER_HPP

#include "database.hpp"
#include "browsepage.hpp"

#include <mutex>
#include <queue>
#include <thread>
#include <curl/curl.h>

class Fetcher {
public:
  Fetcher(int first, int last, int parallel);
 ~Fetcher();
  bool alive;
  std::string* pop();
private:
  CURLM *curl_multi;
  std::mutex lock;
  std::thread thread;
  std::queue<std::string*> results;
  void add_job(int page);
  void work(int first, int last, int parallel);
};

class Updater {
public:
  Updater(Database&);
 ~Updater();
  bool start(int pages); // 0->all, N=>from start, -N=>from end
  bool write_to_database();
private:
  Database &db;
  Fetcher *fetcher;
  void insert_album(const Album&);
  void insert_styles(const std::map<std::string, std::string>&);
  void insert_browsepage(const BrowsePage&);
};

#endif
