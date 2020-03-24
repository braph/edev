#include "colors.hpp"

#include <cctype>
#include <stdexcept>

using namespace UI;

// === UI::Color ==============================================================

Color :: mapping Color :: colors[] = {
  {"none",      -1},
  {"white",     COLOR_WHITE},
  {"black",     COLOR_BLACK},
  {"red",       COLOR_RED},
  {"blue",      COLOR_BLUE},
  {"cyan",      COLOR_CYAN},
  {"green",     COLOR_GREEN},
  {"yellow",    COLOR_YELLOW},
  {"magenta",   COLOR_MAGENTA}
};

short Color :: parse(const std::string& color, short on_error_return = -2) {
  if (! color.empty()) {
    if (isdigit(color[0]))
      return std::stoi(color);

    for (const auto& it : colors)
      if (color == it.name)
        return it.value;
  }

  return on_error_return;
}

std::string Color :: to_string(short color) {
  for (const auto& it : colors)
    if (it.value == color)
      return it.name;

  return std::to_string(color);
}

// === UI::Attribute ==========================================================

Attribute :: mapping Attribute :: attributes[] = {
  {"bold",      A_BOLD},
  {"dim",       A_DIM},
  {"blink",     A_BLINK},
  {"italic",    A_ITALIC},
  {"normal",    A_NORMAL},
  {"standout",  A_STANDOUT},
  {"underline", A_UNDERLINE}
};

unsigned int Attribute :: parse(const std::string& attribute) {
  for (const auto& e : attributes)
    if (attribute == e.name)
      return e.value;

  return 0;
}

std::string Attribute :: to_string(unsigned int attribute) {
  for (const auto& e : attributes)
    if (attribute == e.value)
      return e.name;

  return "";
}

// === UI::Colors =============================================================

std::vector<Colors::pair_id> Colors :: color_pairs;
int Colors :: last_id = 1;

int Colors :: create_color_pair(short fg, short bg) {
  for (const auto& pair : color_pairs)
    if (pair.fg == fg && pair.bg == bg)
      return pair.id;

  Colors::pair_id new_pair = {fg, bg, last_id++};
  init_pair(new_pair.id, fg, bg);
  color_pairs.push_back(new_pair);
  return new_pair.id;
}

unsigned int Colors :: set(short fg, short bg, unsigned int attributes) {
  return COLOR_PAIR(create_color_pair(fg, bg)) | attributes;
}

#ifdef TEST_COLORS
#include "../lib/test.hpp"
int main() {
  TEST_BEGIN();

  assert(UI::Color::parse("none")             == -1);
  assert(UI::Color::parse("white")            == COLOR_WHITE);
  assert(UI::Color::parse("black")            == COLOR_BLACK);
  assert(UI::Color::parse("red")              == COLOR_RED);
  assert(UI::Color::parse("blue")             == COLOR_BLUE);
  assert(UI::Color::parse("cyan")             == COLOR_CYAN);
  assert(UI::Color::parse("green")            == COLOR_GREEN);
  assert(UI::Color::parse("yellow")           == COLOR_YELLOW);
  assert(UI::Color::parse("magenta")          == COLOR_MAGENTA);
  assert(UI::Color::parse("123")              == 123);
  assert(UI::Color::parse("no_color")         == -2);

  assert(UI::Color::to_string(-1)             == "none");
  assert(UI::Color::to_string(COLOR_WHITE)    == "white");
  assert(UI::Color::to_string(COLOR_BLACK)    == "black");
  assert(UI::Color::to_string(COLOR_RED)      == "red");
  assert(UI::Color::to_string(COLOR_BLUE)     == "blue");
  assert(UI::Color::to_string(COLOR_CYAN)     == "cyan");
  assert(UI::Color::to_string(COLOR_GREEN)    == "green");
  assert(UI::Color::to_string(COLOR_YELLOW)   == "yellow");
  assert(UI::Color::to_string(COLOR_MAGENTA)  == "magenta");
  assert(UI::Color::to_string(123)            == "123");

  assert(UI::Attribute::parse("bold")           == A_BOLD);
  assert(UI::Attribute::parse("blink")          == A_BLINK);
  assert(UI::Attribute::parse("standout")       == A_STANDOUT);
  assert(UI::Attribute::parse("underline")      == A_UNDERLINE);

  assert(UI::Attribute::to_string(A_BOLD)       == "bold");
  assert(UI::Attribute::to_string(A_BLINK)      == "blink");
  assert(UI::Attribute::to_string(A_STANDOUT)   == "standout");
  assert(UI::Attribute::to_string(A_UNDERLINE)  == "underline");

  assert(UI::Colors::create_color_pair(COLOR_BLUE, COLOR_BLACK) == 1);
  assert(UI::Colors::create_color_pair(COLOR_BLUE, COLOR_BLACK) == 1);
  assert(UI::Colors::create_color_pair(-1, -1) == 2);
  assert(UI::Colors::create_color_pair(-1, -1) == 2);
  assert(UI::Colors::create_color_pair(-1, COLOR_BLACK) == 3);
  assert(UI::Colors::create_color_pair(-1, COLOR_BLACK) == 3);
  assert(UI::Colors::create_color_pair(COLOR_BLACK, -1) == 4);
  assert(UI::Colors::create_color_pair(COLOR_BLACK, -1) == 4);

  TEST_END();
}
#endif
