#ifndef _READLINE_WIDGET_CPP
#define _READLINE_WIDGET_CPP

#include "../ui.hpp"

#include <readline/readline.h>

class ReadlineWidget : public UI::Window {
public:
  void draw();
  void _getch();
private:
  std::string prompt;
};

#endif

void ReadlineWidget :: draw() {
  werase(win);
  mvwprintw(win, 0, 0, ">>%s%s", rl_display_prompt, rl_line_buffer);
}

static ReadlineWidget *readlineWidget;

static int  input;
static bool input_avail;

static int readline_getc(FILE*_) {
  input_avail = false;
  return input;
}

static int readline_input_avail(void) {
  return input_avail;
}

static void readline_redisplay() {
  if (readlineWidget)
    readlineWidget->draw();
}

static void got_command(char *line) {
  //if (line)
  //  throw std::runtime_error(line);
}

static char *readline_NULL_complete(const char *_, int __) {
  return NULL;
}

static void initReadline() {
  // Let ncurses do all terminal and signal handling
  rl_catch_signals = 0;
  rl_catch_sigwinch = 0;
  rl_deprep_term_function = NULL;
  rl_prep_term_function = NULL;
  rl_change_environment = 0;
  rl_getc_function = readline_getc;
  rl_input_available_hook = readline_input_avail;
  rl_redisplay_function = readline_redisplay;
  rl_completion_entry_function = readline_NULL_complete;
  rl_callback_handler_install("> ", got_command);
}

void ReadlineWidget :: _getch() {
  readlineWidget = this;

  int c = wgetch(win);

  switch (c) {
    case KEY_RESIZE:
      //resize();
      break;

    case '\f': // ^L
      clearok(curscr, TRUE);
      //resize();
      break;

    default:
      input = c;
      input_avail = true;
      rl_callback_read_char();
  }
}

#ifdef TEST_READLINE
#include "../test.hpp"
int main() {
  TEST_BEGIN();

  initscr();
  cbreak();
  noecho();
  nonl();

  ReadlineWidget w;

  initReadline();

  for (;;) {
    w._getch();
    w.draw();
    w.noutrefresh();
    doupdate();
  }

  TEST_END();
}
#endif
