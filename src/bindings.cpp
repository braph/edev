#include "bindings.hpp"

#define C(K) (K % 32)

namespace Bindings {

Actions::ActionID global[KEY_MAX];
Actions::ActionID playlist[KEY_MAX];
Actions::ActionID pad[KEY_MAX];

void init() {
#define _ global
  _['f']                  = _[KEY_RIGHT]  = Actions::PLAYER_FORWARD;
  _['b']                  = _[KEY_LEFT]   = Actions::PLAYER_BACKWARD;

  _['`']     = _['^']                     = Actions::SPLASH_SHOW;
  _['1']                                  = Actions::PLAYLIST_SHOW;
  _['2']                                  = Actions::BROWSER_SHOW;
  _['3']                                  = Actions::INFO_SHOW;
  _['4']                  = _[KEY_F(1)]   = Actions::HELP_SHOW;
  _['!']     = _['\\']    = _[KEY_F(2)]   = Actions::PLAYINGINFO_TOGGLE;
  _['%']     = _['~']     = _[KEY_F(3)]   = Actions::PROGRESSBAR_TOGGLE;
  _['=']                  = _[KEY_F(4)]   = Actions::TABBAR_TOGGLE;
  _['s']                                  = Actions::PLAYER_STOP;
  _['p']     = _[' ']                     = Actions::PLAYER_TOGGLE;
  _['>']                                  = Actions::PLAYLIST_NEXT;
  _['<']                                  = Actions::PLAYLIST_PREV;
  _['l']     = _['}']     = _['\t']       = Actions::TABS_NEXT;
  _['h']     = _['{']     = _[KEY_BTAB]   = Actions::TABS_PREV;
  _[C('l')]                               = Actions::REDRAW;
  _['U']                                  = Actions::UPDATE;
  _['q']                                  = Actions::QUIT;
//_[C('r')]                               = RELOAD;
#undef _

#define _ playlist
  _['\n']    = _['\r']    = _[KEY_ENTER]  = Actions::PLAYLIST_PLAY;
  _['g']                  = _[KEY_HOME]   = Actions::TOP;
  _['G']                  = _[KEY_END]    = Actions::BOTTOM;
  _['k']                  = _[KEY_UP]     = Actions::UP;
  _['j']                  = _[KEY_DOWN]   = Actions::DOWN;
  _[C('u')]               = _[KEY_PPAGE]  = Actions::PAGE_UP;
  _[C('d')]               = _[KEY_NPAGE]  = Actions::PAGE_DOWN;
  _['/']                                  = Actions::SEARCH;
  _['n']                                  = Actions::SEARCH_NEXT;
  _['N']                                  = Actions::SEARCH_PREV;
  _['o']                                  = Actions::PLAYLIST_GOTO_CURRENT;
  _['C']                                  = Actions::PLAYLIST_CLEAR;
  _['d']                                  = Actions::PLAYLIST_DELETE;
#undef _

#define _ pad
  _['k']                  = _[KEY_UP]     = Actions::UP;
  _['j']                  = _[KEY_DOWN]   = Actions::DOWN;
  _[C('u')]               = _[KEY_PPAGE]  = Actions::PAGE_UP;
  _[C('d')]               = _[KEY_NPAGE]  = Actions::PAGE_DOWN;
  _['g']                  = _[KEY_HOME]   = Actions::TOP;
  _['G']                  = _[KEY_END]    = Actions::BOTTOM;
#undef _
}

} // namespace Bindings

/* nav: help, info, browser, playlist

@bindings[:playlist]
  # selection
  :'playlist.toggle_selection' => ['^v'                          ],
  # search
  :'playlist.search_up'      => [??                              ], TODO
  :'playlist.search_down'    => [?/                              ], TODO
  # playlist
  :'playlist.download_album' => [?$                              ],
  :'playlist.reload'         => [?r                              ],

@bindings[:browser]
  # selection
  :'browser.toggle_selection' => ['^v'                           ],
  # search
  :'browser.search_next'     => [?n                              ],
  :'browser.search_prev'     => [?N                              ],
  :'browser.search_up'       => [??                              ],
  :'browser.search_down'     => [?/                              ],
  # browser
  :'browser.add_to_playlist' => [' ', ?a                         ],
  :'browser.enter'           => [         ICurses::KEY_ENTER     ],
  :'browser.back'            => [?B,      ICurses::KEY_BACKSPACE ]
*/