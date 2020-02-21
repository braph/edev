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

/* The progressbar is drawed only *once* in layout().
 * It is positioned in the right place later by setPercent() */

void ProgressBar :: draw() {
}

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

  int i;
  wchar_t c;

  c = Config::progressbar_progress_char;
  for (i = 0; i < size.width; ++i) {
    wattron(win, UI::Colors::set(proportionalGet(fade, fade_size, size.width, i), -1, 0));
    waddnwstr(win, &c, 1);
  }
  
  c = Config::progressbar_rest_char;
  wattrset(win, Theme::get(Theme::PROGRESSBAR_REST));
  for (; i < size.width*2; ++i)
    waddnwstr(win, &c, 1);
}

void ProgressBar :: setPercent(float percent) {
  pad_mincol = size.width - (percent * size.width);
}

bool ProgressBar :: handleMouse(MEVENT& m) {
  if (m.y == pos.y /* + size.height - 1 */) {
    if (percentChanged)
      percentChanged(static_cast<float>(m.x+1) / size.width);
    return true;
  }
  return false;
}

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
