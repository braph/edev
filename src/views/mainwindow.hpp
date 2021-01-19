#ifndef VIEWS_MAINWINDOW_HPP
#define VIEWS_MAINWINDOW_HPP

/* This view holds all views of the application */

#include "splash.hpp"
#include "help.hpp"
#include "info.hpp"
#include "progressbar.hpp"
#include "infoline.hpp"
#include "playlist.hpp"
#include "browser.hpp"
#include "tabbar.hpp"

#include "../ui/core.hpp"
#include "../ui/container.hpp"
#include "../widgets/readline.hpp"
#include "../application.hpp"

namespace Views {

enum class MainWidgets : unsigned char {
  NONE,
  INFOLINE,
  TABBAR,
  READLINE,
  WINDOWS,
  PROGRESSBAR
};

enum class TabWidgets : unsigned char {
  NONE,
  SPLASH,
  PLAYLIST,
  BROWSER,
  INFO,
  HELP
};

class MainWindow : public UI::VerticalContainer {
public:
  MainWindow();

  void layout(UI::Pos, UI::Size)    override;
  bool handle_key(int)              override;

  void readline(std::string, ReadlineWidget::onFinishFunction);

  Views::InfoLine       infoLine;
  Views::ProgressBar    progressBar;
  Views::TabBar         tabBar;
  ReadlineWidget        readlineWidget;
  UI::StackedContainer  windows;
  Views::Splash         splash;
  Views::Playlist       playlist;
  Views::Browser        browser;
  Views::Info           info;
  Views::Help           help;
};

} // namespace Views

#endif
