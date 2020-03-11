#ifndef _BINDINGS_HPP
#define _BINDINGS_HPP

#include CURSES_INC

#include "actions.hpp"

namespace Bindings {

void init();
extern Actions::ActionID global[KEY_MAX];
extern Actions::ActionID playlist[KEY_MAX];
extern Actions::ActionID help[KEY_MAX];
extern Actions::ActionID info[KEY_MAX];

} // namespace Bindings

#endif
