#ifndef _CONFIG_HPP
#define _CONFIG_HPP

#include "database.hpp"
#include <string>
#include <vector>

struct PlaylistColumnFormat {
  enum Justification { Right, Left };

  Database::ColumnID tag;
  short fg;
  short bg;
  short size;
  bool relative;
  enum Justification justify;

  PlaylistColumnFormat()
    : tag(Database::COLUMN_NONE), fg(-1), bg(-1), size(0), relative(false), justify(Left)
  {
  }
};

struct PlayingInfoFormatFoo {
  short fg;
  short bg;
  unsigned int attributes;
  Database::ColumnID tag;
  std::string text;

  PlayingInfoFormatFoo()
    : fg(-1), bg(-1), attributes(0)
  {
  }
};

typedef std::vector<PlaylistColumnFormat> PlaylistColumns;
typedef std::vector<PlayingInfoFormatFoo> PlayingInfoFormat;

namespace Config {

#include "config/config.members.hpp"
void init();
void set(const std::string&, const std::string&);
void read(const std::string&);

} // namespace Config
#endif
