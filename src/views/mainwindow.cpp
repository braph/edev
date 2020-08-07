#include "mainwindow.hpp"
#include "../bindings.hpp"
#include "../config.hpp"
#define case break;case

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
  tabBar.visible         = Config::tabbar_visible;
  infoLine.visible       = Config::infoline_visible;
  progressBar.visible    = Config::progressbar_visible;
  readlineWidget.visible = false;

  for (auto w : Config::main_widgets) {
    switch (w) {
    case MainWidgets::NONE:
    case MainWidgets::INFOLINE:     add_widget(&infoLine);
    case MainWidgets::PROGRESSBAR:  add_widget(&progressBar);
    case MainWidgets::TABBAR:       add_widget(&tabBar);
    case MainWidgets::READLINE:     add_widget(&readlineWidget);
    case MainWidgets::WINDOWS:      add_widget(&windows);
    }
  }

  current_widget(&windows);

  for (auto w : Config::tabs_widgets) {
    switch (w) {
    case TabWidgets::NONE:
    case TabWidgets::SPLASH:
      windows.add_widget(&splash);
      tabBar.add_tab("splash");
    case TabWidgets::PLAYLIST:
      windows.add_widget(&playlist);
      tabBar.add_tab("playlist");
    case TabWidgets::BROWSER:
      windows.add_widget(&playlist); // TODO [later]
      tabBar.add_tab("browser");
      break;
    case TabWidgets::INFO:
      windows.add_widget(&info);
      tabBar.add_tab("info");
    case TabWidgets::HELP:
      windows.add_widget(&help);
      tabBar.add_tab("help");
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
    layout(pos, size);
    draw();
    noutrefresh();
  };
  layout(pos, size);
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

