#include "bindings.hpp"
#define C(K) (K%32)
namespace Bindings {

unsigned char global[KEY_MAX];
unsigned char playlist[KEY_MAX];

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
  _['p']                                  = Actions::PLAYER_TOGGLE;
  _['>']                                  = Actions::PLAYLIST_NEXT;
  _['<']                                  = Actions::PLAYLIST_PREV;
  _['l']  = _['}']  = _['\t']             = Actions::TABS_NEXT;
  _['h']  = _['{']  = _[KEY_BTAB]         = Actions::TABS_PREV;
  _[C('l')]                               = Actions::REDRAW;
  //_[C('r')]                               = RELOAD;
  //_[C('U')]                               = UPDATE;

  _['q']                                  = Actions::QUIT;
#undef _

#define _ playlist
  _['\n'] = _['\r']   = _[KEY_ENTER]      = Actions::PLAYLIST_PLAY;
  _['g']              = _[KEY_HOME]       = Actions::PLAYLIST_TOP;
  _['G']              = _[KEY_END]        = Actions::PLAYLIST_BOTTOM;
  _['k']              = _[KEY_UP]         = Actions::PLAYLIST_UP;
  _['j']              = _[KEY_DOWN]       = Actions::PLAYLIST_DOWN;
  _[C('u')]           = _[KEY_PPAGE]      = Actions::PLAYLIST_PAGE_UP;
  _[C('d')]           = _[KEY_NPAGE]      = Actions::PLAYLIST_PAGE_DOWN;
  _['o']                                  = Actions::PLAYLIST_GOTO_CURRENT;
#undef _
}

} // namespace Bindings




#if klasjdfslkdjf
#define C(K) K

void init() {
#define _ global
  _['`']     = _['^']                     = SPLASH__SHOW;
  _['1']                                  = PLAYLIST__SHOW;
  _['2']                                  = BROWSER__SHOW;
  _['3']                                  = INFO__SHOW;
  _['4']                  = _[KEY_F1]     = HELP__SHOW;
  _['!']     = _['\\']    = _[KEY_F2]     = PLAYINGINFO__TOGGLE;
  _['%']     = _['~']     = _[KEY_F3]     = PROGRESSBAR__TOGGLE;
  _['=']                  = _[KEY_F4]     = TABBAR__TOGGLE;
  _['f']                  = _[KEY_RIGHT]  = PLAYER__FORWARD;
  _['b']                  = _[KEY_LEFT]   = PLAYER__BACKWARD;
  _['s']                                  = PLAYER__STOP;
  _['p']                                  = PLAYER__TOGGLE;
  _['>']                                  = PLAYER__PLAY_NEXT;
  _['<']                                  = PLAYER__PLAY_PREV;
  _['l']  = _['}']  = _['\t']             = TABS__NEXT;
  _['h']  = _['{']  = _[KEY_BTAB]         = TABS__PREV;
  _['q']                                  = QUIT;
  _[C('l')]                               = REFRESH;
  _[C('r')]                               = RELOAD;
  _[C('U')]                               = UPDATE;

#undef _
#define _ navigation
  _['g']      = _[KEY_HOME]               = TOP;
  _['G']      = _[KEY_END]                = BOTTOM;
  _['k']      = _[KEY_UP]                 = UP;
  _['j']      = _[KEY_DOWN]               = DOWN;
  _[C('u')]   = _[KEY_PPAGE]              = PAGE_UP;
  _[C('d')]   = _[KEY_NPAGE]              = PAGE_DOWN;
}

#endif

#if NUR_ZUR_UEBERSICHT
  // nav: help, info, browser, playlist
   @bindings[:playlist] = {
      # selection
      :'playlist.toggle_selection' => ['^v'                          ],
      # search
      :'playlist.search_next'    => [?n                              ],
      :'playlist.search_prev'    => [?N                              ],
      :'playlist.search_up'      => [??                              ],
      :'playlist.search_down'    => [?/                              ],
      # playlist
      :'playlist.download_album' => [?$                              ],
      :'playlist.reload'         => [?r                              ],
      :'playlist.clear'          => [?c                              ],
      :'playlist.delete'         => [?d                              ],
      # other
      :'player.toggle'           => [' '                             ]
   }
   @bindings[:browser] = {
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
   }
   @bindings[:splash] = {}
#endif
