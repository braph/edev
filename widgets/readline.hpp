#ifndef WIDGETS_READLINE_HPP
#define WIDGETS_READLINE_HPP

#include "../ui.hpp"

#include <readline/readline.h>

#include <string>
#include <functional>

class ReadlineWidget : public UI::Window {
public:
  ReadlineWidget();

  void draw()                         override;
  void layout(UI::Pos, UI::Size)      override;
  bool handleKey(int)                 override;

  void setPrompt(std::string)         noexcept;

  using onFinishFunction = std::function<void(std::string, bool)>;
  onFinishFunction onFinish;

private:
  std::string _prompt;
};

#endif
