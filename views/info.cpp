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
#define TRY_LINE_BREAK   70 // Try to break the line on next space
#define FORCE_LINE_BREAK 85 // Forces line breaks even in words

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
  mvwaddwstr(win, y, START_INFO, toWideString(info));
  wattrset(win, Theme::get(Theme::INFO_VALUE));
  wmove(win, y, START_INFO_VALUE);
}

void Info :: drawLink(const std::string& url, const std::string &title, bool addURL = true) {
  wattrset(win, Theme::get(Theme::URL));
  UI::Pos start = cursorPos();
  waddwstr(win, toWideString(title));
  if (addURL)
    clickableURLs.add(start, cursorPos(), UrlAndTitle(url, title));
}

struct MarkupParser {
  const char* it;
  enum Type { BOLD = 1, ITALIC = 2, LINK_TEXT = 4, LINK_URL = 8 };
  int type;

  MarkupParser(const char* s) : it(s), type(0) {
    mbtowc(NULL, NULL, 0); // Clear mbtowc's shift state
  }

  wchar_t nextChar() {
    wchar_t c;
    for (;;) {
      size_t read = mbtowc(&c, it, 6);
      if (read == -1) { it++; continue; } // Skip invalid char
      if (read == 0)  { return 0;       } // EOF
      it += read;

      if (c == *it) // doubled char
        switch (*it) {
          case '*': type ^= BOLD;      it++; continue;
          case '_': type ^= ITALIC;    it++; continue;
          case '(': type |= LINK_TEXT; it++; continue;
          case ')': type &=~LINK_TEXT; it++; continue;
          case '[': type |= LINK_URL;  it++; continue;
          case ']': type &=~LINK_URL;  it++; continue;
        }

      return c;
    }
  }
};

void Info :: draw() {
  clickableURLs.clear();
  wclear(win);
  int y = 1;

  if (currentTrack) {
    std::string buffer;

    auto track = currentTrack;
    auto album = currentTrack.album();

    // Track ==================================================================
    drawHeading(y++, "Current track");
    drawTag(y++, "Title");
    *this << toWideString(track.title());
    if (*(track.remix()))
      *this << " (" << toWideString(track.remix()) << ')';

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
    drawLink(buffer, album.title());

    drawTag(y++, "Artist");
    *this << toWideString(album.artist());

    drawTag(y++, "Date");
    *this << time_format(album.date(), "%B %d, %Y");

    drawTag(y++, "Styles");
    Database::StylesArray styleIDs(album.styles());
    const char* comma = "";
    for (auto id : styleIDs)
      if (id) {
        *this << comma << track.db.styles[id].name();
        comma = ", ";
      }

    drawTag(y++, "Downloads");
    *this << album.download_count();

    drawTag(y++, "Rating");
    wprintw(win, "%2.2f%% (%d Votes)", album.rating(), album.votes());

    drawTag(y++, "Cover");
    buffer = album.cover_url();
    Ektoplayer::url_expand(buffer, EKTOPLAZM_COVER_BASE_URL, ".jpg");
    drawLink(buffer, "Cover");

    y++; // Newline

    // Description ============================================================
    drawHeading(y++, "Description");
    wmove(win, y, START_TAG);
    MarkupParser markupParser(album.description());
    std::string linkURL;
    std::string linkText;
    wchar_t c;
    while ((c = markupParser.nextChar())) {
      int attr = Theme::get(Theme::INFO_VALUE);
      if (markupParser.type & MarkupParser::BOLD)   attr |= A_BOLD;
      if (markupParser.type & MarkupParser::ITALIC) attr |= A_UNDERLINE;

      if (markupParser.type & MarkupParser::LINK_TEXT) {
        linkText.append(toNarrowChar(c));
        continue;
      }

      if (markupParser.type & MarkupParser::LINK_URL) {
        linkURL.append(toNarrowChar(c));
        continue;
      }

      int x;
      getyx(win, y, x);
      if (x < 3)                                wmove(win, y,   START_TAG);
      else if (x >= FORCE_LINE_BREAK)           wmove(win, y+1, START_TAG);
      else if (x >= TRY_LINE_BREAK && c == ' ') wmove(win, y+1, START_TAG-1);

      if (!linkText.empty() && !linkURL.empty()) {
        if (linkURL == "@") // Protected email, see updater.cpp
          linkURL = "Protected e-mail";
        drawLink(linkURL, linkText);
        linkURL.clear();
        linkText.clear();
      }

      wattrset(win, attr);
      *this << c;
    }

    y = getcury(win) + 2;
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
  drawLink(EKTOPLAZM_URL, EKTOPLAZM_URL, false);

  drawInfo(y++, "Github URL");
  drawLink(GITHUB_URL, GITHUB_URL, false);
  y++;

  // URLs ===================================================================
  drawHeading(y++, "URLs");
  for (auto event : clickableURLs) {
    drawInfo(y++, event.data.second.c_str());
    *this << toWideString(event.data.first.c_str());
  }
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
