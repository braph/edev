#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "views/playlist.hpp"
#include "views/playinginfo.hpp"

#include <string>
#include <vector>

namespace Config {

using namespace Views;
#include "config/options.declare.hpp"
void init();
void read(const std::string&);
void set(const std::vector<std::string>&);
void color(const std::vector<std::string>&);
void bind(const std::vector<std::string>&);
void unbind(const std::vector<std::string>&);
void unbind_all(const std::vector<std::string>&);

} // namespace Config
#endif
