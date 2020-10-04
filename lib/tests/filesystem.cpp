#include <lib/filesystem.hpp>
#include <lib/test.hpp>

int main() {
  assert( Filesystem::exists("/"));
  assert(!Filesystem::exists("/non-existant"));
  assert( Filesystem::exists(Filesystem::home()));
  assert( Filesystem::expand("~")     == Filesystem::home());
  assert( Filesystem::expand("~/foo") == std::string(Filesystem::home()) + "/foo");
}

