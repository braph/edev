#ifndef APPLICATION_HPP
#define APPLICATION_HPP

namespace Database { class Database;   }
namespace Views    { class MainWindow; }
class Mpg123Playback;
class TrackLoader;
class Updater;

extern Database::Database   database;
extern Mpg123Playback       player;
extern Updater              updater;
extern TrackLoader          trackloader;
extern Views::MainWindow*   mainwindow;

#endif
