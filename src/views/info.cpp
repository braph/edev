#include "info.hpp"

#include "../ektoplayer.hpp"
#include "../bindings.hpp"
#include "../theme.hpp"
#include "../config.hpp"
#include "../player.hpp"
#include "../programs.hpp"
#include "../ui/colors.hpp"
#include <lib/filesystem.hpp>
#include <lib/bit_tools.hpp>

using namespace UI;
using namespace Views;

#define START_HEADING    1
#define START_TAG        3
#define START_TAG_VALUE  20
#define START_INFO       3
#define START_INFO_VALUE 26
#define TRY_LINE_BREAK   70 // Try to break the line on next space
#define FORCE_LINE_BREAK 85 // Forces line breaks even in words

struct MarkupParser {
  enum Type { BOLD = 1, ITALIC = 2, LINK_TEXT = 4, LINK_URL = 8 };

  const char* it;
  int type;
  char multibyte[MB_LEN_MAX + 1];

  MarkupParser(const char* s) : it(s), type(0) {
    mbtowc(NULL, NULL, 0); // Reset internal state
  }

  wchar_t next_char() {
    wchar_t c;
    for (;;) {
      int n = std::mbtowc(&c, it, MB_LEN_MAX);
      if (n == 0)   { return 0;       } // EOF
      if (n == -1)  { it++; continue; } // Skip invalid char
      std::memcpy(multibyte, it, size_t(n));
      multibyte[n] = '\0';
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
  attrset(colors.info_head);
  addstr(y, START_HEADING, heading);
}

inline void Info :: draw_tag(int y, const char* tag) noexcept {
  attrset(colors.info_tag);
  addstr(y, START_TAG, tag);
  attrset(colors.info_value);
  move(y, START_TAG_VALUE);
}

template<class Str>
inline void Info :: draw_info(int y, Str&& info) noexcept {
  attrset(colors.info_tag);
  addstr(y, START_INFO, info);
  attrset(colors.info_value);
  move(y, START_INFO_VALUE);
}

template<class Str, class Str1>
inline void Info :: draw_link(Str&& url, Str1&& title) noexcept {
  attrset(colors.url);
  UI::Pos start = cursorPos();
  addstr(title);
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
    *this << track.title();
    if (*track.remix())
      *this << " (" << track.remix() << ')';

    draw_tag(y++, "Artist");
    *this << track.artist();

    draw_tag(y++, "Number");
    printw("%02d", track.number());

    draw_tag(y++, "BPM");
    *this << track.bpm();

    draw_tag(y++, "Length");
    printw("%02d:%02d", player.length()/60, player.length()%60);

    // Album ==================================================================
    y++;
    draw_heading(y++, "Current album");

    draw_tag(y++, "Album");
    std::string album_url = album.url();
    Ektoplayer::url_expand(album_url, EKTOPLAZM_ALBUM_BASE_URL);
    draw_link(std::move(album_url), album.title());

    draw_tag(y++, "Artist");
    *this << album.artist();

    draw_tag(y++, "Date");
    char buf[32];
    *this << time_format(buf, "%B %d, %Y", album.date());

    draw_tag(y++, "Styles");
    const char* comma = "";
    for (auto id : iterate_set_bits(album.styles())) {
      *this << comma << track.table->db.styles[size_t(id+1)].name();
      comma = ", ";
    }

    draw_tag(y++, "Downloads");
    *this << album.download_count();

    draw_tag(y++, "Rating");
    printw("%2.2f%% (%d Votes)", album.rating(), album.votes());

    draw_tag(y++, "Cover");
    std::string cover_url = album.cover_url();
    Ektoplayer::url_expand(cover_url, EKTOPLAZM_COVER_BASE_URL, ".jpg");
    draw_link(std::move(cover_url), "Cover");

    // Description ============================================================
    y++;
    draw_heading(y++, "Description");
    move(y, START_TAG);
    MarkupParser markupParser(album.description());
    std::string linkURL, linkText;
    wchar_t c;
    while ((c = markupParser.next_char())) {
      if (markupParser.type & MarkupParser::LINK_TEXT)
        linkText.append(markupParser.multibyte);
      else if (markupParser.type & MarkupParser::LINK_URL)
        linkURL.append(markupParser.multibyte);
      else {
        attr_t attr = colors.info_value;
        if (markupParser.type & MarkupParser::BOLD)   attr |= A_BOLD;
        if (markupParser.type & MarkupParser::ITALIC) attr |= A_UNDERLINE;

        getyx(y, x);
        if      (x < START_TAG)                   move(y,   START_TAG);
        else if (x >= FORCE_LINE_BREAK)           move(y+1, START_TAG);
        else if (x >= TRY_LINE_BREAK && c == ' ') move(y+1, START_TAG-1);

        if (!linkText.empty() && !linkURL.empty()) {
          if (linkURL == "@") // Protected email, see updater.cpp
            linkURL = "Protected e-mail";
          else
            linkURL = "http://" + linkURL; // http?s stripped off, updater.cpp
          draw_link(std::move(linkURL), std::move(linkText));
        }

        attrset(attr);
        *this << c;
      }
    }

    y = getcury() + 2;
  }

  // Player ===================================================================
  draw_heading(y++, "Player");
  draw_info(y++, "Version");
  *this << VERSION;

  draw_info(y++, "Tracks in database");
  *this << database.tracks.size();

  draw_info(y++, "Albums in database");
  *this << database.albums.size();

  draw_info(y++, "Tracks in playlist");
  *this << mainwindow->playlist.list()->size();

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
    draw_info(y++, _clickable_urls[i].data.title);
    addstr(y++, START_INFO + 2, _clickable_urls[i].data.url);
  }

#if 0
  // Downloads ==============================================================
  y++;
  draw_heading(y++, "Downloads");
  for (const auto& dl : trackloader.downloads().downloads()) {
    addstr(y++, START_INFO, dl.download->last_url());
  }
#endif
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
      Programs::browser(event->data.url.c_str()).detach();
    return true;
  }
  return false;
}

