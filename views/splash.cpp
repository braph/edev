#include "../ui.hpp"
#include "../theme.hpp"
#include "../colors.hpp"
#include "../common.hpp"
#include <cstdint>
#include <vector>

#define EKTOPLAZM_LOGO_WIDTH   78
#define EKTOPLAZM_LOGO_HEIGHT  10
const char EKTOPLAZM_LOGO[EKTOPLAZM_LOGO_HEIGHT][EKTOPLAZM_LOGO_WIDTH+1] = {
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

#define EKTOPLAZM_SIGNATURE_WIDTH  69
#define EKTOPLAZM_SIGNATURE_HEIGHT 4
const char EKTOPLAZM_SIGNATURE[EKTOPLAZM_SIGNATURE_HEIGHT][EKTOPLAZM_SIGNATURE_WIDTH+1] = {
  {"  ___                   _   _    _ _                  _   _          "},
  {" / __| ___ _  _ _ _  __| | | |  (_) |__  ___ _ _ __ _| |_(_)___ _ _  "},
  {" \\__ \\/ _ \\ || | ' \\/ _` | | |__| | '_ \\/ -_) '_/ _` |  _| / _ \\ ' \\ "},
  {" |___/\\___/\\_,_|_||_\\__,_| |____|_|_.__/\\___|_| \\__,_|\\__|_\\___/_||_|"},
};

#define BUBBLES_SIZE ARRAY_SIZE(BUBBLES)
const uint8_t BUBBLES[][2] = {{6,3}, {6,7}, {28,1}, {28,9}, {46,7}, {71,9}};

std::vector<short> colorFade_0       = {-1};

std::vector<short> logoFade_8        = {COLOR_BLUE};
std::vector<short> bubbleFade_8      = {COLOR_RED};
std::vector<short> signatureFade_8   = {COLOR_MAGENTA};

std::vector<short> logoFade_256      = {23,23,29,36,42,48,42,36,29,23};
std::vector<short> bubbleFade_256    = {168,167,161,161,161};
std::vector<short> signatureFade_256 = {99,105,111,117};

using namespace UI;
namespace Views {
class Splash : public UI::Window {
public:
  void draw() {
    werase(win);
    if (EKTOPLAZM_LOGO_HEIGHT > size.height || EKTOPLAZM_LOGO_WIDTH > size.width)
      return;

    // Assume no colors by default
    auto *logoFade      = &colorFade_0;
    auto *bubbleFade    = &colorFade_0;
    auto *signatureFade = &colorFade_0;

    if (Theme::current == 256) {
      logoFade       = &logoFade_256;
      bubbleFade     = &bubbleFade_256;
      signatureFade  = &signatureFade_256;
    } else if (Theme::current == 8) {
      logoFade       = &logoFade_8;
      bubbleFade     = &bubbleFade_8;
      signatureFade  = &signatureFade_8;
    }

    int w_center = size.width / 2;
    int h_center = size.height / 2;
    int left_pad = w_center - (EKTOPLAZM_LOGO_WIDTH / 2);
    int top_pad  = h_center - (EKTOPLAZM_LOGO_HEIGHT / 2);
    bool draw_signature = false;

    if (EKTOPLAZM_LOGO_HEIGHT + EKTOPLAZM_SIGNATURE_HEIGHT + 3 <= size.height) {
      top_pad -= 3;
      draw_signature = true;
    }

#define FG(COLOR) UI::Colors::set("", COLOR, -1, 0)
    for (unsigned i = 0; i < EKTOPLAZM_LOGO_HEIGHT; ++i) {
      wattrset(win, FG(proportionalGet(*logoFade, EKTOPLAZM_LOGO_HEIGHT, i)));
      mvwaddstr(win, top_pad + i, left_pad, EKTOPLAZM_LOGO[i]);
    }

    for (unsigned i = 0; i < BUBBLES_SIZE; ++i) {
      int x = BUBBLES[i][0];
      int y = BUBBLES[i][1];
      wattrset(win, FG(proportionalGet(*bubbleFade, EKTOPLAZM_LOGO_HEIGHT, y - 1)));
      mvwaddch(win, top_pad + y - 1, left_pad + x + 1, '_');
      wattrset(win, FG(proportionalGet(*bubbleFade, EKTOPLAZM_LOGO_HEIGHT, y)));
      mvwaddstr(win, top_pad + y, left_pad + x, "(_)");
    }

    if (! draw_signature) return;

    top_pad += EKTOPLAZM_LOGO_HEIGHT + 2;
    left_pad = w_center - (EKTOPLAZM_SIGNATURE_WIDTH / 2);

    for (unsigned i = 0; i < EKTOPLAZM_SIGNATURE_HEIGHT; ++i) {
      wmove(win, top_pad + i, left_pad);
      for (unsigned j = 0; j < EKTOPLAZM_SIGNATURE_WIDTH; ++j) {
        wattrset(win, FG(proportionalGet2(*signatureFade, EKTOPLAZM_SIGNATURE_WIDTH, j)));
        waddch(win, EKTOPLAZM_SIGNATURE[i][j]);
      }
    }
  }
};
}

#if TEST_SPLASH
#include "../test.hpp"
#include "../colors.hpp"
int main() {
  TEST_BEGIN

  initscr();
  start_color();
  use_default_colors();
  UI::Color::init();
  UI::Colors::init();
  UI::Attribute::init();
  Theme::current = 256;
  
  Widget *s = new Views::Splash;
  s->layout({10,10}, {20,80});
  s->draw();
  s->refresh();
  wgetch(s->active_win());

  TEST_END
}
#endif
