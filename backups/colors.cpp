#include "colors.hpp"
#include <cctype>
// TODO: make this thing a singleton
// ATTRIBUTES.default_proc = fail "Unknown attribute #{key}" unless key.is_a?Integer

using namespace UI;

void Colors :: start() {
  // TODO: Reset cached and aliases?! - also why is ||= in ruby used?!
  id = 1;
}

void Colors :: reset() {
  start();
}

int Colors :: getColorByName(const std::string& color) {
  try {
    if (isdigit(color[0]))
      return std::stoi(color);
    return COLORS.at(color);
  } catch (...) {
    throw std::invalid_argument("TODO");
  }
}

int Colors :: getAttrByName(const std::string& color) {
  return ATTRIBUTES.at(color);
}

void Colors :: setDefaultFG(const std::string &color) {
  defaultFG = getColorByName(color);
}

void Colors :: setDefaultBG(const std::string &color) {
  defaultBG = getColorByName(color);
}

int Colors :: init_pair_cached(const std::string &fg_name, const std::string &bg_name) {
  int fg, bg;

  if (fg_name == "default")
    fg = defaultFG;
  else
    fg = getColorByName(fg_name);

  if (bg_name == "default")
    bg = defaultBG;
  else
    bg = getColorByName(bg_name);

  int pair_id = cached[fg][bg];
  if (! pair_id)
    pair_id = cached[fg][bg] = id++;

  init_pair(pair_id, fg, bg); // or fail?
  return COLOR_PAIR(pair_id); // XXX why COLOR_PAIR???
}

int Colors :: set(
    const std::string &name,
    const std::string &fg_name,
    const std::string &bg_name
    // TODO: Attributes
) {
  int pair = init_pair_cached(fg_name, bg_name);
  // attrs.each { |attr| @@aliases[name] |= ATTRIBUTES[attr] }
  aliases[name] = pair;
  return pair;
}

int Colors :: get(const std::string &name) {
  return aliases.at(name);
}

    /*
      def self.add_attributes(*attrs)
         flags = 0
         attrs.each { |attr| flags |= ATTRIBUTES[attr] }
         flags
      end
    */

#if TEST_COLORS
#include <cassert>
int main() {
  Colors c;
  assert(c.getColorByName("none")     == -1);
  assert(c.getColorByName("white")    == COLOR_WHITE);
  assert(c.getColorByName("black")    == COLOR_BLACK);
  assert(c.getColorByName("red")      == COLOR_RED);
  assert(c.getColorByName("blue")     == COLOR_BLUE);
  assert(c.getColorByName("cyan")     == COLOR_CYAN);
  assert(c.getColorByName("green")    == COLOR_GREEN);
  assert(c.getColorByName("yellow")   == COLOR_YELLOW);
  assert(c.getColorByName("magenta")  == COLOR_MAGENTA);

  assert(c.getAttrByName("bold")      == A_BOLD);
  assert(c.getAttrByName("blink")     == A_BLINK);
  assert(c.getAttrByName("standout")  == A_STANDOUT);
  assert(c.getAttrByName("underline") == A_UNDERLINE);

  c.start();

  //assert(c.init_pair_cached("blue", "blue") == 1);
  //assert(c.init_pair_cached("blue", "blue") == 1);
}
#endif
