

template<typename T> constexpr const char* specifier();
template<>           constexpr const char* specifier<int>() { return "%d"; }

  
