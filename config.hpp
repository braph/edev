#ifndef _CONFIG_HPP
#define _CONFIG_HPP

#include "database.hpp"
#include <string>
#include <vector>

struct PlaylistColumnFormat {
  enum Justification { Right, Left };

  std::string tag;
  short fg;
  short bg;
  short size;
  bool relative;
  enum Justification justify;

  PlaylistColumnFormat()
    : fg(-1), bg(-1), size(0), relative(false), justify(Left)
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

class Config {
  Config(); /* Singleton */
public:
#include "config/config.members.hpp"
  static void init();
  static void set(const std::string&, const std::string&);
  static void read(const std::string&);
};

#endif
