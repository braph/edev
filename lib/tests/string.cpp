#include <lib/string.hpp>
#include <lib/test.hpp>

int main() {
  TEST_BEGIN();

  std::string s;

  s = "aaa";    replace_all(s, 'a', "xx"); assert(s == "xxxxxx");
  s = "xxxxxx"; replace_all(s, "xx", 'a'); assert(s == "aaa");

  TEST_END();
}
