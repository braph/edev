#ifndef APPLICATION_HPP
#define APPLICATION_HPP

namespace Database { class Database;   }
namespace Views    { class MainWindow; }
class Mpg123Player;
class TrackLoader;
class Updater;

struct Context {
  Database::Database*  database;
  Mpg123Player*        player;
  TrackLoader*         trackloader;
  Updater*             updater;
  Views::MainWindow*   mainwindow;
};

extern Context ctxt;

#endif
