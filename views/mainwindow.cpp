#include "mainwindow.hpp"
#include "../bindings.hpp"
#include "../config.hpp"

#include "../lib/switch.hpp"

using namespace UI;
using namespace Views;
using pack = StringPack::AlphaNoCase;

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
    switch (pack::pack_runtime(w)) {
      case pack("infoline"):      addWidget(&infoLine);         break;
      case pack("progressbar"):   addWidget(&progressBar);      break;
      case pack("tabbar"):        addWidget(&tabBar);           break;
      case pack("readline"):      addWidget(&readlineWidget);   break;
      case pack("windows"):       addWidget(&windows);          break;
      default: assert(!"not reached");
    }
  }

  setCurrentIndex(indexOf(&windows));

  for (const auto& w : Config::tabs_widgets) {
    switch (pack::pack_runtime(w)) {
      case pack("splash"):    windows.addWidget(&splash);       break;
      case pack("playlist"):  windows.addWidget(&playlist);     break;
      case pack("browser"):   windows.addWidget(&playlist);     break; /*TODO*/
      case pack("info"):      windows.addWidget(&info);         break;
      case pack("help"):      windows.addWidget(&help);         break;
      default: assert(!"not reached");
    }

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
