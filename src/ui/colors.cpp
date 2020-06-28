#include "colors.hpp"

#include <cinttypes>

using namespace UI;

// === UI::Color ==============================================================

Color::mapping Color :: colors[] = {
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

short Color :: parse(const std::string& color) noexcept {
  for (const auto& it : colors)
    if (color == it.name)
      return it.value;

  char *end;
  std::intmax_t i = std::strtoimax(color.c_str(), &end, 10);
  if (! color.empty() && !*end && i >= -1 && i <= 256)
    return i;

  return Invalid;
}

std::string Color :: to_string(short color) noexcept {
  for (const auto& it : colors)
    if (it.value == color)
      return it.name;

  char _[10];
  return sprintf(_, "%hd", color), _;
}

// === UI::Attribute ==========================================================

Attribute::mapping Attribute :: attributes[] = {
  {"bold",      A_BOLD},
  {"dim",       A_DIM},
  {"blink",     A_BLINK},
  {"italic",    A_ITALIC},
  {"normal",    A_NORMAL},
  {"standout",  A_STANDOUT},
  {"underline", A_UNDERLINE}
};

unsigned int Attribute :: parse(const std::string& attribute) noexcept {
  for (const auto& e : attributes)
    if (attribute == e.name)
      return e.value;

  return Invalid;
}

std::string Attribute :: to_string(unsigned int attribute) noexcept {
  for (const auto& e : attributes)
    if (attribute == e.value)
      return e.name;

  return "";
}

// === UI::Colors =============================================================

int Colors :: last_id = 0;
std::array<Colors::color_pair, 256> Colors :: color_pairs;

int Colors :: create_color_pair(short fg, short bg) noexcept {
  int pair_id;
  for (pair_id = 1; pair_id <= last_id; ++pair_id)
    if (color_pairs[size_t(pair_id)].fg == fg && color_pairs[size_t(pair_id)].bg == bg)
      return pair_id;

  if (last_id == color_pairs.size())
    return 0;

  color_pairs[size_t(pair_id)] = {fg, bg};
  init_pair(pair_id, fg, bg);
  return ++last_id;
}

unsigned int Colors :: set(short fg, short bg, unsigned int attributes) noexcept {
  return COLOR_PAIR(create_color_pair(fg, bg)) | attributes;
}

void Colors :: reset() noexcept {
  last_id = 1;
}

#ifdef TEST_COLORS
#include <lib/test.hpp>
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
  assert(UI::Color::parse("no_color")         == UI::Color::Invalid);

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

  assert(UI::Attribute::parse("normal")         == A_NORMAL);
  assert(UI::Attribute::parse("bold")           == A_BOLD);
  assert(UI::Attribute::parse("blink")          == A_BLINK);
  assert(UI::Attribute::parse("standout")       == A_STANDOUT);
  assert(UI::Attribute::parse("underline")      == A_UNDERLINE);

  assert(UI::Attribute::to_string(A_NORMAL)     == "normal");
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
