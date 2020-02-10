#include "../ui.hpp"
#include "../theme.hpp"
#include "../config.hpp"
#include "../common.hpp"
#include "../colors.hpp"
#include <vector>

std::vector<short> fading_256 = {25,26,27,32,39,38,44,44,45,51,87,159,195};
std::vector<short> fading_8   = {COLOR_BLUE};
std::vector<short> fading_0   = {-1};

namespace Ektoplayer {
  namespace Views {
    //typedef void (*clicked)(percent);

    class ProgressBar : public UI::Pad {
      public:
        void setPercent(float);
        void layout();
    };
  }
}

using namespace Ektoplayer;
using namespace Ektoplayer::Views;

void ProgressBar :: setPercent(float percent) {
  int new_width = size.width - (percent * size.width);
  if (new_width != pad_mincol) {
    pad_mincol = new_width;
    // need refresh();
  }
}

/* We draw the progress bar once inside the pad.  Everything else */
void ProgressBar :: layout() {
  auto *fade = &fading_0;
  if (Theme::current == 256)
    fade = &fading_256;
  else if (Theme::current == 8)
    fade = &fading_8;

  wresize(win, 1, size.width * 2); // self.pad_size
  wmove(win, 0, 0);

#define FG(COLOR) UI::Colors::set(COLOR, -1, 0)
  char c = Config::progressbar_progress_char;
  for (unsigned i = 0; i < size.width; ++i) {
    wattrset(win, FG(proportionalGet(*fade, size.width, i)));
    waddch(win, c);
  }

  wattrset(win, Theme::get(Theme::PROGRESSBAR_REST));
  whline(win, Config::progressbar_rest_char, 1337);
}

#if foo
void on_click(int button, int y, int x) {
  clicked(button, x / size.width); // mevent.x - pad_mincol? size.width - 1?
  // progress_width = x .... callback will do this, i think?
}
#endif

#if TEST_PROGRESSBAR
#include <iostream>
#include <unistd.h>
int main() {
  initscr();
  start_color();
  use_default_colors();
  Config::init();
  Theme::loadTheme(256);
  
  try {
    ProgressBar b;
    b.setSize(LINES, COLS);
    b.layout();
    b.refresh();
    for (float f = 0.0; f < 1; f += 0.01) {
      b.setPercent(f);
      b.refresh();
      usleep(1000 * 50);
    }
    std::cout << "\n Done. Press ^C" << std::endl;
    pause();
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
    return 1;
  }
  return 0;
}
#endif
