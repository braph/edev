#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "theme.hpp"
#include "views/playlist.hpp"
#include "views/infoline.hpp"
#include "views/mainwindow.hpp"
#include <lib/arrayview.hpp>
#include <lib/packed/tinyarray.hpp>

#include <string>

struct ConfigError : public std::invalid_argument {
  using std::invalid_argument::invalid_argument;

  ConfigError(std::string s1, const char* s2)
    : invalid_argument(s1+": "+s2)
  {}

  ConfigError(std::string s1, const char* s2, const char* s3)
    : invalid_argument(s1+": "+s2+": "+s3)
  {}
};

namespace Config {

using namespace Views;
using string_array = ArrayView<const char*>;

#include "config/options.declare.hpp"

void init();
void read(const char*);
void set(const string_array&);
void color(Theme&, const string_array&);
void bind(const string_array&);
void unbind(const string_array&);
void unbind_all(const string_array&);

} // namespace Config
#endif
