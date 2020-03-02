#ifndef _READLINE_WIDGET_CPP
#define _READLINE_WIDGET_CPP

#include "../ui.hpp"

#include <readline/readline.h>

#include <string>
#include <functional>

class ReadlineWidget : public UI::Window {
public:
  ReadlineWidget();
  void draw();
  bool handleKey(int);
  std::function<void(const std::string&, bool)> onFinish;
  void setPrompt(const std::string&);
private:
  std::string prompt;
};

#endif

// Readline should never attempt to read a char from STDIN.
static int readline_input_available_dummy(void) { return 0; }

// We don't want any completion
static char *readline_completion_dummy(const char*, int) { return NULL; }

// The C callbacks need to refer to the widget object
static ReadlineWidget *widgetInstance;

static void readline_forward_redisplay() {
  if (widgetInstance)
    widgetInstance->draw();
}

static void readline_forward_finished(char *line) {
  std::string result;
  if (line) {
    result = line;
    free(line);
  }
  if (widgetInstance)
    if (widgetInstance->onFinish)
      widgetInstance->onFinish(result, static_cast<bool>(line));
}

ReadlineWidget :: ReadlineWidget() : UI::Window() {
  keypad(win, false);

  rl_initialize();
  rl_catch_signals = 0;
  rl_catch_sigwinch = 0;
  rl_deprep_term_function = NULL;
  rl_prep_term_function = NULL;
  rl_change_environment = 0;
  rl_input_available_hook = readline_input_available_dummy;
  rl_completion_entry_function = readline_completion_dummy;
  rl_redisplay_function = readline_forward_redisplay;
  rl_callback_handler_install("", readline_forward_finished);
}

void ReadlineWidget :: setPrompt(const std::string& s) {
  prompt = s;
}

void ReadlineWidget :: draw() {
  wclear(win);
  mvwaddstr(win, 0, 0, prompt.c_str());
  int x = getcurx(win);
  waddstr(win, rl_line_buffer);
  wmove(win, 0, x + rl_point);
}

bool ReadlineWidget :: handleKey(int key) {
  widgetInstance = this;
  wtimeout(win, 0);
  rl_stuff_char(key);
  while ((key = wgetch(win)) != ERR)
    rl_stuff_char(key);
  rl_callback_read_char();
  return true;
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
  w.layout({0,0}, {LINES, COLS});
  w.setPrompt("> ");
  w.onFinish = [](const std::string& s, bool notEOF) {
    if (notEOF)
      std::cerr << "Line: " << s << std::endl;
    else
      std::cerr << "EOF" << std::endl;
  };

  for (;;) {
    wtimeout(w.active_win(), 1000);
    int key = wgetch(w.active_win());
    if (key == ERR)
      continue;
    w.handleKey(key);
    w.draw();
    w.noutrefresh();
    doupdate();
  }

  TEST_END();
}
#endif
