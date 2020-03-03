#ifndef _READLINE_WIDGET_CPP
#define _READLINE_WIDGET_CPP

#include "../ui.hpp"

#include <readline/readline.h>

#include <string>
#include <functional>

class ReadlineWidget : public UI::Window {
public:
  ReadlineWidget();

  using onFinishFunction = std::function<void(const std::string&, bool)>;
  onFinishFunction onFinish;

  void draw();
  void layout(UI::Pos, UI::Size);
  bool handleKey(int);
  void setPrompt(const std::string&);
private:
  std::string prompt;
};

#endif
