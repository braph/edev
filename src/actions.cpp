#include "actions.hpp"

#include "updater.hpp"
#include "database.hpp"
#include "programs.hpp"
#include "trackloader.hpp"
#include "mpg123playback.hpp"
#include "views/mainwindow.hpp"

#include <csignal>
#include <cassert>

int Actions :: call(ActionID id) {
  int index;

  switch (id) {
  case NONE:
    break;
  case QUIT:
    std::raise(SIGTERM);
    break;
  case REDRAW:
    std::raise(SIGWINCH);
    break;
  case UPDATE:
    updater.start();
    break;
  case SHOW_COVER:
    if (! mainwindow->playlist.empty() && mainwindow->playlist.active_index() >= 0) {
      auto track = mainwindow->playlist.active_item();
      std::string cover_url = track.album().cover_url();
      Ektoplayer::url_expand(cover_url, EKTOPLAZM_COVER_BASE_URL, ".jpg");
      Programs::image_viewer(cover_url).detach();
    }
    break;

  // Player
  case PLAYER_FORWARD:
    player.seek(+10);
    break;
  case PLAYER_BACKWARD:
    player.seek(-10);
    break;
  case PLAYER_STOP:
    player.stop();
    break;
  case PLAYER_TOGGLE:
    player.toggle();
    break;

  // Widgets visibility
  case TABBAR_TOGGLE:
    mainwindow->tabBar.visible = !mainwindow->tabBar.visible;
    std::raise(SIGWINCH);
    break;
  case PLAYINGINFO_TOGGLE:
    mainwindow->infoLine.visible = !mainwindow->infoLine.visible;
    std::raise(SIGWINCH);
    break;
  case PROGRESSBAR_TOGGLE:
    mainwindow->progressBar.visible = !mainwindow->progressBar.visible;
    std::raise(SIGWINCH);
    break;

  // Playlist
  case PLAYLIST_GOTO_CURRENT:
    mainwindow->playlist.goto_active();
    break;
  case PLAYLIST_DOWNLOAD:
    if (! mainwindow->playlist.empty() && mainwindow->playlist.cursor_index() >= 0) {
      try {
        trackloader.download_album(mainwindow->playlist.cursor_item());
      } catch (const std::exception& e) {
        log_write("%s\n", e);
      }
    }
    break;
  case PLAYLIST_NEXT:
    index = mainwindow->playlist.active_index() + 1;
    goto PLAYLIST_PLAY;
  case PLAYLIST_PREV:
    index = mainwindow->playlist.active_index() - 1;
    goto PLAYLIST_PLAY;
  case PLAYLIST_PLAY:
    index = mainwindow->playlist.cursor_index();
    goto PLAYLIST_PLAY;

PLAYLIST_PLAY:
    mainwindow->playlist.active_index(index);
    if (! mainwindow->playlist.empty() && mainwindow->playlist.active_index() >= 0) {
      try {
        auto track = mainwindow->playlist.active_item();
        player.play(trackloader.get_file_for_track(track, false));
      } catch (const std::exception& e) {
        log_write("%s\n", e);
      }
    }
    break;

  // Windows
  case TABS_NEXT:
    index = mainwindow->windows.current_index() + 1;
    goto SELECT_TAB;
  case TABS_PREV:
    index = mainwindow->windows.current_index() - 1;
    goto SELECT_TAB;
  case SPLASH_SHOW:
    index = mainwindow->windows.index_of(&mainwindow->splash);
    goto SELECT_TAB;
  case PLAYLIST_SHOW:
    index = mainwindow->windows.index_of(&mainwindow->playlist);
    goto SELECT_TAB;
  case BROWSER_SHOW:
    index = mainwindow->windows.index_of(&mainwindow->playlist); // TODO [later]
    goto SELECT_TAB;
  case INFO_SHOW:
    index = mainwindow->windows.index_of(&mainwindow->info);
    goto SELECT_TAB;
  case HELP_SHOW:
    index = mainwindow->windows.index_of(&mainwindow->help);
    goto SELECT_TAB;

SELECT_TAB:
    index = (index + mainwindow->windows.count()) % mainwindow->windows.count();
    mainwindow->windows.current_index(index);
    mainwindow->tabBar.current_index(index);
    std::raise(SIGWINCH); // TODO: We shouldn't need to refresh the screen
    break;

  // Silence warnings
  case UP:
  case DOWN:
  case PAGE_UP:
  case PAGE_DOWN:
  case TOP:
  case BOTTOM:
  case SEARCH_UP:
  case SEARCH_DOWN:
  case SEARCH_PREV:
  case SEARCH_NEXT:
  case ACTIONID_COUNT: break;
  default: assert(0);
  }

  return 0;
}

static const char *action_strings[Actions::ACTIONID_COUNT] = {
#define X(ENUM, STR) STR,
  XACTIONS
#undef X
};

Actions::ActionID Actions :: parse(const std::string& s) {
  for (int i = 0; i < Actions::ACTIONID_COUNT; ++i)
    if (s == action_strings[i])
      return static_cast<Actions::ActionID>(i);
  return Actions::ACTIONID_COUNT;
}

const char* Actions :: to_string(Actions::ActionID id) {
  return action_strings[id];
}

