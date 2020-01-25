#ifndef _CONFIG_HPP
#define _CONFIG_HPP

#include <string>
#include <vector>

class PlaylistColumnFormat {
  public:
    enum Justification { Right, Left };

    std::string tag;
    int fg;
    int bg;
    unsigned int rel;
    unsigned int size;
    enum Justification justify;

    PlaylistColumnFormat()
      : fg(-1), bg(-1), rel(0), size(0), justify(Left)
    {
    }
};

class PlayingInfoFormatFoo {
  public:
    int fg;
    int bg;
    unsigned int attributes;
    std::string tag;
    std::string text;

    PlayingInfoFormatFoo()
      : fg(-1), bg(-1), attributes(0)
    {
    }
};

typedef std::vector<PlaylistColumnFormat> PlaylistColumns;
typedef std::vector<PlayingInfoFormatFoo> PlayingInfoFormat;

class Config {
  private:
    Config(); // Singleton
  public:
#include "config/config.members.hpp"
    static void init();
    static void set(const std::string&, const std::string&);
    static void read(const std::string&);
};

#endif
