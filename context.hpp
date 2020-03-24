#ifndef CONTEXT_HPP
#define CONTEXT_HPP

namespace Database { class Database; }
namespace Views { class MainWindow; }
class Mpg123Player;
class TrackLoader;
class Downloads;

struct Context {
  Database::Database*
    database;
  Mpg123Player*
    player;
  Views::MainWindow*
    mainwindow;
  TrackLoader*
    trackloader;
  Downloads*
    downloads;
};

#endif
