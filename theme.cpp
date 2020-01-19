#include "theme.hpp"
#include "colors.hpp"
#include <set>
#include <iostream> // XXX
// TODO: current -> 0,1,2? 0,8,256?
// TODO: test
using namespace Ektoplayer;

// For better readability ...
#define default    -2
#define none       -1
#define white      COLOR_WHITE
#define black      COLOR_BLACK
#define red        COLOR_RED
#define blue       COLOR_BLUE
#define cyan       COLOR_CYAN
#define green      COLOR_GREEN
#define yellow     COLOR_YELLOW
#define magenta    COLOR_MAGENTA

#undef standout
#define bold       A_BOLD
#define blink      A_BLINK
#define standout   A_STANDOUT
#define underline  A_UNDERLINE

#define THEME(N) (N >= 256 ? 2 : (N >= 8 ? 1 : 0))

unsigned int Theme :: current;

std::map<std::string, ThemeDefinition> Theme :: themes[3] = {
  // Mono (no colors)
  {
    {"default",                  {-1, -1                      }},
    {"url",                      {default, default, underline }},
    {"tabbar.selected",          {default, default, bold      }}
  },

  // 8 Colors (default)
  {
    {"default"                 , {-1, -1                       }},
    {"url"                     , {magenta, default, underline  }},

    {"info.head"               , {blue, default, bold          }},
    {"info.tag"                , {blue                         }},
    {"info.value"              , {magenta                      }},
    {"info.description"        , {blue                         }},
    {"info.download.file"      , {blue                         }},
    {"info.download.percent"   , {magenta                      }},
    {"info.download.error"     , {red                          }},

    {"progressbar.progress"    , {blue                         }},
    {"progressbar.rest"        , {black                        }},

    {"tabbar.selected"         , {blue                         }},
    {"tabbar.unselected"       , {white                        }},

    {"list.item_even"          , {blue                         }},
    {"list.item_odd"           , {blue                         }},
    {"list.item_selection"     , {magenta                      }},

    {"playinginfo.position"    , {magenta                      }},
    {"playinginfo.state"       , {cyan                         }},

    {"help.widget_name"        , {blue, default, bold          }},
    {"help.key_name"           , {blue                         }},
    {"help.command_name"       , {magenta                      }},
    {"help.command_desc"       , {yellow                       }}
  },

  // 256 Colors
  {
    {"default"                 , {white, 233                   }},
    {"url"                     , {97, default, underline       }},

    {"info.head"               , {32, default, bold            }},
    {"info.tag"                , {74                           }},
    {"info.value"              , {67                           }},
    {"info.description"        , {67                           }},
    {"info.download.file"      , {75                           }},
    {"info.download.percent"   , {68                           }},
    {"info.download.error"     , {red                          }},

    {"progressbar.progress"    , {23                           }},
    {"progressbar.rest"        , {black                        }},

    {"tabbar.selected"         , {75                           }},
    {"tabbar.unselected"       , {250                          }},

    {"list.item_even"          , {26                           }},
    {"list.item_odd"           , {25                           }},
    {"list.item_selection"     , {97                           }},

    {"playinginfo.position"    , {97                           }},
    {"playinginfo.state"       , {37                           }},

    {"help.widget_name"        , {33                           }},
    {"help.key_name"           , {75                           }},
    {"help.command_name"       , {68                           }},
    {"help.command_desc"       , {29                           }}
  }
};

void Theme :: set(unsigned int theme, const std::string &name, short fg, short bg, int attributes) {
  themes[theme][name] = {fg,bg,attributes};
}

void Theme :: loadTheme(unsigned int theme) {
  // fail 'unknown theme' unless ....
  current = theme;
  int theme_idx = THEME(theme);

  UI::Colors::init();

  std::set<std::string> keys;
  for (unsigned i = 0; i < 3; ++i)
    for (auto pair = themes[i].cbegin(); pair != themes[i].cend(); ++pair)
      keys.insert(pair->first);
  keys.erase(keys.find("default"));

  const ThemeDefinition& deflt = themes[theme_idx].at("default");

  for (auto key = keys.cbegin(); key != keys.cend(); ++key) {
    for (int i = theme_idx; i >= 0; --i) {
      try {
        const ThemeDefinition& def = themes[i].at(*key);
        UI::Colors::set(*key,
            (def.fg == -2 ? deflt.fg : def.fg),
            (def.bg == -2 ? deflt.bg : def.bg),
            (def.attributes == -2 ? deflt.attributes : def.attributes)
        );
        break; // TODO!
      }
      catch (...){}
    }
  }
}

#if TEST_THEME
int main() {
}
#endif

