#include "mainwindow.hpp"
#include "../bindings.hpp"
#include "../config.hpp"

using namespace UI;
using namespace Views;

#define case break;case
MainWindow :: MainWindow(Context& ctxt)
: infoLine()
, progressBar()
, tabBar()
, readlineWidget()
, windows()
, splash()
, playlist(ctxt)
, info(ctxt)
, help()
, ctxt(ctxt)
{
  readlineWidget.visible = false;

  for (auto w : Config::main_widgets) {
    switch (w) {
      case MainWidgets::NONE:
      case MainWidgets::INFOLINE:     addWidget(&infoLine);
      case MainWidgets::PROGRESSBAR:  addWidget(&progressBar);
      case MainWidgets::TABBAR:       addWidget(&tabBar);
      case MainWidgets::READLINE:     addWidget(&readlineWidget);
      case MainWidgets::WINDOWS:      addWidget(&windows);
    }
  }

  setCurrentIndex(indexOf(&windows));

  for (auto w : Config::tabs_widgets) {
    switch (w) {
      case TabWidgets::NONE:
      case TabWidgets::SPLASH:
        windows.addWidget(&splash);
        tabBar.addTab("splash");
      case TabWidgets::PLAYLIST:
        windows.addWidget(&playlist);
        tabBar.addTab("playlist");
      case TabWidgets::BROWSER:
        windows.addWidget(&playlist); // TODO
        tabBar.addTab("browser");
      case TabWidgets::INFO:
        windows.addWidget(&info);
        tabBar.addTab("info");
      case TabWidgets::HELP:
        windows.addWidget(&help);
        tabBar.addTab("help");
    }
  }
}
#undef case

void MainWindow :: readline(std::string prompt, ReadlineWidget::onFinishFunction callback) {
  readlineWidget.visible = true;
  readlineWidget.setPrompt(std::move(prompt));
  int oldWidget = currentIndex();
  setCurrentIndex(indexOf(&readlineWidget));
  readlineWidget.onFinish = [=](std::string line, bool isEOF) {
    callback(line, isEOF);
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
    Actions::call(ctxt, Bindings::global[key]);
  return true;
}
