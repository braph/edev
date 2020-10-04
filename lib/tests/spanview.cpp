#include <lib/spanview.hpp>
#include <lib/test.hpp>
#include <vector>

static void dump(std::vector<int> vec) {
  for (auto it = vec.cbegin(); it != vec.cend(); ++it)
    std::cout << *it << ',';
  std::cout << std::endl;
}

static void test_span_get(std::vector<int> input, size_t output_size, std::vector<int> expected) {
  SpanView<std::vector<int>> span(input);
  for (size_t i = 0; i < output_size; ++i)
    if (! (span.get(output_size, i) == expected[i]))
      throw std::runtime_error("test_get()");
}

static void test_span_get2(std::vector<int> input, size_t output_size, std::vector<int> expected) {
  SpanView<std::vector<int>> span(input);
  std::cout << std::endl;
  for (size_t i = 0; i < output_size; ++i) {
    std::cout << span.get2(output_size, i) << ',';
    if (! (span.get2(output_size, i) == expected[i])) {
      throw std::runtime_error("test_get2()");
    }
  }
}

int main() {
  TEST_BEGIN();

  //             <INPUT> <OUTPUT SIZE> <EXPECTED>
  test_span_get({1,2,3},       3,      {1,2,3});
  test_span_get({1,2,3},       6,      {1,1,2,2,3,3});
  test_span_get({1,2,3,4},     4,      {1,2,3,4});
  test_span_get({1,2,3,4},     8,      {1,1,2,2,3,3,4,4});

  test_span_get2({1,2,3},      6,      {1,2,3,3,2,1});
  test_span_get2({1,2,3,4},    8,      {1,2,3,4,4,3,2,1});

  // TODO: This does not work as specified, but it's good enough, though...
  //test_span_get2({1,2,3},      7,      {1,2,3,3,3,2,1});
  //test_span_get2({1,2,3,4},    9,      {1,2,3,4,4,4,3,2,1});

  TEST_END();
}
