#include <vector>
#include <cinttypes>
#include <iostream>

/* A vector that holds custom sized (`odd`) ints */

// XXX: Overflow on large keysets (N * 32 / bits)

class OddVector {
  int bits;
  size_t _size;

public:
  std::vector<uint32_t> v;

  OddVector(int bits) : bits(bits), _size(0) {}

  bool    empty()     const;
  size_t  size()      const;
  size_t  capacity()  const;
  void    clear();
  void    reserve(size_t count);
  void    resize(size_t count);

  int     operator[](int index);
  void    set(size_t index, int value);
  void    push_back(const int &value);

  //reference at(size_t pos);
  
  inline std::vector<uint32_t>& vec() { return v; }
};

#define C OddVector
size_t C :: size()      const         { return this->_size;              }
bool   C :: empty()     const         { return this->_size == 0;         }
void   C :: clear()                   { this->_size = 0;                 }
size_t C :: capacity()  const         { return v.capacity() * 32 / bits; }

void   C :: reserve(size_t n) {
  if (n)
    n = ((n+1) * bits / 32 - 1);
  v.reserve(n);
}

void   C :: resize(size_t n) {
  _size=n; v.resize((n+1) * bits / 32  - 1);
}

static inline uint32_t replace_bits(uint32_t src, uint32_t val, uint32_t offset, uint32_t len) {
  if (len == 32)
    return val;
  uint32_t mask = (~(0xFFFFFFFF << len)) << offset;
  return (src & ~mask) | (val & mask);
}

int    C :: operator[](int index) {
  uint32_t bitOffset   = (index * bits) % 32;
  uint32_t vectorIndex = (index * bits - bitOffset) / 32;

  if (bitOffset + bits <= 32) {
    uint32_t elm    = v[vectorIndex];
    uint32_t lShift = bitOffset;
    uint32_t rShift = 32 - (bitOffset + bits);
    return ((elm << lShift) >> lShift >> rShift);
  } else {
    uint32_t overlap = bitOffset + bits - 32;
    uint32_t left    = v[vectorIndex]   << bitOffset;
    uint32_t right   = v[vectorIndex+1] >> (32 - overlap);
    return (left >> (32 - bits)) | right;
  }
  return -1;
}

void   C :: set(size_t index, int value) {
  uint32_t vectorIndex = (index * bits) / 32;
  uint32_t bitOffset   = (index * bits) % 32;
/*
  uint32_t bitRest     = bitOffset + bits % 32;
  bitOffset           -= bitRest;
*/

  std::cout
    << "set(value="    << value
    << " vectorIndex=" << vectorIndex
    << " bitOffset="   << bitOffset
    << ")\n";

  if (bitOffset + bits <= 32) {
    int e = v[vectorIndex];
    v[vectorIndex] = replace_bits(e, value, bitOffset, bits - 1);
  } else {
    uint32_t overlap = bitOffset + bits - 32;
    v[vectorIndex] = replace_bits(v[vectorIndex], value, bitOffset, bits);
    v[vectorIndex+1] = replace_bits(v[vectorIndex+1], value, 0, overlap);
  }
}

void  C :: push_back(const int &value) {
  resize(size() + 1);
  set(size() - 1, value);
}

#ifdef TEST_ODDVECTOR
#include <cassert>
#include <iostream>

void dump(OddVector &odd) {
  for (auto i : odd.v) {
    std::cout << '[' << i << ']' << ' ';
    for (int n = 0; n < 32; ++n)
      std::cout << ((i >> n) & 1U);
    std::cout << ' ';
  }
  std::cout << std::endl;
}


#define CHECK check(vec, odd)
static inline void check(std::vector<uint32_t> &vec, OddVector &odd) {
  assert(vec.size()     ==  odd.size());
  assert(vec.empty()    ==  odd.empty());
  //assert(vec.capacity() ==  odd.capacity()); XXX capacity may change
}

#define TEST(WHAT)      vec.WHAT; odd.WHAT; CHECK
#define TEST_INT(WHAT)  assert((int) vec.WHAT == (int) odd.WHAT); CHECK

int main() {
  // Test if the implementation behaves like a normal vector for bit sizes
  // 32 to 1.
  for (int bits = 32; bits; --bits) {
    std::cout << "Testing with " << bits << " bits." << std::endl;

    std::vector<uint32_t> vec;
    OddVector             odd(bits);
    CHECK;

    vec.reserve(0);
    TEST(reserve(0));

    TEST(reserve(1));

    //TEST(reserve(10));
    //TEST(resize(0));

    //TEST(reserve(1));
    //TEST(resize(1));

    TEST(push_back(1));
    dump(odd);
    std::cout << odd[0] << ':' << vec[0] << std::endl;
    TEST_INT(operator[](0));

    TEST(push_back(2));
    std::cout << vec[1] << ':' << odd[1] << std::endl;
    TEST_INT(operator[](0));
    TEST_INT(operator[](1));
  }

#if 0
  /*********/ vec.reserve(1);
  assert(0 == vec.size());
  assert(1 == vec.capacity());
  /*********/ vec.resize(1);
  assert(0 == vec[0]);

  // === Test the OddVector ===
  assert(0 == odd.size());
  assert(0 == odd.v.size());
  assert(0 == odd.capacity());
  assert(0 == odd.v.capacity());
  /*********/ odd.reserve(1);
  assert(0 == odd.size());
  assert(0 == odd.v.size());
  assert(1 == odd.capacity());
  assert(1 == odd.v.capacity());
  /*********/ odd.resize(1);
  assert(1 == odd.size());
  assert(1 == odd.v.size());
  assert(1 == odd.capacity());
  assert(1 == odd.v.capacity());
  assert(0 == odd[0]);
#endif

  //OddVector t0(2);
  //assert(t0.size() == 0);
}
#endif




#if FUCK
  if (bitOffset + bits <= 32) {

    uint32_t e = v[vectorIndex];
    uint32_t clearMask =
      FILLBITS_LEFT(bitOffset)|FILLBITS_RIGHT(32-bits-bitOffset);
    e &= clearMask;
    e |= (value << 32-bits-bitOffset);


    v[vectorIndex] =
      // Preserve left bits
      ((e >> lShift) << lShift)     |
      // Foo
      (value << (32 - bitOffset))   |
      // Bar
      ((e & (
      v[vectorIndex] 

// I'm an idiot. This stuff can for sure be optimized.
#define FULLBITS          ((uint32_t) 0xFFFFFFFF) //        1111
#define FILLBITS_LEFT(N)  (FULLBITS << (32 - N))  // N=1 -> 1000
#define FILLBITS_RIGHT(N) (FULLBITS >> (32 - N))  // N=1 -> 0001

  uint32_t bitOffset   = (index * bits) % 32;
  uint32_t vectorIndex = (index * bits - bitOffset) / 32;

  if (bitOffset + bits <= 32) {
    uint32_t elm    = v[vectorIndex];
    uint32_t lShift = bitOffset;
    uint32_t rShift = 32 - (bitOffset + bits);
    return ((elm << lShift) >> lShift >> rShift);
  } else {
    uint32_t overlap = bitOffset + bits - 32;
    uint32_t left    = v[vectorIndex]   << bitOffset;
    uint32_t right   = v[vectorIndex+1] >> (32 - overlap);
    return (left >> (32 - bits)) | right;
  }
#endif
