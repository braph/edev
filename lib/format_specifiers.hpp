#ifndef LIB_FORMAT_SPECIFIERS
#define LIB_FORMAT_SPECIFIERS

#include <cinttypes>

namespace format_specifiers {

namespace print {

template<typename T> struct specifier { };

template<>  struct specifier<std::int8_t>   { static constexpr const char* value = PRId8;   };
template<>  struct specifier<std::int16_t>  { static constexpr const char* value = PRId16;  };
template<>  struct specifier<std::int32_t>  { static constexpr const char* value = PRId32;  };
template<>  struct specifier<std::int64_t>  { static constexpr const char* value = PRId64;  };

template<>  struct specifier<std::uint8_t>  { static constexpr const char* value = PRIu8;   };
template<>  struct specifier<std::uint16_t> { static constexpr const char* value = PRIu16;  };
template<>  struct specifier<std::uint32_t> { static constexpr const char* value = PRIu32;  };
template<>  struct specifier<std::uint64_t> { static constexpr const char* value = PRIu64;  };

template<>  struct specifier<float>         { static constexpr const char* value = "%f";    };
template<>  struct specifier<double>        { static constexpr const char* value = "%Lf";   };

template<>  struct specifier<void*>         { static constexpr const char* value = "%p";    };

} // namespace print

} // namespace format_specifiers

#endif
