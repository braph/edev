#include "common.hpp"

#if TEST_COMMON
#include <cassert>
int main() {
  assert( secondsToTime(0)  == "00:00" );
  assert( secondsToTime(1)  == "00:01" );
  assert( secondsToTime(60) == "01:00" );
  assert( secondsToTime(61) == "01:01" );
}
#endif
