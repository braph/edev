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
, trackRenderer(Config::playlist_columns)
, playlist(trackRenderer)
, info()
, help()
, pl(db.getTracks())
{
  for (auto w : Config::main_widgets) {
    /**/ if (w == "playinginfo")    addWidget(&playingInfo);
    else if (w == "progressbar")    addWidget(&progressBar);
    else if (w == "tabbar")         addWidget(&tabBar);
    else if (w == "windows")        addWidget(&windows);
    else assert_not_reached();
  }

  for (auto w : Config::tabs_widgets) {
    /**/ if (w == "splash")   windows.addWidget(&splash);
    else if (w == "playlist") windows.addWidget(&playlist);
    else if (w == "browser")  continue; // windows.addWidget(&browser);
    else if (w == "info")     windows.addWidget(&info);
    else if (w == "help")     windows.addWidget(&help);
    else assert_not_reached();
    tabBar.addTab(w);
  }

  playlist.attachList(&pl);
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
