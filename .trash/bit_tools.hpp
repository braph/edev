template<class T>
constexpr int bitlength_const(T n, int i = sizeof(T)*CHAR_BIT) noexcept {
  using Unsigned_T = typename std::make_unsigned<T>::type;

  return
    (i) ? (
      (Unsigned_T(n) >= (1UL << i)) ? (
        i 
      ) : (
        bitlength_const(n, i - 1)
      )
    ) : ( 0 );
}

static constexpr int bitlength_const(unsigned long long i) noexcept {
#define X(BITS) i < (1UL<<BITS) ? BITS :
  return
    X( 0) X( 1) X( 2) X( 3) X( 4) X( 5) X( 6) X( 7)
    X( 8) X( 9) X(10) X(11) X(12) X(13) X(14) X(15)
    X(16) X(17) X(18) X(19) X(20) X(21) X(22) X(23)
    X(24) X(25) X(26) X(27) X(28) X(29) X(30) X(31)

    X(32) X(33) X(34) X(35) X(36) X(37) X(38) X(39)
    X(40) X(41) X(42) X(43) X(44) X(45) X(46) X(47)
    X(48) X(49) X(50) X(51) X(52) X(53) X(54) X(55)
    X(56) X(57) X(58) X(59) X(60) X(61) X(62) X(63)  64;
#undef X
}

#if (defined(__GNUC__) || defined(__clang__))
// Unsigned ===================================================================
static inline int clz(unsigned char n) noexcept {
  return (n ? __builtin_clz(n) : int(sizeof(int) * CHAR_BIT));
}

static inline int clz(unsigned short n) noexcept {
  return (n ? __builtin_clz(n) : int(sizeof(int) * CHAR_BIT));
}

static inline int clz(unsigned int n) noexcept {
  return (n ? __builtin_clz(n) : int(sizeof(int) * CHAR_BIT));
}

static inline int clz(unsigned long n) noexcept {
  return (n ? __builtin_clzl(n) : int(sizeof(long) * CHAR_BIT));
}

static inline int clz(unsigned long long n) noexcept {
  return (n ? __builtin_clzll(n) : int(sizeof(long long) * CHAR_BIT));
}

// Signed =====================================================================
static inline int clz(char n) noexcept {
  return clz(static_cast<unsigned char>(n));
}

static inline int clz(short n) noexcept {
  return clz(static_cast<unsigned short>(n));
}

static inline int clz(int n) noexcept {
  return clz(static_cast<unsigned int>(n));
}

static inline int clz(long n) noexcept {
  return clz(static_cast<unsigned long>(n));
}

static inline int clz(long long n) noexcept {
  return clz(static_cast<unsigned long long>(n));
}
#endif

