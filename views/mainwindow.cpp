#include "mainwindow.hpp"
#include "../common.hpp"
#include "../config.hpp"

using namespace UI;
using namespace Views;

MainWindow :: MainWindow(Actions& actions, Database& db, Mpg123Player& player)
: actions(actions)
, playingInfo(db)
, progressBar()
, tabBar()
, readlineWidget()
, windows()
, splash()
, playlist(actions, *this)
, info(db, player)
, help()
{
  readlineWidget.visible = false;

  for (const auto& w : Config::main_widgets) {
    /**/ if (w == "playinginfo")    addWidget(&playingInfo);
    else if (w == "progressbar")    addWidget(&progressBar);
    else if (w == "tabbar")         addWidget(&tabBar);
    else if (w == "readline")       addWidget(&readlineWidget);
    else if (w == "windows")        addWidget(&windows);
    else assert_not_reached();
  }

  setCurrentIndex(indexOf(&windows));

  for (const auto& w : Config::tabs_widgets) {
    /**/ if (w == "splash")   windows.addWidget(&splash);
    else if (w == "playlist") windows.addWidget(&playlist);
    else if (w == "browser")  windows.addWidget(&playlist); /*TODO*/
    else if (w == "info")     windows.addWidget(&info);
    else if (w == "help")     windows.addWidget(&help);
    else assert_not_reached();
    tabBar.addTab(w);
  }
}

void MainWindow :: readline(const std::string& prompt, ReadlineWidget::onFinishFunction callback) {
  readlineWidget.visible = true;
  readlineWidget.setPrompt(prompt);
  int oldWidget = currentIndex();
  setCurrentIndex(indexOf(&readlineWidget));
  readlineWidget.onFinish = [=](const std::string& line, bool notEOF) {
    callback(line, notEOF);
    readlineWidget.visible = false;
    setCurrentIndex(oldWidget);
    layout(pos, size); // TODO: is this an design error? draw on setC,
    draw();
    noutrefresh();
  };
  layout(pos, size); // TODO: dito ^
  draw();
  noutrefresh();
}

void MainWindow :: layout(Pos pos, Size size) {
  this->pos = pos;
  this->size = size;

  tabBar.layout(pos, size);
  progressBar.layout(pos, size);
  playingInfo.layout(pos, size);
  readlineWidget.layout(pos, size);

  if (tabBar.visible)         size.height -= tabBar.size.height;
  if (progressBar.visible)    size.height -= progressBar.size.height;
  if (playingInfo.visible)    size.height -= playingInfo.size.height;
  if (readlineWidget.visible) size.height -= readlineWidget.size.height;

  windows.layout(pos, size);
  
  VerticalContainer::layout(this->pos, this->size);
}

bool MainWindow :: handleKey(int key) {
  if (! VerticalContainer::handleKey(key))
    actions.call(Bindings::global[key]);
  return true;
}
