#ifndef UPDATER_HPP
#define UPDATER_HPP

#include "browsepage.hpp"
#include <lib/downloads.hpp>

#include <string>
#include <cstdint>

namespace Database { class Database; }

class Updater {
public:
  Updater(Database::Database&)    noexcept;
  void start(int pages = INT_MAX) noexcept;
  Downloads& downloads()          noexcept { return _downloads; }

#ifndef TEST_UPDATER
private:
#endif
  Database::Database& _db;
  Downloads _downloads;
  int _max_pages;

  void fetch_page(int, std::string&&="")     noexcept;
  void insert_album(Album&)                  noexcept;
  void insert_browsepage(BrowsePageParser&)  noexcept;
};

#endif
