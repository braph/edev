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
    case MainWidgets::INFOLINE:     add_widget(&infoLine);         break;
    case MainWidgets::PROGRESSBAR:  add_widget(&progressBar);      break;
    case MainWidgets::TABBAR:       add_widget(&tabBar);           break;
    case MainWidgets::READLINE:     add_widget(&readlineWidget);   break;
    case MainWidgets::WINDOWS:      add_widget(&windows);          break;
    }
  }

  current_widget(&windows);

  for (auto w : Config::tabs_widgets) {
    switch (w) {
    case TabWidgets::NONE:
      break;
    case TabWidgets::SPLASH:
      windows.add_widget(&splash);
      tabBar.add_tab("splash");
      break;
    case TabWidgets::PLAYLIST:
      windows.add_widget(&playlist);
      tabBar.add_tab("playlist");
      break;
    case TabWidgets::BROWSER:
      windows.add_widget(&playlist); // TODO
      tabBar.add_tab("browser");
      break;
    case TabWidgets::INFO:
      windows.add_widget(&info);
      tabBar.add_tab("info");
      break;
    case TabWidgets::HELP:
      windows.add_widget(&help);
      tabBar.add_tab("help");
      break;
    }
  }
}

void MainWindow :: readline(std::string prompt, ReadlineWidget::onFinishFunction callback) {
  readlineWidget.visible = true;
  readlineWidget.set_prompt(std::move(prompt));
  int oldWidget = current_index();
  current_widget(&readlineWidget);
  readlineWidget.onFinish = [=](std::string line, bool isEOF) {
    callback(line, isEOF);
    readlineWidget.visible = false;
    current_index(oldWidget);
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

bool MainWindow :: handle_key(int key) {
  if (! VerticalContainer::handle_key(key))
    Actions::call(Bindings::global[key]);
  return true;
}

