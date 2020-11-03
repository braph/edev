#include "info.hpp"

#include "../ektoplayer.hpp"
#include "../bindings.hpp"
#include "../theme.hpp"
#include "../config.hpp"
#include "../player.hpp"
#include "../url_handler.hpp"
#include "../ui/colors.hpp"
#include <lib/filesystem.hpp>
#include <lib/bit_tools.hpp>

using namespace UI;
using namespace Views;
using ElementID = Theme::ElementID;

#define START_HEADING    1
#define START_TAG        3
#define START_TAG_VALUE  20
#define START_INFO       3
#define START_INFO_VALUE 26
#define TRY_LINE_BREAK   70 // Try to break the line on next space
#define FORCE_LINE_BREAK 85 // Forces line breaks even in words

struct MarkupParser {
  const char* it;
  enum Type { BOLD = 1, ITALIC = 2, LINK_TEXT = 4, LINK_URL = 8 };
  int type;

  MarkupParser(const char* s) : it(s), type(0) {
    mbtowc(NULL, NULL, 0); // Reset internal state
  }

  wchar_t next_char() {
    wchar_t c;
    for (;;) {
      int n = std::mbtowc(&c, it, MB_CUR_MAX);
      if (n == 0)   { return 0;       } // EOF
      if (n == -1)  { it++; continue; } // Skip invalid char
      it += n;

      bool haveDouble = (c == *it); // `Having ((double`
      bool haveMore   = (haveDouble && c == *(it+1)); // `Having (((more`

      if (haveDouble)
        switch (*it) {
          case '*': type ^= BOLD;      ++it; continue;
          case '_': type ^= ITALIC;    ++it; continue;
          case '(': if (haveMore) break;
                    type |= LINK_TEXT; ++it; continue;
          case ')': type &=~LINK_TEXT; ++it; continue;
          case '[': if (haveMore) break;
                    type |= LINK_URL;  ++it; continue;
          case ']': type &=~LINK_URL;  ++it; continue;
        }

      return c;
    }
  }
};

void Info :: layout(Pos pos, Size size) {
  this->pos  = pos;
  this->size = size;
  resize(110, size.width);
  pad_minrow = 0;
  pad_mincol = 0;
}

void Info :: track(Database::Tracks::Track track) noexcept {
  if (track != _track) {
    _track = track;
    draw();
  }
}

inline void Info :: draw_heading(int y, const char* heading) noexcept {
  attrSet(Theme::get(ElementID::INFO_HEAD));
  mvAddStr(y, START_HEADING, heading);
}

void Info :: draw_tag(int y, const char* tag) noexcept {
  attrSet(Theme::get(ElementID::INFO_TAG));
  mvAddStr(y, START_TAG, tag);
  attrSet(Theme::get(ElementID::INFO_VALUE));
  moveCursor(y, START_TAG_VALUE);
}

void Info :: draw_info(int y, const char* info) noexcept {
  attrSet(Theme::get(ElementID::INFO_TAG));
  mvAddStr(y, START_INFO, toWideString(info));
  attrSet(Theme::get(ElementID::INFO_VALUE));
  moveCursor(y, START_INFO_VALUE);
}

void Info :: draw_link(std::string url, std::string title) noexcept {
  attrSet(Theme::get(ElementID::URL));
  UI::Pos start = cursorPos();
  addStr(toWideString(title));
  _clickable_urls.add(start, cursorPos(), {std::move(url), std::move(title)});
}

