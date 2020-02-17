#ifndef _BINDINGS_HPP
#define _BINDINGS_HPP

#include <ncurses.h>
#include "actions.hpp"

namespace Bindings {

void init();
extern unsigned char global[KEY_MAX];
extern unsigned char playlist[KEY_MAX];

} // namespace Bindings

#endif
