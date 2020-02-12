#ifndef _MAINWINDOW_HPP
#define _MAINWINDOW_HPP

/* This view holds all views of the application */

#include "splash.hpp"
#include "help.hpp"
#include "progressbar.hpp"
#include "playinginfo.hpp"
#include "tabbar.hpp"

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
  Views::Help           help;
  MainWindow(Database&);
  void layout(UI::Pos, UI::Size);
};

} // namespace Views

#endif
