#include "info.hpp"

#include "../theme.hpp"
#include "../config.hpp"
#include "../colors.hpp"
#include "../common.hpp"
#include "../filesystem.hpp"
#include "../ektoplayer.hpp"

using namespace UI;
using namespace Views;

#define START_HEADING    1
#define START_TAG        3
#define START_TAG_VALUE  20
#define START_INFO       3
#define START_INFO_VALUE 26

Info :: Info(Database& db, Mpg123Player& p)
: db(db)
, player(p)
, currentTrack(db, 0)
{}

void Info :: layout(Pos pos, Size size) {
  this->pos  = pos;
  this->size = size;
  wresize(win, 200, 110);
  pad_minrow = 0;
  pad_mincol = 0;
}

void Info :: setCurrentTrack(Database::Tracks::Track track) {
  if (track != currentTrack) {
    currentTrack = track;
    draw();
  }
}

void Info :: drawHeading(int y, const char* heading) {
  wattrset(win, Theme::get(Theme::INFO_HEAD));
  mvwaddstr(win, y, START_HEADING, heading);
}

void Info :: drawTag(int y, const char* tag) {
  wattrset(win, Theme::get(Theme::INFO_TAG));
  mvwaddstr(win, y, START_TAG, tag);
  wattrset(win, Theme::get(Theme::INFO_VALUE));
  wmove(win, y, START_TAG_VALUE);
}

void Info :: drawInfo(int y, const char* info) {
  wattrset(win, Theme::get(Theme::INFO_TAG));
  mvwaddstr(win, y, START_INFO, info);
  wattrset(win, Theme::get(Theme::INFO_VALUE));
  wmove(win, y, START_INFO_VALUE);
}

void Info :: drawURL(const std::string& url, const std::string &title, bool saveURL = true) {
  wattrset(win, Theme::get(Theme::URL));
  UI::Pos start = cursorPos();
  waddwstr(win, toWideString(title));
  if (saveURL)
    clickableURLs.add(start, cursorPos(), UrlAndTitle(url, title));
}

struct MarkupParser {
  const char* it;
  enum Type : char { NORMAL, BOLD, ITALIC, URL, URL_TEXT };
  MarkupParser(const char* s) : it(s) {}

  bool getText(std::string& buffer, Type& type) {
    buffer.clear();
    
    const struct { char start, stop; Type type; } markupPairs[] = {
      {'*', '*', BOLD},
      {'_', '_', ITALIC},
      {'(', ')', URL_TEXT},
      {'[', ']', URL}
    };

    for (const auto& pair : markupPairs) {
      if (haveDouble(pair.start)) {
        it += 2;
        type = pair.type;
        while (*it)
          if (haveDouble(pair.stop)) { it+=2; break; }
          else buffer.push_back(*it++);
        return true;
      }
    }

    type = NORMAL;
    while (*it && ((*it != '*' && *it != '_' && *it != '(' && *it != '[') || !haveDouble(*it)))
      buffer.push_back(*it++);

    return !buffer.empty();
  }

  inline bool haveDouble(char c) {
    return (*it == c && *(it+1) == c);
  }
};

