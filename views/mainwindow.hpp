#ifndef _MAINWINDOW_HPP
#define _MAINWINDOW_HPP

/* This view holds all views of the application */

#include "splash.hpp"
#include "help.hpp"
#include "info.hpp"
#include "progressbar.hpp"
#include "playinginfo.hpp"
#include "playlist.hpp"
#include "tabbar.hpp"

#include "../player.hpp"
#include "../database.hpp"
#include "../ui.hpp"
#include "../ui/container.hpp"

namespace Views {

class MainWindow : public UI::VerticalContainer {
public:
  Views::PlayingInfo    playingInfo;
  Views::ProgressBar    progressBar;
  Views::TabBar         tabBar;
  UI::StackedContainer  windows;
  Views::Splash         splash;
  Views::Playlist       playlist;
  Views::Info           info;
  Views::Help           help;
  MainWindow(Database&, Mpg123Player&);
  void layout(UI::Pos, UI::Size);
};

} // namespace Views

#endif
