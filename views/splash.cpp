#include "../ui.hpp"
#include "../theme.hpp"
#include "../colorfader.hpp"
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
const uint8_t BUBBLES[][2] = { {6,3}, {6,7}, {28,1}, {28,9}, {46,7}, {71,9} };

std::vector<int> colorFader_0       = {-1};

std::vector<int> logoFader_8        = {COLOR_BLUE};
std::vector<int> bubbleFader_8      = {COLOR_RED};
std::vector<int> signatureFader_8   = {COLOR_MAGENTA};

std::vector<int> logoFader_256      = {23,23,29,36,42,48,42,36,29,23};
std::vector<int> bubbleFader_256    = {168,167,161,161,161};
std::vector<int> signatureFader_256 = {99,105,111,117};

class Splash : public UI::Window {
  public:
    Splash() { }

    void draw() {
      werase(win);
      if (EKTOPLAZM_LOGO_HEIGHT >= size.height || EKTOPLAZM_LOGO_WIDTH >= size.width)//>=?TODO
        return;

      // Assume no colors by default
      std::vector<int> *logoFader      = &colorFader_0;
      std::vector<int> *bubbleFader    = &colorFader_0;
      std::vector<int> *signatureFader = &colorFader_0;

      if (Ektoplayer::Theme::current == 256) {
        logoFader       = &logoFader_256;
        bubbleFader     = &bubbleFader_256;
        signatureFader  = &signatureFader_256;
      } else if (Ektoplayer::Theme::current == 8) {
        logoFader       = &logoFader_8;
        bubbleFader     = &bubbleFader_8;
        signatureFader  = &signatureFader_8;
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

      for (unsigned i = 0; i < EKTOPLAZM_LOGO_HEIGHT; ++i) {
        wattrset(win, UI::ColorFader::fade2(*logoFader, i, EKTOPLAZM_LOGO_HEIGHT));
        mvwaddstr(win, top_pad + i, left_pad, EKTOPLAZM_LOGO[i]);
      }

      // @bubble_fade FADE not fade2! on EKTOPLAZM_LOGO_HEIGHT
      for (unsigned i = 0; i < BUBBLES_SIZE; ++i) {
        int x = BUBBLES[i][0];
        int y = BUBBLES[i][1];
        wattrset(win, UI::ColorFader::fade(*bubbleFader, y - 1, EKTOPLAZM_LOGO_HEIGHT));
        mvwaddstr(win, top_pad + y - 1, left_pad + x + 1, "_");
        wattrset(win, UI::ColorFader::fade(*bubbleFader, y, EKTOPLAZM_LOGO_HEIGHT));
        mvwaddstr(win, top_pad + y, left_pad + x, "(_)");
      }

      if (! draw_signature) return;

      top_pad += EKTOPLAZM_LOGO_HEIGHT + 2;
      left_pad = w_center - (EKTOPLAZM_SIGNATURE_WIDTH / 2);

      for (unsigned i = 0; i < EKTOPLAZM_SIGNATURE_HEIGHT; ++i) {
        wmove(win, top_pad + i, left_pad);
        for (unsigned j = 0; j < EKTOPLAZM_SIGNATURE_WIDTH; ++j) {
          wattrset(win, UI::ColorFader::fade2(*signatureFader, j, EKTOPLAZM_SIGNATURE_WIDTH));
          waddch(win, EKTOPLAZM_SIGNATURE[i][j]);
        }
      }
    }
};

#if TEST_SPLASH
#include <iostream>
#include <unistd.h>
#include "../colors.hpp"
int main() {
  initscr();
  start_color();
  use_default_colors();
  UI::Color::init();
  UI::Colors::init();
  UI::Attribute::init();
  Ektoplayer::Theme::current = 256;
  
  try {
    Splash s;
    s.draw();
    s.refresh();
    pause();
  } catch (const std::exception &e) {
    std::cout << e.what() << std::endl;
    return 1;
  }
  return 0;
}
#endif
