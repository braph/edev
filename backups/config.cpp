#include "config.hpp"

#include <fstream>
#include "ektoplayer.hpp"
#include "filesystem.hpp"
#include "shellsplit.hpp"
#include "options.hpp"

#include <iostream> //XXX:rm
using namespace Ektoplayer;

std::map<std::string, int>                       Config :: opts_int;
std::map<std::string, bool>                      Config :: opts_bool;
std::map<std::string, std::string>               Config :: opts_string;
std::map<std::string, std::vector<std::string> > Config :: opts_strings;
std::map<std::string, PlaylistColumns>           Config :: opts_columns;
std::map<std::string, PlayingInfoFormat>         Config :: opts_formats;

std::map<std::string, opt_callback_int>          Config :: cast_int;
std::map<std::string, opt_callback_bool>         Config :: cast_bool;
std::map<std::string, opt_callback_string>       Config :: cast_string;
std::map<std::string, opt_callback_strings>      Config :: cast_strings;
std::map<std::string, opt_callback_columns>      Config :: cast_columns;
std::map<std::string, opt_callback_formats>      Config :: cast_formats;

void Config :: register_option(const std::string &option, const std::string &value, opt_callback_string cb) {
  opts_string[option] = cb(value);
  cast_string[option] = cb;
}

void Config :: register_option(const std::string &option, const std::string &value, opt_callback_int cb) {
  opts_int[option] = cb(value);
  cast_int[option] = cb;
}

void Config :: register_option(const std::string &option, const std::string &value, opt_callback_bool cb) {
  opts_bool[option] = cb(value);
  cast_bool[option] = cb;
}

void Config :: register_option(const std::string &option, const std::string &value, opt_callback_columns cb) {
  opts_columns[option] = cb(value);
  cast_columns[option] = cb;
}

void Config :: register_option(const std::string &option, const std::string &value, opt_callback_formats cb) {
  opts_formats[option] = cb(value);
  cast_formats[option] = cb;
}

#define reg(OPTION, DESCRIPTION, VALUE, CALLBACK) register_option(OPTION, VALUE, CALLBACK)

void Config :: init() {
  std::string CONFIG_DIR = config_dir();
  std::string HOME       = Filesystem::home();
  #include "options2.hpp"
}

bool Config :: getBool(const std::string &option) {
  return opts_bool.at(option);
}

std::string Config :: getString(const std::string &option) {
  return opts_string.at(option);
}

void Config :: set(const std::string &option, const std::string &value) {
  auto bool_cb = cast_bool.find(option);
  if (bool_cb != cast_bool.end()) {
    opts_bool[option] = bool_cb->second(value);
    return;
  }

  auto int_cb = cast_int.find(option);
  if (int_cb != cast_int.end()) {
    opts_int[option] = int_cb->second(value);
    return;
  }

  auto string_cb = cast_string.find(option);
  if (string_cb != cast_string.end()) {
    opts_string[option] = string_cb->second(value);
    return;
  }

  auto columns_cb = cast_columns.find(option);
  if (columns_cb != cast_columns.end()) {
    opts_columns[option] = columns_cb->second(value);
    return;
  }

  auto formats_cb = cast_formats.find(option);
  if (formats_cb != cast_formats.end()) {
    opts_formats[option] = formats_cb->second(value);
    return;
  }

  throw std::invalid_argument("unknown option: " + option);
  // fail "Invalid value '#{value}' for '#{option}': #{$!}"
}

/*
void Config :: color(const std::vector<std::string> &vec) {
  int fg, bg, attr;
}
*/

void Config :: read(const std::string &file) {
  std::ifstream infile(file);
  std::string   line;
  unsigned int  no = 0;

  while (getline(infile, line)) {
    ++no;

    // line.chomp? next if line.empty? or start_with #
    std::vector<std::string> args = shellsplit(line);
    if (! args.size() || args[0][0] == '#')
      continue;

    std::string command = args[0];
#define on(STR) else if (command == STR)
    try {
      if (0) {}
      on("set")        { set(args[1], args[2]);     }
      /*
      on("bind")       { bindings.bind(...);        }
      on("unbind")     { bindings.unbind(...);      }
      on("unbind_all") { bindings.unbind_all(...);  }
      on("color")      { theme.color(...);          }
      on("color_256")  { theme.color_256(...);      }
      on("color_mono") { theme.color_mono(...);     }
      */
      else             { throw std::invalid_argument(command); }
    }
    catch (const std::exception &e) {
      char msg[4192];
      std::sprintf(msg, "%s:%u: %s: %s", file.c_str(), no, command.c_str(), e.what());
      throw std::invalid_argument(msg);
      //throw "#{file}:#{$.}: #{command}: #{$!}";
    }
  }
}

#if 0
   class ColumnFormat
      def self.parse_column_format(format)
         self.parse_simple_format(format).
            select { |f| f[:tag] != 'text' }.
            map do |fmt|
            fmt[:size]    = fmt[:size].to_i      if fmt[:size]
            fmt[:rel]     = fmt[:rel].to_i       if fmt[:rel]
            fmt[:justify] = fmt[:justify].to_sym if fmt[:justify]

            begin
               fail 'Missing size= or rel=' if (!fmt[:size] and !fmt[:rel])
               fail 'size= and rel= are mutually exclusive' if (fmt[:size] and fmt[:rel])
            rescue 
               fail "column: #{fmt[:tag]}: #{$!}"
            end

            fmt
         end
      end

      def self.parse_simple_format(format)
         self._parse_markup(format).map do |fmt|
            fmt[:curses_attrs] = [
               (fmt[:fg] and Integer(fmt[:fg]) rescue fmt[:fg].to_sym),
               (fmt[:bg] and Integer(fmt[:bg]) rescue fmt[:bg].to_sym),
            ]
            fmt[:curses_attrs] << :bold      if fmt[:bold]
            fmt[:curses_attrs] << :blink     if fmt[:blink]
            fmt[:curses_attrs] << :standout  if fmt[:standout]
            fmt[:curses_attrs] << :underline if fmt[:underline]
            fmt
         end
      end

      def self._parse_markup(format)
         Nokogiri::XML("<f>#{format}</f>").first_element_child.
            children.map do |fmt|
               fmt1 = fmt.attributes.map do |name,a|
                  [name.to_sym, a.value]
               end.to_h.update(tag: fmt.name)
               fmt1[:text] = fmt.text if fmt1[:tag] == 'text'
               fmt1
         end
      end
   end
#endif

#if TEST_CONFIG
#include <iostream>
#include "colors.hpp"

int main() {
  Config c;
  try {
    UI::Colors::init();
    UI::Color::init();
    UI::Attribute::init();
    Config::init();
    c.set("foo", "bar");
  }
  catch (const std::exception &e) {
    std::cout << "Error: " << e.what() << std::endl;
  }
}
#endif
