#include "progressbar.hpp"

#include "../theme.hpp"
#include "../config.hpp"
#include "../common.hpp"
#include "../colors.hpp"

static const short fading_0[]   = {-1};
static const short fading_8[]   = {COLOR_BLUE};
static const short fading_256[] = {25,26,27,32,39,38,44,44,45,51,87,159,195};

using namespace UI;
using namespace Views;

void ProgressBar :: setPercent(float percent) {
  pad_mincol = size.width - (percent * size.width);
}

void ProgressBar :: draw() {
}

/* We draw the progress bar once inside a larger pad and just move it later */
void ProgressBar :: layout(Pos pos, Size size) {
  size.height = 1;
  this->pos = pos;
  this->size = size;

  const short *fade = fading_0;
  size_t fade_size  = ARRAY_SIZE(fading_0);

  if (Theme::current == 256) {
    fade      = fading_256;
    fade_size = ARRAY_SIZE(fading_256);
  }
  else if (Theme::current == 8) {
    fade      = fading_8;
    fade_size = ARRAY_SIZE(fading_8);
  }

  wresize(win, 1, size.width * 2);
  mvwin(win, pos.y, pos.x);
  wmove(win, 0, 0);

#define FG(COLOR) UI::Colors::set(COLOR, -1, 0)
  char c = Config::progressbar_progress_char;
  for (int i = 0; i < size.width; ++i) {
    waddch(win, c|FG(proportionalGet(fade, fade_size, size.width, i)));
  }

  whline(win,
    Config::progressbar_rest_char|Theme::get(Theme::PROGRESSBAR_REST), 1337);
}

#if TODO
void on_click(int button, int y, int x) {
  clicked(button, x / size.width); // mevent.x - pad_mincol? size.width - 1?
  // progress_width = x .... callback will do this, i think?
}
#endif

#if TEST_PROGRESSBAR
#include "../test.hpp"
int main() {
  TEST_BEGIN();
  NCURSES_INIT();

  Config::init();
  Theme::loadTheme(256);
  
  ProgressBar b;
  b.layout({0,0}, {LINES,COLS});
  for (;;)
    for (float f = 0.0; f < 1; f += 0.01) {
      b.setPercent(f);
      b.noutrefresh();
      doupdate();
      usleep(1000 * 30);
    }

  TEST_END();
}
#endif