void Info :: draw() {
  clickableURLs.clear();
  wclear(win);
  int y = 1;

  if (currentTrack) {
    const size_t bufsz = 64;
    std::string buffer('\0', bufsz);
    char* cbuf = const_cast<char*>(buffer.c_str());

    auto track = currentTrack;
    auto album = currentTrack.album();

    // Track ==================================================================
    drawHeading(y++, "Current track");
    drawTag(y++, "Title");
    *this << toWideString(track.title()) << ' ' << toWideString(track.remix());

    drawTag(y++, "Artist");
    *this << toWideString(track.artist());

    drawTag(y++, "Number");
    wprintw(win, "%02d", track.number());

    drawTag(y++, "BPM");
    *this << track.bpm();

    drawTag(y++, "Length");
    wprintw(win, "%02d:%02d", player.length()/60, player.length()%60);

    y++; // Newline

    // Album ==================================================================
    drawHeading(y++, "Current album");

    drawTag(y++, "Album");
    buffer = album.url();
    Ektoplayer::url_expand(buffer, EKTOPLAZM_ALBUM_BASE_URL);
    drawURL(buffer, album.title());

    drawTag(y++, "Artist");
    *this << toWideString(album.artist());

    drawTag(y++, "Date");
    *this << time_format(album.date(), "%B %d, %Y", cbuf, bufsz);

    drawTag(y++, "Styles");
    TinyPackedArray<uint8_t, uint32_t> styleIDs(album.styles());
    auto beg = styleIDs.begin();
    auto end = styleIDs.end();
    if (*beg)
      *this << track.db.styles[*beg++].name();
    while (beg != end && *beg)
      *this << ", " << track.db.styles[*beg++].name();
      
    drawTag(y++, "Downloads");
    *this << album.download_count();

    drawTag(y++, "Rating");
    wprintw(win, "%2.2f%% (%d Votes)", album.rating(), album.votes());

    drawTag(y++, "Cover");
    buffer = album.cover_url();
    Ektoplayer::url_expand(buffer, EKTOPLAZM_COVER_BASE_URL, ".jpg");
    drawURL(buffer, "Cover");

    y++; // Newline

    // Description ============================================================
    drawHeading(y++, "Description");
    wmove(win, y, START_TAG);
    MarkupParser::Type type;
    MarkupParser markupParser(album.description());
    std::string urlText;
    while (markupParser.getText(buffer, type)) {
      int attr = Theme::get(Theme::INFO_VALUE);
      switch (type) {
      case MarkupParser::NORMAL:    break;
      case MarkupParser::BOLD:      attr |= A_BOLD; break;
      case MarkupParser::ITALIC:    attr |= A_UNDERLINE; break;
      case MarkupParser::URL_TEXT:  urlText = buffer;
                                    continue; // URL follows
      case MarkupParser::URL:       drawURL(buffer, urlText);
                                    continue;
      }
      wattrset(win, attr);
      waddwstr(win, toWideString(buffer));
    }

    y = getcury(win) + 2;

    // URLs ===================================================================
    drawHeading(y++, "URLs");
    for (auto event : clickableURLs) { // TODO wideString
      drawInfo(y++, event.data.second.c_str()); *this << event.data.first.c_str();
    }
    y++;
  }

  // Player ===================================================================
  drawHeading(y++, "Player");
  drawInfo(y++, "Version");
  *this << VERSION;

  drawInfo(y++, "Tracks in database");
  *this << db.tracks.size();

  drawInfo(y++, "Albums in database");
  *this << db.albums.size();

  drawInfo(y++, "Tracks in playlist");
  *this << 0; // TODO

  drawInfo(y++, "Cache dir size");
  *this << Filesystem::dir_size(Config::cache_dir) / 1024 / 1024 << "MB";

  drawInfo(y++, "Archive dir size");
  *this << Filesystem::dir_size(Config::archive_dir) / 1024 / 1024 << "MB";

  drawInfo(y++, "Ektoplazm URL");
  drawURL(EKTOPLAZM_URL, EKTOPLAZM_URL, false);

  drawInfo(y++, "Github URL");
  drawURL(GITHUB_URL, GITHUB_URL, false);
}

bool Info :: handleMouse(MEVENT& m) {
  if (wmouse_trafo(win, &m.y, &m.x, false)) {
    auto event = clickableURLs.find(m);
    if (event != clickableURLs.end())
      open_url(event->data.first);
    return true;
  }
  return false;
}

#ifdef TEST_INFO
#include "../test.hpp"
int main() {
  TEST_BEGIN();
  NCURSES_INIT();

  Bindings::init();

  //Theme::current = colors;

  Widget *s = new Views::Info;
  s->layout({10,10}, {30,80});
  s->draw();
  s->noutrefresh();
  doupdate();
  wgetch(s->active_win());

  TEST_END();
}
#endif
