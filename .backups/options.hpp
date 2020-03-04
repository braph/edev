#ifndef _OPTIONS_HPP
#define _OPTIONS_HPP

#include <string>
#include <vector>
#include <stdexcept>

enum Justification { Right, Left };

class PlaylistColumnFormat {
  public:
    std::string column;
    unsigned int fg;
    unsigned int bg;
    unsigned int rel;
    unsigned int size;
    enum Justification justification;

    PlaylistColumnFormat()
      : fg(0), bg(0), rel(0), size(0)
    {
    }
};

typedef std::vector<PlaylistColumnFormat> PlaylistColumns;

class PlayingInfoFormatFoo {
  public:
    unsigned int fg;
    unsigned int bg;
    unsigned int attributes;
    std::string tag;

    PlayingInfoFormatFoo()
      : fg(0), bg(0), attributes(0)
    {
    }
};

typedef std::vector<PlayingInfoFormatFoo> PlayingInfoFormat;

static const char * const DEFAULT_PLAYLIST_FORMAT =
  "<number size='3' fg='magenta' />"
  "<artist rel='25' fg='blue'    />"
  "<album  rel='30' fg='red'     />"
  "<title  rel='33' fg='yellow'  />"
  "<styles rel='20' fg='cyan'    />"
  "<bpm    size='3' fg='green' justify='right' />";

static const char * const DEFAULT_PLAYINGINFO_FORMAT_TOP =
  "<text fg='black'>&lt;&lt; </text><title bold='on' fg='yellow' /><text fg='black'> &gt;&gt;</text>";

static const char * const DEFAULT_PLAYINGINFO_FORMAT_BOTTOM = 
  "<artist bold='on' fg='blue' /><text> - </text><album bold='on' fg='red' /><text> (</text><year fg='cyan' /><text>)</text>";

static const char * const DEFAULT_PLAYLIST_FORMAT_256 =
  "<number size='3' fg='97'  />"
  "<artist rel='25' fg='24'  />"
  "<album  rel='30' fg='160' />"
  "<title  rel='33' fg='178' />"
  "<styles rel='20' fg='37'  />"
  "<bpm    size='3' fg='28' justify='right' />";

static const char * const DEFAULT_PLAYINGINFO_FORMAT_TOP_256 =
  "<text fg='236'>&lt;&lt; </text><title bold='on' fg='178' /><text fg='236'> &gt;&gt;</text>";

static const char * const DEFAULT_PLAYINGINFO_FORMAT_BOTTOM_256 = 
 "<artist bold='on' fg='24' /><text> - </text><album bold='on' fg='160' /><text> (</text><year fg='37' /><text>)</text>";

int opt_parse_int(const std::string&);
bool opt_parse_bool(const std::string&);
char opt_parse_char(const std::string&);
std::string opt_parse_string(const std::string&);
std::string validate_use_colors(const std::string&);
int validate_threads(const std::string&);
PlaylistColumns opt_parse_playlist_format(const std::string&);
PlayingInfoFormat opt_parse_playinginfo_format(const std::string&);
std::vector<std::string> opt_parse_tabs_widgets(const std::string&);
std::vector<std::string> opt_parse_main_widgets(const std::string&);

#endif
