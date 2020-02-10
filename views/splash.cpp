#include "splash.hpp"

#include "../theme.hpp"
#include "../colors.hpp"
#include "../common.hpp"

#define LOGO_WIDTH   78
#define LOGO_HEIGHT  10
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
static const char SIGNATURE[SIGNATURE_HEIGHT][SIGNATURE_WIDTH+1] = {
  {"  ___                   _   _    _ _                  _   _          "},
  {" / __| ___ _  _ _ _  __| | | |  (_) |__  ___ _ _ __ _| |_(_)___ _ _  "},
  {" \\__ \\/ _ \\ || | ' \\/ _` | | |__| | '_ \\/ -_) '_/ _` |  _| / _ \\ ' \\ "},
  {" |___/\\___/\\_,_|_||_\\__,_| |____|_|_.__/\\___|_| \\__,_|\\__|_\\___/_||_|"},
};

#define BUBBLES_SIZE ARRAY_SIZE(BUBBLES)
static const char BUBBLES[][2] = {{6,3}, {6,7}, {28,1}, {28,9}, {46,7}, {71,9}};

static const short colorFade_0[]       = {-1};

static const short logoFade_8[]        = {COLOR_BLUE};
static const short bubbleFade_8[]      = {COLOR_RED};
static const short signatureFade_8[]   = {COLOR_MAGENTA};

static const short logoFade_256[]      = {23,23,29,36,42,48,42,36,29,23};
static const short bubbleFade_256[]    = {168,167,161,161,161};
static const short signatureFade_256[] = {99,105,111,117};

using namespace UI;
using namespace Views;
void Splash :: draw() {
  werase(win);
  if (LOGO_HEIGHT > size.height || LOGO_WIDTH > size.width)
    return;

  // Assume no colors by default
  const short *logoFade      = colorFade_0;
  const short *bubbleFade    = colorFade_0;
  const short *signatureFade = colorFade_0;
  size_t logoFade_size       = ARRAY_SIZE(colorFade_0);
  size_t bubbleFade_size     = ARRAY_SIZE(colorFade_0);
  size_t signatureFade_size  = ARRAY_SIZE(colorFade_0);

  if (Theme::current == 256) {
    logoFade            = logoFade_256;
    bubbleFade          = bubbleFade_256;
    signatureFade       = signatureFade_256;
    logoFade_size       = ARRAY_SIZE(logoFade_256);
    bubbleFade_size     = ARRAY_SIZE(bubbleFade_256);
    signatureFade_size  = ARRAY_SIZE(signatureFade_256);
  } else if (Theme::current == 8) {
    logoFade            = logoFade_8;
    bubbleFade          = bubbleFade_8;
    signatureFade       = signatureFade_8;
    logoFade_size       = ARRAY_SIZE(logoFade_8);
    bubbleFade_size     = ARRAY_SIZE(bubbleFade_8);
    signatureFade_size  = ARRAY_SIZE(signatureFade_8);
  }

  int w_center = size.width / 2;
  int h_center = size.height / 2;
  int left_pad = w_center - (LOGO_WIDTH / 2);
  int top_pad  = h_center - (LOGO_HEIGHT / 2);
  bool draw_signature = false;

  if (LOGO_HEIGHT + SIGNATURE_HEIGHT + 3 <= size.height) {
    top_pad -= 3;
    draw_signature = true;
  }

#define FG(COLOR) UI::Colors::set(COLOR, -1, 0)
  for (size_t i = 0; i < LOGO_HEIGHT; ++i) {
    wattrset(win, FG(proportionalGet(logoFade, logoFade_size, LOGO_HEIGHT, i)));
    mvwaddstr(win, top_pad + i, left_pad, LOGO[i]);
  }

  for (size_t i = 0; i < BUBBLES_SIZE; ++i) {
    const int x = BUBBLES[i][0];
    const int y = BUBBLES[i][1];
    wattrset(win, FG(proportionalGet(bubbleFade, bubbleFade_size, LOGO_HEIGHT, y - 1)));
    mvwaddch(win, top_pad + y - 1, left_pad + x + 1, '_');
    wattrset(win, FG(proportionalGet(bubbleFade, bubbleFade_size, LOGO_HEIGHT, y)));
    mvwaddstr(win, top_pad + y, left_pad + x, "(_)");
  }

  if (! draw_signature) return;

  top_pad += LOGO_HEIGHT + 2;
  left_pad = w_center - (SIGNATURE_WIDTH / 2);

  for (size_t i = 0; i < SIGNATURE_HEIGHT; ++i) {
    wmove(win, top_pad + i, left_pad);
    for (size_t j = 0; j < SIGNATURE_WIDTH; ++j) {
      wattrset(win, FG(proportionalGet2(signatureFade, signatureFade_size, SIGNATURE_WIDTH, j)));
      waddch(win, SIGNATURE[i][j]);
    }
  }
}

#if TEST_SPLASH
#include "../test.hpp"
int main() {
  TEST_BEGIN();
  NCURSES_INIT();

  Widget *s = new Views::Splash;
  s->layout({10,10}, {20,80});

  for (auto colors : {0,8,256}) {
    Theme::current = colors;
    s->draw();
    s->refresh();
    wgetch(s->active_win());
  }

  TEST_END();
}
#endif
