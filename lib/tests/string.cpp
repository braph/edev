#include <lib/string.hpp>
#include <lib/test.hpp>

int main() {
  TEST_BEGIN();

  {
    std::string s;
    s = "aaa";    replace_all(s, 'a', "xx"); assert(s == "xxxxxx");
    s = "xxxxxx"; replace_all(s, "xx", 'a'); assert(s == "aaa");
  }

  assert(icontains("I don't like Summer", "i"));
  assert(icontains("I don't like Summer", "i "));
  assert(icontains("I don't like Summer", "summer"));
  assert(icontains("I don't like Summer", "sum"));
  assert(icontains("I don't like Summer", "r"));

  TEST_END();
}
