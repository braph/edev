#include "mainwindow.hpp"
#include "../bindings.hpp"
#include "../config.hpp"

using namespace UI;
using namespace Views;

MainWindow :: MainWindow()
: infoLine()
, progressBar()
, tabBar()
, readlineWidget()
, windows()
, splash()
, playlist()
, info()
, help()
{
  readlineWidget.visible = false;

  for (auto w : Config::main_widgets) {
    switch (w) {
    case MainWidgets::NONE:                                       break;
    case MainWidgets::INFOLINE:     addWidget(&infoLine);         break;
    case MainWidgets::PROGRESSBAR:  addWidget(&progressBar);      break;
    case MainWidgets::TABBAR:       addWidget(&tabBar);           break;
    case MainWidgets::READLINE:     addWidget(&readlineWidget);   break;
    case MainWidgets::WINDOWS:      addWidget(&windows);          break;
    }
  }

  currentWidget(&windows);

  for (auto w : Config::tabs_widgets) {
    switch (w) {
    case TabWidgets::NONE:
      break;
    case TabWidgets::SPLASH:
      windows.addWidget(&splash);
      tabBar.addTab("splash");
      break;
    case TabWidgets::PLAYLIST:
      windows.addWidget(&playlist);
      tabBar.addTab("playlist");
      break;
    case TabWidgets::BROWSER:
      windows.addWidget(&playlist); // TODO
      tabBar.addTab("browser");
      break;
    case TabWidgets::INFO:
      windows.addWidget(&info);
      tabBar.addTab("info");
      break;
    case TabWidgets::HELP:
      windows.addWidget(&help);
      tabBar.addTab("help");
      break;
    }
  }
}

void MainWindow :: readline(std::string prompt, ReadlineWidget::onFinishFunction callback) {
  readlineWidget.visible = true;
  readlineWidget.setPrompt(std::move(prompt));
  int oldWidget = currentIndex();
  currentWidget(&readlineWidget);
  readlineWidget.onFinish = [=](std::string line, bool isEOF) {
    callback(line, isEOF);
    readlineWidget.visible = false;
    currentIndex(oldWidget);
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
  infoLine.layout(pos, size);
  readlineWidget.layout(pos, size);

  if (tabBar.visible)         size.height -= tabBar.size.height;
  if (progressBar.visible)    size.height -= progressBar.size.height;
  if (infoLine.visible)       size.height -= infoLine.size.height;
  if (readlineWidget.visible) size.height -= readlineWidget.size.height;

  windows.layout(pos, size);
  
  VerticalContainer::layout(this->pos, this->size);
}

bool MainWindow :: handleKey(int key) {
  if (! VerticalContainer::handleKey(key))
    Actions::call(Bindings::global[key]);
  return true;
}

