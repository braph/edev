#ifndef _MAINWINDOW_HPP
#define _MAINWINDOW_HPP

/* This view holds all views of the application */

#include "../database.hpp"
#include "../ui.hpp"
#include "../ui/container.hpp"
#include "splash.hpp"
#include "progressbar.hpp"
#include "playinginfo.hpp"
#include "tabbar.hpp"

namespace Views {
  class MainWindow : public UI::VerticalContainer {
  public:
    Views::Splash         splash;
    Views::PlayingInfo    playingInfo;
    Views::ProgressBar    progressBar;
    Views::TabBar         tabBar;
    UI::StackedContainer  windows;
    MainWindow(Database&);
    void layout(UI::Pos, UI::Size);
  };
}

#endif
