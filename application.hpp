#ifndef CONTEXT_HPP
#define CONTEXT_HPP

namespace Database { class Database;   }
namespace Views    { class MainWindow; }
class Mpg123Player;
class TrackLoader;
class Updater;

struct Context {
  Database::Database*  database;
  Mpg123Player*        player;
  Views::MainWindow*   mainwindow;
  TrackLoader*         trackloader;
  Updater*             updater;
};

extern Context ctxt;

#endif
