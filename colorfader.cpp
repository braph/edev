#include "colorfader.hpp"
#include "colors.hpp"

using namespace UI;

ColorFader :: ColorFader(std::vector<int> _colors)
  : colors(_colors)
{
}

/* Instance methods (bound to a vector) */

int ColorFader :: fade(unsigned int pos, unsigned int size) {
  return fade(colors, pos, size);
}

int ColorFader :: fade2(unsigned int pos, unsigned int size) {
  return fade2(colors, pos, size);
}

/* Class methods */

int ColorFader :: fade(const std::vector<int> &colors, unsigned int pos, unsigned int size) {
  unsigned int i = pos * colors.size() / size;
  return UI::Colors::set("", colors[i], -1, 0); // TODO?
}

int ColorFader :: fade2(const std::vector<int> &colors, unsigned int pos, unsigned int size) {
  unsigned int i = pos * colors.size() * 2 / size;

  if (i >= colors.size())
    i = colors.size() - (i - colors.size() + 1);

  return UI::Colors::set("", colors[i], -1, 0); // TODO?

#if sdf
  if (pos * 2 / size) // 2nd half
    colors.length() - pos * colors.length() / s

  else // 1st half
    return pos * colors.length() / (size / 2);
#endif
}

#if TEST_COLORFADER
#include <cassert>
#include <iostream>

void dump(std::vector<int> vec) {
  for (auto it = vec.begin(); it != vec.end(); ++it)
    std::cout << *it << ',';
  std::cout << std::endl;
}

std::vector<int> fade(std::vector<int> vec, unsigned size) {
  std::vector<int> result;
  for (unsigned i = 0; i < size; ++i)
    result.push_back(UI::ColorFader::fade(vec, i, size));
  return result;
}

std::vector<int> fade2(std::vector<int> vec, unsigned size) {
  std::vector<int> result;
  for (unsigned i = 0; i < size; ++i)
    result.push_back(UI::ColorFader::fade2(vec, i, size));
  return result;
}

#define test(FUNC, DATA, SIZE, ...) \
  assert(FUNC(DATA, SIZE) == std::vector<int>(__VA_ARGS__))

int main() {
  std::vector<int> fade_odd  = {1,2,3};
  std::vector<int> fade_even = {1,2,3,4};

  // Normal fade
  test(fade, fade_odd,  3,  {1,2,3});
  test(fade, fade_even, 4,  {1,2,3,4});

  test(fade, fade_odd,  6,  {1,1,2,2,3,3});
  test(fade, fade_even, 8,  {1,1,2,2,3,3,4,4});

  // Double fade
  //test(fade2, fade_odd,  3,  {1,2,3});
  //test(fade2, fade_even, 4,  {1,2,3,4});
  
  //dump(fade2(fade_even, 8));
  //dump(fade2(fade_even, 8));
  //return 0;

  test(fade2, fade_odd,  6,  {1,2,3,3,2,1});
  test(fade2, fade_even, 8,  {1,2,3,4,4,3,2,1});
}
#endif

#if asdfsadf
  def ColorFader._fade(colors, size)
     return [colors[0]] * size if colors.size == 1

     part_len = (size / colors.size)
     diff = size - part_len * colors.size

     (colors.size - 1).times.map do |color_i|
        [colors[color_i]] * part_len
     end.flatten.concat( [colors[-1]] * (part_len + diff) )
  end

  def ColorFader._fade2(colors, size)
     half = size / 2
     ColorFader._fade(colors, half) + ColorFader._fade(colors.reverse, size - half)
  end
#endif
