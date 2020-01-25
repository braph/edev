#include "colors.hpp"
#include <cctype>
#define ARRAY_SIZE(A) (sizeof(A)/sizeof(*A))
using namespace UI;

// === UI::Color ==============================================================

std::map<std::string, short> Color :: colors;

void Color :: init() {
  colors = {
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
}

short Color :: parse(const std::string& color) {
  try {
    if (isdigit(color[0]))
      return std::stoi(color);
    return colors.at(color);
  } catch (...) {
    throw std::invalid_argument(color + ": Not a color"); // TODO
  }
}

std::string Color :: to_string(short color) {
  for (const auto &it : colors)
    if (it.second == color)
      return it.first;

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

void Attribute :: init() {
  // TODO: This method is obsolete
}

int Attribute :: parse(const std::string& attribute) {
  for (int i = 0; i < ARRAY_SIZE(attributes); ++i)
    if (attribute == attributes[i].name)
      return attributes[i].value;

  throw std::invalid_argument(attribute + ": invalid attribute");
}

std::string Attribute :: to_string(int attribute) {
  for (int i = 0; i < ARRAY_SIZE(attributes); ++i)
    if (attribute == attributes[i].value)
      return attributes[i].name;

  throw std::invalid_argument("invalid attribute value"); // TODO
}

// === UI::Colors =============================================================

#define SHORTS_TO_INT(A,B) (A + (B<<16))
std::map<int32_t, int> Colors :: color_pairs;
std::map<std::string, int> Colors :: aliases;
int Colors :: id;

void Colors :: init() {
  // TODO: Reset cached and aliases?! - also why did I use ||= in ruby?!
  id = 1;
}

int Colors :: create_color_pair(short fg, short bg) {
  int pair_id = color_pairs[SHORTS_TO_INT(fg,bg)];
  if (! pair_id)
    pair_id = color_pairs[SHORTS_TO_INT(fg,bg)] = id++;

  init_pair(pair_id, fg, bg);
  return pair_id;
}

int Colors :: set(const std::string &name, short fg, short bg, int attributes = 0) {
  return aliases[name] = COLOR_PAIR(create_color_pair(fg, bg)) | attributes;
}

int Colors :: get(const std::string &name) {
  return aliases.at(name);
}

#if TEST_COLORS
#include <cassert>
int main() {
  UI::Color::init();

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

  UI::Attribute::init();

  assert(UI::Attribute::parse("bold")           == A_BOLD);
  assert(UI::Attribute::parse("blink")          == A_BLINK);
  assert(UI::Attribute::parse("standout")       == A_STANDOUT);
  assert(UI::Attribute::parse("underline")      == A_UNDERLINE);

  assert(UI::Attribute::to_string(A_BOLD)       == "bold");
  assert(UI::Attribute::to_string(A_BLINK)      == "blink");
  assert(UI::Attribute::to_string(A_STANDOUT)   == "standout");
  assert(UI::Attribute::to_string(A_UNDERLINE)  == "underline");

  UI::Colors::init();

  assert(UI::Colors::create_color_pair(COLOR_BLUE, COLOR_BLACK) == 1);
  assert(UI::Colors::create_color_pair(COLOR_BLUE, COLOR_BLACK) == 1);
  assert(UI::Colors::create_color_pair(-1, -1) == 2);
  assert(UI::Colors::create_color_pair(-1, -1) == 2);
  assert(UI::Colors::create_color_pair(-1, COLOR_BLACK) == 3);
  assert(UI::Colors::create_color_pair(-1, COLOR_BLACK) == 3);
  assert(UI::Colors::create_color_pair(COLOR_BLACK, -1) == 4);
  assert(UI::Colors::create_color_pair(COLOR_BLACK, -1) == 4);
  int col = UI::Colors::set("mycolor", COLOR_BLUE, COLOR_BLACK);
  assert(col == UI::Colors::get("mycolor"));
}
#endif
