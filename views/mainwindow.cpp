#include "mainwindow.hpp"
#include "../bindings.hpp"
#include "../config.hpp"

using namespace UI;
using namespace Views;

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

  for (const auto& w : Config::main_widgets) {
    /**/ if (w == "infoline")       addWidget(&infoLine);
    else if (w == "progressbar")    addWidget(&progressBar);
    else if (w == "tabbar")         addWidget(&tabBar);
    else if (w == "readline")       addWidget(&readlineWidget);
    else if (w == "windows")        addWidget(&windows);
    else assert(0);
  }

  setCurrentIndex(indexOf(&windows));

  for (const auto& w : Config::tabs_widgets) {
    /**/ if (w == "splash")   windows.addWidget(&splash);
    else if (w == "playlist") windows.addWidget(&playlist);
    else if (w == "browser")  windows.addWidget(&playlist); /*TODO*/
    else if (w == "info")     windows.addWidget(&info);
    else if (w == "help")     windows.addWidget(&help);
    else assert(0);
    tabBar.addTab(w);
  }
}

void MainWindow :: readline(std::string prompt, ReadlineWidget::onFinishFunction callback) {
  readlineWidget.visible = true;
  readlineWidget.setPrompt(std::move(prompt));
  int oldWidget = currentIndex();
  setCurrentIndex(indexOf(&readlineWidget));
  readlineWidget.onFinish = [=](std::string line, bool notEOF) {
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
