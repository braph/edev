#ifndef VIEWS_MAINWINDOW_HPP
#define VIEWS_MAINWINDOW_HPP

/* This view holds all views of the application */

#include "splash.hpp"
#include "help.hpp"
#include "info.hpp"
#include "progressbar.hpp"
#include "infoline.hpp"
#include "playlist.hpp"
#include "tabbar.hpp"

#include "../ui.hpp"
#include "../ui/container.hpp"
#include "../widgets/readline.hpp"
#include "../application.hpp"

namespace Views {

class MainWindow : public UI::VerticalContainer {
public:
  Views::InfoLine       infoLine;
  Views::ProgressBar    progressBar;
  Views::TabBar         tabBar;
  ReadlineWidget        readlineWidget;
  UI::StackedContainer  windows;
  Views::Splash         splash;
  Views::Playlist       playlist;
  Views::Info           info;
  Views::Help           help;
  Context&              ctxt;
  MainWindow(Context&);
  void layout(UI::Pos, UI::Size);
  bool handleKey(int);
  void readline(std::string, ReadlineWidget::onFinishFunction);
};

} // namespace Views

#endif