void Info :: draw() {
  clear();
  _clickable_urls.clear();
  int x, y = 1;

  if (_track) {
    const auto& track = _track;
    const auto& album = _track.album();

    // Track ==================================================================
    draw_heading(y++, "Current track");
    draw_tag(y++, "Title");
    *this << toWideString(track.title());
    if (*track.remix())
      *this << " (" << toWideString(track.remix()) << ')';

    draw_tag(y++, "Artist");
    *this << toWideString(track.artist());

    draw_tag(y++, "Number");
    printW("%02d", track.number());

    draw_tag(y++, "BPM");
    *this << track.bpm();

    draw_tag(y++, "Length");
    printW("%02d:%02d", ctxt.player->length()/60, ctxt.player->length()%60);

    // Album ==================================================================
    y++;
    draw_heading(y++, "Current album");

    draw_tag(y++, "Album");
    std::string album_url = album.url();
    Ektoplayer::url_expand(album_url, EKTOPLAZM_ALBUM_BASE_URL);
    draw_link(std::move(album_url), album.title());

    draw_tag(y++, "Artist");
    *this << toWideString(album.artist());

    draw_tag(y++, "Date");
    *this << time_format(album.date(), "%B %d, %Y");

    draw_tag(y++, "Styles");
    const char* comma = "";
    for (auto id : extract_set_bits(unsigned(album.styles()))) {
      *this << comma << track.table->db.styles[id].name();
      comma = ", ";
    }

    draw_tag(y++, "Downloads");
    *this << album.download_count();

    draw_tag(y++, "Rating");
    printW("%2.2f%% (%d Votes)", album.rating(), album.votes());

    draw_tag(y++, "Cover");
    std::string cover_url = album.cover_url();
    Ektoplayer::url_expand(cover_url, EKTOPLAZM_COVER_BASE_URL, ".jpg");
    draw_link(std::move(cover_url), "Cover");

    // Description ============================================================
    y++;
    draw_heading(y++, "Description");
    moveCursor(y, START_TAG);
    MarkupParser markupParser(album.description());
    std::string linkURL, linkText;
    wchar_t c;
    while ((c = markupParser.next_char())) {
      if (markupParser.type & MarkupParser::LINK_TEXT)
        linkText.append(toNarrowChar(c));
      else if (markupParser.type & MarkupParser::LINK_URL)
        linkURL.append(toNarrowChar(c));
      else {
        unsigned int attr = Theme::get(ElementID::INFO_VALUE);
        if (markupParser.type & MarkupParser::BOLD)   attr |= A_BOLD;
        if (markupParser.type & MarkupParser::ITALIC) attr |= A_UNDERLINE;

        getCursorYX(y, x);
        if      (x < START_TAG)                   moveCursor(y,   START_TAG);
        else if (x >= FORCE_LINE_BREAK)           moveCursor(y+1, START_TAG);
        else if (x >= TRY_LINE_BREAK && c == ' ') moveCursor(y+1, START_TAG-1);

        if (!linkText.empty() && !linkURL.empty()) {
          if (linkURL == "@") // Protected email, see updater.cpp
            linkURL = "Protected e-mail";
          else
            linkURL = "http://" + linkURL; // http?s stripped off, updater.cpp
          draw_link(std::move(linkURL), std::move(linkText));
        }

        attrSet(attr);
        *this << c;
      }
    }

    y = getCursorY() + 2;
  }

  // Player ===================================================================
  draw_heading(y++, "Player");
  draw_info(y++, "Version");
  *this << VERSION;

  draw_info(y++, "Tracks in database");
  *this << ctxt.database->tracks.size();

  draw_info(y++, "Albums in database");
  *this << ctxt.database->albums.size();

  draw_info(y++, "Tracks in playlist");
  *this << ctxt.mainwindow->playlist.list()->size();

  draw_info(y++, "Cache dir size");
  *this << Filesystem::dir_size(Config::cache_dir) / 1024 / 1024 << "MB";

  draw_info(y++, "Archive dir size");
  *this << Filesystem::dir_size(Config::archive_dir) / 1024 / 1024 << "MB";

  draw_info(y++, "Ektoplazm URL");
  draw_link(EKTOPLAZM_URL, EKTOPLAZM_URL);

  draw_info(y++, "Github URL");
  draw_link(GITHUB_URL, GITHUB_URL);

  // URLs ===================================================================
  y++;
  draw_heading(y++, "URLs");
  for (size_t i = 0; i < _clickable_urls.size() - 2; ++i) {
    draw_info(y++, _clickable_urls[i].data.title.c_str());
    mvAddStr(y++, START_INFO + 2, toWideString(_clickable_urls[i].data.url));
  }
}

bool Info :: handle_key(int n) {
  switch (Bindings::pad[n]) {
  case Actions::UP:         up();         return true;
  case Actions::DOWN:       down();       return true;
  case Actions::PAGE_UP:    page_up();    return true;
  case Actions::PAGE_DOWN:  page_down();  return true;
  case Actions::TOP:        top();        return true;
  case Actions::BOTTOM:     bottom();     return true;
  default:                                return false;
  }
}

bool Info :: handle_mouse(MEVENT& m) {
  if (wmouse_trafo(win, &m.y, &m.x, false)) {
    m.y += pad_minrow;
    m.x += pad_mincol;
    auto event = _clickable_urls.find(m);
    if (event != _clickable_urls.end())
      open_url(event->data.url);
    return true;
  }
  return false;
}

