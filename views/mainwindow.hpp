#ifndef VIEWS_MAINWINDOW_HPP
#define VIEWS_MAINWINDOW_HPP

/* This view holds all views of the application */

#include "splash.hpp"
#include "help.hpp"
#include "info.hpp"
#include "progressbar.hpp"
#include "playinginfo.hpp"
#include "playlist.hpp"
#include "tabbar.hpp"

#include "../actions.hpp"
#include "../player.hpp"
#include "../database.hpp"
#include "../ui.hpp"
#include "../ui/container.hpp"
#include "../widgets/readline.hpp"

namespace Views {

class MainWindow : public UI::VerticalContainer {
public:
  Actions&              actions;
  Views::PlayingInfo    playingInfo;
  Views::ProgressBar    progressBar;
  Views::TabBar         tabBar;
  ReadlineWidget        readlineWidget;
  UI::StackedContainer  windows;
  Views::Splash         splash;
  Views::Playlist       playlist;
  Views::Info           info;
  Views::Help           help;
  MainWindow(Actions&, Database::Database&, Mpg123Player&);
  void layout(UI::Pos, UI::Size);
  bool handleKey(int);
  void readline(const std::string&, ReadlineWidget::onFinishFunction);
};

} // namespace Views

#endif
