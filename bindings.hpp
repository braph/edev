#ifndef _BINDINGS_HPP
#define _BINDINGS_HPP

#include <ncurses.h>
#include "actions.hpp"

namespace Bindings {

void init();
extern Actions::ActionID global[KEY_MAX];
extern Actions::ActionID playlist[KEY_MAX];

} // namespace Bindings

#endif
