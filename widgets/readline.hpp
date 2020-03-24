#ifndef WIDGETS_READLINE_HPP
#define WIDGETS_READLINE_HPP

#include "../ui.hpp"

#include <readline/readline.h>

#include <string>
#include <functional>

class ReadlineWidget : public UI::Window {
public:
  ReadlineWidget();

  using onFinishFunction = std::function<void(std::string, bool)>;
  onFinishFunction onFinish;

  void draw();
  void layout(UI::Pos, UI::Size);
  bool handleKey(int);
  void setPrompt(std::string);

private:
  std::string _prompt;
};

#endif
