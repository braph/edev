#ifndef _UPDATER_HPP
#define _UPDATER_HPP

#include "database.hpp"
#include "browsepage.hpp"
#include "downloads.hpp"

class Updater {
public:
  Updater(Database&, Downloads&);
  bool start(int pages); // 0->all, N=>from start, -N=>from end
private:
  Database &db;
  Downloads &downloads;
  void insert_album(Album&);
  void insert_browsepage(BrowsePage&);
};

#endif
