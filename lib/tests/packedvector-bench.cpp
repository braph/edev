#include "../packedvector.hpp"
#include <climits>

int main() {
  PackedVector<int> v(sizeof(short) * CHAR_BIT - 1);

  const int nIter = 1024;
  v.reserve(SHRT_MAX * nIter);

  for (int i = 0; i < SHRT_MAX * nIter; ++i)
    v.push_back(i);

  int sum = 0;
  //for (auto i : v) sum += i;
  //for (auto i : v) sum -= i;
  return sum;
}
