#include <lib/sscan.hpp>
#include <lib/test.hpp>

void test_myscanf(const char* s) {
  std::intmax_t a, b, c, d;
  SScan scanner(s);
  scanner.strtoimax(a);
  scanner.strtoimax(b);
  scanner.strtoimax(c);
  scanner.strtoimax(d);
  assert(a == 1);
  assert(b == 2);
  assert(c == 3);
  assert(d == 4);
  assert(scanner);
}

int main(int, char** argv) {
  const char* s = argv[1];
  std::cout << s << std::endl;
  for (int i = 0; i < 9999999; ++i)
    test_myscanf(s);
}
