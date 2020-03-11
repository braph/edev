#ifndef _UPDATER_HPP
#define _UPDATER_HPP

class Database;
class Downloads;
class BrowsePage;
class Album;

class Updater {
public:
  Updater(Database&, Downloads&);
  bool start(int pages); // 0->all, N=>from start, -N=>from end

#ifndef TEST_UPDATER
private:
#endif
  Database &db;
  Downloads &downloads;
  void insert_album(Album&);
  void insert_browsepage(BrowsePage&);
};

#endif
