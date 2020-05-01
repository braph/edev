#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "views/playlist.hpp"
#include "views/infoline.hpp"

#include <string>
#include <vector>

namespace Config {

using namespace Views;
using string_vector = std::vector<std::string>;

#include "config/options.declare.hpp"

void init();
void read(const std::string&);
void set(const string_vector&);
void color(const string_vector&);
void bind(const string_vector&);
void unbind(const string_vector&);
void unbind_all(const string_vector&);

} // namespace Config
#endif
