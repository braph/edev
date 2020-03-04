// Splash with pads....
#include "splash.hpp"

#include "../theme.hpp"
#include "../colors.hpp"
#include "../common.hpp"
#include "../generic.hpp"

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
static const char BUBBLES[][2] = {{6,3}, {6,7}, {28,1}, {28,9}, {46,7}, {71,9}};

static const short colorFading_0[]       = {-1};

static const short logoFading_8[]        = {COLOR_BLUE};
static const short bubbleFading_8[]      = {COLOR_RED};
static const short signatureFading_8[]   = {COLOR_MAGENTA};

static const short logoFading_256[]      = {23,23,29,36,42,48,42,36,29,23};
static const short bubbleFading_256[]    = {168,167,161,161,161};
static const short signatureFading_256[] = {99,105,111,117};

using namespace UI;
using namespace Views;
#define setFG(COLOR) UI::Colors::set(COLOR, -1, 0)

Splash :: Splash() : UI::Pad() {
  wresize(win, LOGO_HEIGHT + 3 + SIGNATURE_HEIGHT, LOGO_WIDTH); //TODO: move to constructor

  ArrayView<const short> logoFading(colorFading_0);
  ArrayView<const short> bubbleFading(colorFading_0);
  ArrayView<const short> signatureFading(colorFading_0);

  if (Theme::current == 256) {
    logoFading            = logoFading_256;
    bubbleFading          = bubbleFading_256;
    signatureFading       = signatureFading_256;
  } else if (Theme::current == 8) {
    logoFading            = logoFading_8;
    bubbleFading          = bubbleFading_8;
    signatureFading       = signatureFading_8;
  }

  SpanView<ArrayView<const short>> fader = logoFading;
  for (int i = 0; i < LOGO_HEIGHT; ++i) {
    wattrset(win, setFG(fader.get(LOGO_HEIGHT, unsigned(i))));
    mvwaddstr(win, i, 0, LOGO[i]);
  }

  fader = bubbleFading;
  for (int i = 0; i < BUBBLES_SIZE; ++i) {
    const int x = BUBBLES[i][0];
    const int y = BUBBLES[i][1];
    mvwaddch(win, y - 1, x + 1,
        '_' | setFG(fader.get(LOGO_HEIGHT, unsigned(y - 1))));
    wattrset(win, setFG(fader.get(LOGO_HEIGHT, unsigned(y))));
    mvwaddstr(win, y, x, "(_)");
  }

  const int w_center = LOGO_WIDTH / 2;
  const int h_center = LOGO_HEIGHT / 2;
  const int left_pad = w_center - (SIGNATURE_WIDTH / 2);
  const int top_pad  = LOGO_HEIGHT + 3;

  fader = signatureFading;
  for (int i = 0; i < SIGNATURE_HEIGHT; ++i) {
    moveCursor(top_pad + i, left_pad);
    for (unsigned j = 0; j < SIGNATURE_WIDTH; ++j)
      waddch(win, SIGNATURE[i][j] | setFG(fader.get2(SIGNATURE_WIDTH, j)));
  }
}

void Splash :: layout(UI::Pos pos, UI::Size size) {
  this->pos = pos;
  this->size = size;
}

void Splash :: noutrefresh() {
  int pad_height = LOGO_HEIGHT; + 3 + SIGNATURE_HEIGHT;
  int pad_width = LOGO_WIDTH;

  if (pad_height > size.height || pad_width > size.width)
    return;

  if (pad_height + 3 + SIGNATURE_HEIGHT <= size.height)
    pad_height += 3 + SIGNATURE_HEIGHT;

  int screen_y = pos.y + size.height / 2 - pad_height / 2;
  int screen_x = pos.x + size.width / 2 - pad_width / 2;

  pnoutrefresh(win, 0, 0, screen_y, screen_x, screen_y + pad_height, screen_x + pad_width);
}

void Splash :: draw() {
}

#ifdef TEST_SPLASH
#include "../test.hpp"
int main() {
  TEST_BEGIN();
  NCURSES_INIT();

  for (auto colors : {0,8,256}) {
    Theme::current = colors;
    Views::Splash s;
    s.layout({10,10}, {40,80}); // TODO
    s.draw();
    s.noutrefresh();
    doupdate();
    wgetch(s.active_win());
  }

  TEST_END();
}
#endif
