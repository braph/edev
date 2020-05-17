#ifndef CONTEXT_HPP
#define CONTEXT_HPP

namespace Database { class Database;   }
namespace Views    { class MainWindow; }
class Mpg123Player;
class TrackLoader;

struct Context {
  Database::Database*  database;
  Mpg123Player*        player;
  Views::MainWindow*   mainwindow;
  TrackLoader*         trackloader;
};

extern Context ctxt;

#endif
