#ifndef UPDATER_HPP
#define UPDATER_HPP

class Album;
class BrowsePage;
class Downloads;
namespace Database { class Database; }

class Updater {
public:
  Updater(Database::Database&, Downloads&);
  bool start(int pages); // 0->all, N=>from start, -N=>from end

#ifndef TEST_UPDATER
private:
#endif
  Database::Database& db;
  Downloads& downloads;
  void insert_album(Album&);
  void insert_browsepage(BrowsePage&);
};

#endif
