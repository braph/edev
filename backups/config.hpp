#ifndef _CONFIG_HPP
#define _CONFIG_HPP

#include <map>
#include <vector>
#include <string>
#include "options.hpp"

// TODO: name opt_callback
typedef int                       (*opt_callback_int)     (const std::string&);
typedef bool                      (*opt_callback_bool)    (const std::string&);
typedef std::string               (*opt_callback_string)  (const std::string&);
typedef std::vector<std::string>  (*opt_callback_strings) (const std::string&);
typedef PlaylistColumns           (*opt_callback_columns) (const std::string&);
typedef PlayingInfoFormat         (*opt_callback_formats) (const std::string&);

namespace Ektoplayer {
  class Config {
    private:
      static std::map<std::string, int>                       opts_int;
      static std::map<std::string, bool>                      opts_bool;
      static std::map<std::string, std::string>               opts_string;
      static std::map<std::string, std::vector<std::string> > opts_strings;
      static std::map<std::string, PlaylistColumns>           opts_columns;
      static std::map<std::string, PlayingInfoFormat>         opts_formats;

      static std::map<std::string, opt_callback_int>          cast_int;
      static std::map<std::string, opt_callback_bool>         cast_bool;
      static std::map<std::string, opt_callback_string>       cast_string;
      static std::map<std::string, opt_callback_strings>      cast_strings;
      static std::map<std::string, opt_callback_columns>      cast_columns;
      static std::map<std::string, opt_callback_formats>      cast_formats;

      static void register_option(const std::string&, const std::string&, opt_callback_string);
      static void register_option(const std::string&, const std::string&, opt_callback_int);
      static void register_option(const std::string&, const std::string&, opt_callback_bool);
      static void register_option(const std::string&, const std::string&, opt_callback_columns);
      static void register_option(const std::string&, const std::string&, opt_callback_formats);

    public:
      static void init();
      static void set(const std::string&, const std::string&);
      static void read(const std::string&);

      static bool         getBool(const std::string&);
      static std::string  getString(const std::string&);
  };
}

#endif
