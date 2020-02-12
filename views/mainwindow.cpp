#include "mainwindow.hpp"
#include "../common.hpp"
#include "../config.hpp"

using namespace UI;
using namespace Views;

MainWindow :: MainWindow(Database &db)
: playingInfo(db)
, progressBar()
, tabBar()
, windows()
, splash()
, help()
{
  for (auto w : Config::main_widgets) {
    /**/ if (w == "playinginfo")    add(&playingInfo);
    else if (w == "progressbar")    add(&progressBar);
    else if (w == "tabbar")         add(&tabBar);
    else if (w == "windows")        add(&windows);
    else assert_not_reached();
  }

  for (auto w : Config::tabs_widgets) {
    tabBar.add(w);
    /**/ if (w == "splash")   windows.add(&splash);
    else if (w == "playlist") (void)0; // windows.add(&playlist);
    else if (w == "browser")  (void)0; // windows.add(&browser);
    else if (w == "info")     (void)0; // windows.add(&info);
    else if (w == "help")     windows.add(&help);
    else assert_not_reached();
  }
}

void MainWindow :: layout(Pos pos, Size size) {
  this->pos = pos;
  this->size = size;

  playingInfo.layout(pos, size);
  progressBar.layout(pos, size);
  tabBar.layout(pos, size);

  if (playingInfo.visible)  size.height -= playingInfo.size.height;
  if (progressBar.visible)  size.height -= progressBar.size.height;
  if (tabBar.visible)       size.height -= tabBar.size.height;

  windows.layout(pos, size);
  
  VerticalContainer::layout(pos, size);
}
