#include <lib/iterator/iterator_pair.hpp>
#include <lib/test.hpp>
#include <vector>
#include <iostream>

int main() {
  std::vector<int> v = {1,2,3,4};

  IteratorPair<std::vector<int>::iterator> pair(v.begin(), v.end());
  IteratorPair<std::vector<int>::iterator> pair2(v);

  while (pair) {
    std::cout << pair.next() << std::endl;
  }
}
