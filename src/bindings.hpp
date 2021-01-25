#ifndef BINDINGS_HPP
#define BINDINGS_HPP

#ifdef   CURSES_INC
#include CURSES_INC
#else
#include <curses.h>
#endif

#include "actions.hpp"

namespace Bindings {

extern Actions::ActionID (&global)[KEY_MAX];
extern Actions::ActionID (&playlist)[KEY_MAX];
extern Actions::ActionID (&browser)[KEY_MAX];
extern Actions::ActionID (&pad)[KEY_MAX];

}

#endif
