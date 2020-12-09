#include "splash.hpp"

#include "../theme.hpp"
#include "../ui/colors.hpp"
#include <lib/spanview.hpp>
#include <lib/arrayview.hpp>

#define LOGO_WIDTH  78
#define LOGO_HEIGHT 10
static const char LOGO[LOGO_HEIGHT][LOGO_WIDTH+1] = {
  {"   ____    _   _     _             ____    _       _____   ______   _ ___ ___"},
  {"  /  __)  ; | | ;   | |           |  _ \\  | |     /___  \\ |____  | | '_  `_  \\"},
  {" /  /     | | | |  _| |__   ___   | | | | | |         | |      | | | | | | | |"},
  {"(  (      | | | | |_   __) / _ \\  | | | | | |      ___| |      / ; | | | | | |"},
  {" \\  \\_    | | | |   | |   | | | | | | | | | |     /  _| |     / /  | | | | | |"},
  {"  )  _)   | | | |   | |   | | | | | |_| ; | |     | | | |    / /   | | | | | |"},
  {" /  /     | |_|/    | |   | |_| | |  __/  | |     | | | |   / /    | | | | | |"},
  {"(  (      |  _{     | |    \\___/  | |     | |     | | | |  / /     | | |_| | |"},
  {" \\  \\__   | | |\\    | |__         | |     | :___  | |_| | : /____  | |     | |"},
  {"  \\____)  |_| |_|   \\___/         |_|     \\____/  \\_____| |______| |_|     |_|"},
};

#define SIGNATURE_WIDTH  69
#define SIGNATURE_HEIGHT 4
static const unsigned char SIGNATURE[SIGNATURE_HEIGHT][SIGNATURE_WIDTH+1] = {
  {"  ___                   _   _    _ _                  _   _          "},
  {" / __| ___ _  _ _ _  __| | | |  (_) |__  ___ _ _ __ _| |_(_)___ _ _  "},
  {" \\__ \\/ _ \\ || | ' \\/ _` | | |__| | '_ \\/ -_) '_/ _` |  _| / _ \\ ' \\ "},
  {" |___/\\___/\\_,_|_||_\\__,_| |____|_|_.__/\\___|_| \\__,_|\\__|_\\___/_||_|"},
};

#define BUBBLES_SIZE 6
static const char BUBBLES[BUBBLES_SIZE][2] = {{6,3}, {6,7}, {28,1}, {28,9}, {46,7}, {71,9}};

static const short colorFading_0[]       = {-1};

static const short logoFading_8[]        = {COLOR_BLUE};
static const short bubbleFading_8[]      = {COLOR_RED};
static const short signatureFading_8[]   = {COLOR_MAGENTA};

static const short logoFading_256[]      = {23,23,29,36,42,48,42,36,29,23};
static const short bubbleFading_256[]    = {168,167,161,161,161};
static const short signatureFading_256[] = {99,105,111,117};

using namespace UI;
using namespace Views;
#define setFG(COLOR) UI::Colors::set(COLOR, -1)

void Splash :: draw() {
  clear();
  if (LOGO_HEIGHT > size.height || LOGO_WIDTH > size.width)
    return;

  // Assume no colors by default
  auto logoFading      = ArrayView<const short>(colorFading_0);
  auto bubbleFading    = ArrayView<const short>(colorFading_0);
  auto signatureFading = ArrayView<const short>(colorFading_0);

  if (current_theme == THEME_8) {
    logoFading         = logoFading_8;
    bubbleFading       = bubbleFading_8;
    signatureFading    = signatureFading_8;
  }
  else if (current_theme == THEME_256) {
    logoFading         = logoFading_256;
    bubbleFading       = bubbleFading_256;
    signatureFading    = signatureFading_256;
  }

  SpanView<ArrayView<const short>> fader;
  int w_center = size.width / 2;
  int h_center = size.height / 2;
  int left_pad = w_center - (LOGO_WIDTH / 2);
  int top_pad  = h_center - (LOGO_HEIGHT / 2);
  bool draw_signature = false;

  if (LOGO_HEIGHT + SIGNATURE_HEIGHT + 3 <= size.height) {
    top_pad -= 3;
    draw_signature = true;
  }

  fader = logoFading;
  for (int i = 0; i < LOGO_HEIGHT; ++i) {
    attrSet(setFG(fader.get(LOGO_HEIGHT, unsigned(i))));
    addstr(top_pad + i, left_pad, LOGO[i]);
  }

  fader = bubbleFading;
  for (int i = 0; i < BUBBLES_SIZE; ++i) {
    const int x = BUBBLES[i][0];
    const int y = BUBBLES[i][1];
    addch(top_pad + y - 1, left_pad + x + 1, '_' | setFG(fader.get(LOGO_HEIGHT, size_t(y - 1))));
    attrSet(setFG(fader.get(LOGO_HEIGHT, size_t(y))));
    addstr(top_pad + y, left_pad + x, "(_)");
  }

  if (! draw_signature)
    return;

  top_pad += LOGO_HEIGHT + 2;
  left_pad = w_center - (SIGNATURE_WIDTH / 2);

  fader = signatureFading;
  for (int i = 0; i < SIGNATURE_HEIGHT; ++i) {
    moveCursor(top_pad + i, left_pad);
    for (unsigned j = 0; j < SIGNATURE_WIDTH; ++j)
      addch(SIGNATURE[i][j] | setFG(fader.get2(SIGNATURE_WIDTH, j)));
  }
}

#ifdef TEST_SPLASH
#include <lib/test.hpp>
int main() {
  TEST_BEGIN();
  NCURSES_INIT();

  Widget *s = new Views::Splash;
  s->layout({10,10}, {20,80});

  for (auto theme : {Theme::ThemeID::THEME_MONO, Theme::ThemeID::THEME_8, Theme::ThemeID::THEME_256}) { // TODO
    Theme::current = theme;
    s->draw();
    s->noutrefresh();
    doupdate();
    wgetch(s->getWINDOW());
  }

  TEST_END();
}
#endif
