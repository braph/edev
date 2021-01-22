
  template<size_t N>
  struct CharArray {
    char data[N];
    inline char*       c_str()       noexcept { return data; }
    inline const char* c_str() const noexcept { return data; }
    inline size_t      size()  const noexcept { return std::strlen(data); } // TODO
    inline char&       operator[](size_t i) noexcept { return data[i]; }
    inline const char& operator[](size_t i) const noexcept { return data[i]; }
  };

