#include "base64.hpp"

// Originally taken from:
// https://stackoverflow.com/questions/342409/how-do-i-base64-encode-decode-in-c/41094722#41094722

namespace base64 {

static const unsigned char B64index_[256] = {
  0,    0,    0,    0,    0,    0,    0,    0,
  0,    0,    0,    0,    0,    0,    0,    0,
  0,    0,    0,    0,    0,    0,    0,    0,
  0,    0,    0,    0,    0,    0,    0,    0,
  0,    0,    0,    0,    0,    0,    0,    0,
  0,    0,    0,   62,   63,   62,   62,   63,
  52,  53,   54,   55,   56,   57,   58,   59,
  60,  61,    0,    0,    0,    0,    0,    0,
  0,    0,    1,    2,    3,    4,    5,    6,
  7,    8,    9,   10,   11,   12,   13,   14,
  15,  16,   17,   18,   19,   20,   21,   22,
  23,  24,   25,    0,   0,    0,    0,    63,
  0,   26,   27,   28,   29,   30,   31,   32,
  33,  34,   35,   36,   37,   38,   39,   40,
  41,  42,   43,   44,   45,   46,   47,   48,
  49,  50,   51
};

static inline unsigned B64index(unsigned char i) {
  return B64index_[i];
}

std::string decode(const char* data, const size_t len)
{
  const unsigned char* p = reinterpret_cast<const unsigned char*>(data);
  const size_t pad = len > 0 && (len % 4 || p[len - 1] == '=');
  const size_t L = ((len + 3) / 4 - pad) * 4;
  std::string str(L / 4 * 3 + pad, '\0');

  for (size_t i = 0, j = 0; i < L; i += 4)
  {
    unsigned n = B64index(p[i]) << 18 | B64index(p[i + 1]) << 12 | B64index(p[i + 2]) << 6 | B64index(p[i + 3]);
    str[j++] = n >> 16;
    str[j++] = n >> 8 & 0xFF;
    str[j++] = n & 0xFF;
  }

  if (pad)
  {
    unsigned n = B64index(p[L]) << 18 | B64index(p[L + 1]) << 12;
    str[str.size() - 1] = n >> 16;

    if (len > L + 2 && p[L + 2] != '=')
    {
      n |= B64index(p[L + 2]) << 6;
      str.push_back(n >> 8 & 0xFF);
    }
  }

  return str;
}

} // namespace base64
