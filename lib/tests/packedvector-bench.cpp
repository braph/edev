#include "../packedvector.hpp"
#include <climits>

int main() {
  PackedVector<int> v(sizeof(short) * CHAR_BIT - 1);

  const int nIter = 1;//1024;
  v.reserve(SHRT_MAX * nIter);

  for (int x = 0; x < 1000; ++x){
    for (int i = 0; i < SHRT_MAX * nIter; ++i)
      v.push_back(i);
    v.clear();
  }

  int sum = 0;
  for (auto i : v) sum += i;
  for (auto i : v) sum -= i;

#if 0
  int sum = 0;
  for (int x = 0; x < 1000; ++x){
  for (auto i : v) sum += i;
  for (auto i : v) sum -= i;
  for (auto i : v) sum += i;
  for (auto i : v) sum -= i;
  for (auto i : v) sum += i;
  for (auto i : v) sum -= i;
  }
#endif

  return sum;
}
