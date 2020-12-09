#include "progressbar.hpp"

#include "../theme.hpp"
#include "../config.hpp"
#include "../ui/colors.hpp"
#include <lib/spanview.hpp>
#include <lib/arrayview.hpp>

static const short fading_0[]   = {-1};
static const short fading_8[]   = {COLOR_BLUE};
static const short fading_256[] = {25,26,27,32,39,38,44,44,45,51,87,159,195};

using namespace UI;
using namespace Views;

/* The progressbar is drawed only *once* in layout().
 * It is positioned in the right place later by percent() */

ProgressBar :: ProgressBar()
: UI::Pad({0,0}, {1,COLS*2})
{
}

void ProgressBar :: draw() {
}

void ProgressBar :: layout(Pos pos, Size size) {
  size.height = 1;
  if (this->pos != pos || this->size != size) {
    this->pos = pos;
    this->size = size;
    wresize(win, size.height, size.width * 2);
    mvwin(win, pos.y, pos.x);
  }

  moveCursor(0, 0);

  ArrayView<const short> fading(fading_0);

  if (current_theme == THEME_8)
    fading = fading_8;
  else if (current_theme == THEME_256)
    fading = fading_256;

  size_t i;
  const size_t width = size_t(size.width);
  auto fader = SpanView<ArrayView<const short>>(fading);

  for (i = 0; i < width; ++i) {
    attrSet(UI::Colors::set(fader.get(width, i), -1));
    *this << Config::progressbar_progress_char;
  }
  
  attrSet(colors.progressbar_rest);
  for (; i < width * 2; ++i)
    *this << Config::progressbar_rest_char;
}

void ProgressBar :: percent(float percent) noexcept {
  pad_mincol = size.width - (percent * size.width);
}

bool ProgressBar :: handle_mouse(MEVENT& m) {
  if (m.y == pos.y /* + size.height - 1 */) {
    if (percent_changed)
      percent_changed(float(m.x + 1) / size.width);
    return true;
  }
  return false;
}

#ifdef TEST_PROGRESSBAR
#include <lib/test.hpp>
int main() {
  TEST_BEGIN();
  NCURSES_INIT();

  Config::init();

  for (int colors : {0, 8, 256}) {
    if (colors > COLORS)
      break;
    Theme::loadThemeByColors(colors); // TODO
  
    ProgressBar b;
    b.layout({0,0}, {LINES,COLS});
    for (;;)
      for (float f = 0.0; f < 1; f += 0.01) {
        b.percent(f);
        b.noutrefresh();
        doupdate();
        usleep(1000 * 30);
      }
  }

  TEST_END();
}
#endif
