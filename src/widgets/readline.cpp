#include "readline.hpp"

namespace {

  // The C callbacks need to refer to a widget object
  ReadlineWidget *widgetInstance;

  // Readline should never attempt to read a char from STDIN.
  int readline_input_available_dummy(void) { return 0; }

  // We don't want any completion
  char *readline_completion_dummy(const char*, int) { return NULL; }

  void readline_forward_redisplay() {
    if (widgetInstance)
      widgetInstance->draw();
  }

  void readline_forward_finished(char *line) {
    std::string result;
    if (line) {
      result = line;
      free(line);
    }
    if (widgetInstance && widgetInstance->onFinish)
      widgetInstance->onFinish(std::move(result), !line);
  }

} // namespace (anonymous)

ReadlineWidget :: ReadlineWidget()
  : UI::Window({0,0}, {1,0})
{
  keypad(win, false);

  rl_initialize();
  rl_catch_signals             = 0;
  rl_catch_sigwinch            = 0;
  rl_deprep_term_function      = NULL;
  rl_prep_term_function        = NULL;
  rl_change_environment        = 0;
  rl_input_available_hook      = readline_input_available_dummy;
  rl_completion_entry_function = readline_completion_dummy;
  rl_redisplay_function        = readline_forward_redisplay;
  rl_callback_handler_install("", readline_forward_finished);
}

void ReadlineWidget :: layout(UI::Pos pos, UI::Size size) {
  size.height = 1;
  resize(size);
  setPos(pos);
}

void ReadlineWidget :: set_prompt(std::string prompt) noexcept {
  _prompt = std::move(prompt);
}

void ReadlineWidget :: draw() {
  clear();
  attrSet(0);
  mvAddStr(0, 0, _prompt);
  int x = getcurx(win);
  addStr(rl_line_buffer);
  mvwchgat(win, 0, x + rl_point, 1, A_STANDOUT, 0, NULL);
}

bool ReadlineWidget :: handle_key(int key) {
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
  w.onFinish = [](std::string s, bool isEOF) {
    if (isEOF)
      std::cerr << "EOF\n";
    else
      std::cerr << "Line: " << s << std::endl;
  };

  for (;;) {
    wtimeout(w.getWINDOW(), 1000);
    int key = wgetch(w.getWINDOW());
    if (key == ERR)
      continue;
    w.handle_key(key);
    w.draw();
    w.noutrefresh();
    doupdate();
  }

  TEST_END();
}
#endif
