#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "theme.hpp"
#include "views/playlist.hpp"
#include "views/infoline.hpp"
#include "views/mainwindow.hpp"
#include <lib/arrayview.hpp>
#include <lib/packed/tinyarray.hpp>
#include <lib/string.hpp>

#include <string>

struct ConfigError : public std::invalid_argument {
  using std::invalid_argument::invalid_argument;

  template<class S, typename ... T>
  ConfigError(S a, ConstChars b, T ... c)
    : ConfigError(std::string(a) + ": " + b.s, c ...)
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
