#include "readline.hpp"

// Readline should never attempt to read a char from STDIN.
static int readline_input_available_dummy(void) { return 0; }

// We don't want any completion
static char *readline_completion_dummy(const char*, int) { return NULL; }

// The C callbacks need to refer to a widget object
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

void ReadlineWidget :: layout(UI::Pos pos, UI::Size size) {
  size.height = 1;
  if (size != this->size) {
    this->size = size;
    wresize(win, size.height, size.width);
  }
  if (pos != this->pos) {
    this->pos = pos;
    mvwin(win, pos.y, pos.x);
  }
}

void ReadlineWidget :: setPrompt(const std::string& s) {
  prompt = s;
}

void ReadlineWidget :: draw() {
  clear();
  attrSet(0);
  mvAddStr(0, 0, prompt.c_str());
  int x = getcurx(win);
  addStr(rl_line_buffer);
  mvwchgat(win, 0, x + rl_point, 1, A_STANDOUT, 0, NULL);
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
  NCURSES_INIT();

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
    wtimeout(w.getWINDOW(), 1000);
    int key = wgetch(w.getWINDOW());
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
