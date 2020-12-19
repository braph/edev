
template<size_t N>
struct CharPool {
  CharPool() : _pos(0) { _data[0] = '\0'; }
  char* create() noexcept { _pos += strlen(&_data[_pos]) + 1; _data[_pos] = 0; return &_data[_pos]; }
  void  clear()  noexcept { _pos = _data[0] = '\0'; }
private:
  size_t _pos;
  char   _data[N];
};

char* strins(char* dest, const char* src) {
  size_t dn = strlen(dest);
  size_t sn = strlen(src);
  std::memmove(dest + sn, dest, dn + 1);
  std::memcpy(dest, src, sn);
  return dest;
}

