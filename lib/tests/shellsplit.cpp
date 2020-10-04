#include <lib/shellsplit.hpp>
#include <lib/test.hpp>

static std::vector<std::string> result; // Global (for checking if .clear works)

static void test(const char* line, const std::vector<std::string>& expected) {
  ShellSplit::split(line, result);
  if (result != expected) {
    std::cout << "Input: " << line << "\nResult: ";
    for (const auto& e : result)
      std::cout << '"' << e << "\",";
    std::cout << '\n';
    throw;
  }
}

int main() {
  TEST_BEGIN();
  test("",          {});
  test(" \t",       {});
  test("1",         {"1"});
  test(" 1",        {"1"});
  test(" 1 ",       {"1"});
  test("1 2",       {"1", "2"});
  test("1 2 3",     {"1", "2", "3"});
  test("'1'",       {"1"});
  test("\"1\"",     {"1"});
  test("' 1 '",     {" 1 "});
  test("\" 1 \"",   {" 1 "});
  test("'a''b'",    {"ab"});
  test("\"a\'\"",   {"a\'"});
  test("\'\\\'",    {"\\"});
  test("\"\\\\\"",  {"\\"});
  test("'\"'",      {"\""});
  test("\"'\"",     {"'"});
  TEST_END();
}
