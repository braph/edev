#ifndef _CONFIG_HPP
#define _CONFIG_HPP

#include "views/playlist.hpp"
#include "views/playinginfo.hpp"

#include <string>
#include <vector>

namespace Config {

using namespace Views;
#include "config/options.declare.hpp"
void init();
void set(const std::string&, const std::string&);
void read(const std::string&);

} // namespace Config
#endif
