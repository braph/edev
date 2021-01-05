#include <cstdlib>

template<class T>
struct auto_cast_helper {
  T v;

  auto_cast_helper(T v_) : v(v_) {}

  template<class TO> operator TO() const { return static_cast<TO>(v); }
  template<class TO> bool operator>(TO rhs) const { return v > rhs; }
  template<class TO> bool operator<(TO rhs) const { return v > rhs; }
};

template<class T>
auto_cast_helper<T> auto_cast(T v)
{ return auto_cast_helper<T>(v); }

void f(int) {}

int main(int, const char** argv) {
  size_t c = std::atol(argv[0]);

  int i = 0;

  return i < auto_cast(c);

  return i < c;

  //f(c);
  //f(auto_cast(c));
}
