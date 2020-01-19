#include "ektoplayer.hpp"

#if TEST_EKTOPLAYER
#include <iostream>

int main() {
  std::cout
    << "album_url():   " << Ektoplayer::album_url("Foo")    << std::endl
    << "cover_url():   " << Ektoplayer::cover_url("Foo")    << std::endl
    << "track_url():   " << Ektoplayer::track_url("Foo")    << std::endl
    << "style_url():   " << Ektoplayer::style_url("Foo")    << std::endl
    << "archive_url(): " << Ektoplayer::archive_url("Foo")  << std::endl
    << "config_dir():  " << Ektoplayer::config_dir()        << std::endl
    << "config_file(): " << Ektoplayer::config_file()       << std::endl
    ;
}
#endif
