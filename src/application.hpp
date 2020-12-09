#ifndef APPLICATION_HPP
#define APPLICATION_HPP

namespace Database { class Database;   }
namespace Views    { class MainWindow; }
class Mpg123Player;
class TrackLoader;
class Updater;

extern Database::Database   database;
extern Mpg123Player         player;
extern Updater              updater;
extern TrackLoader          trackloader;
extern Views::MainWindow*   mainwindow;

#endif
