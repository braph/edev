#include<string>
#include<vector>
//using namespace std;

std::vector<std::string> v = {"hallo du da diest ist ein string bla ich fick dich weg", "allo du da dies istein string ich fickd ich weg"};

char** vec2arr(std::vector<std::string>&& in_v) {
  union _ {
    std::vector<std::string> v;
    ~_() {}
  } u = {std::move(in_v)};

  char** cs = reinterpret_cast<char**>(u.un_v.data());

  std::string* beg = &(u.v[0]);
  std::string* end = &(u.v[u.v.size()]);

  for (int i = 0; beg < end; ++beg, ++i)
    cs[i] = const_cast<char*>(beg->data());

  return cs;
}

int main() {
  vec2arr(std::move(v));

}
