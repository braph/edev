#include "bindings.hpp"

#include "../lib/tmp/array_generator.hpp"

#define BINDINGS_GLOBAL \
  X2('f',               KEY_RIGHT,     PLAYER_FORWARD       )\
  X2('b',               KEY_LEFT,      PLAYER_BACKWARD      )\
  X2('`', '^',                         SPLASH_SHOW          )\
  X1('1',                              PLAYLIST_SHOW        )\
  X1('2',                              BROWSER_SHOW         )\
  X1('3',                              INFO_SHOW            )\
  X2('4',                KEY_F(1),     HELP_SHOW            )\
  X3('!', '\\',          KEY_F(2),     PLAYINGINFO_TOGGLE   )\
  X3('%', '~',           KEY_F(3),     PROGRESSBAR_TOGGLE   )\
  X2('=',                KEY_F(4),     TABBAR_TOGGLE        )\
  X1('s',                              PLAYER_STOP          )\
  X2('p', ' ',                         PLAYER_TOGGLE        )\
  X1('>',                              PLAYLIST_NEXT        )\
  X1('<',                              PLAYLIST_PREV        )\
  X3('l', '}',  '\t',                  TABS_NEXT            )\
  X3('h', '{',          KEY_BTAB,      TABS_PREV            )\
  X1(Control('l'),                     REDRAW               )\
  X1('U',                              UPDATE               )\
  X1('q',                              QUIT                 )\
  X1('I',                              SHOW_COVER           )\

#define BINDINGS_PLAYLIST \
  X3('\n', '\r',        KEY_ENTER,     PLAYLIST_PLAY         )\
  X1('$',                              PLAYLIST_DOWNLOAD     )\
  X2('g',               KEY_HOME,      TOP                   )\
  X2('G',               KEY_END,       BOTTOM                )\
  X2('k',               KEY_UP,        UP                    )\
  X2('j',               KEY_DOWN,      DOWN                  )\
  X2(Control('u'),      KEY_PPAGE,     PAGE_UP               )\
  X2(Control('d'),      KEY_NPAGE,     PAGE_DOWN             )\
  X1('/',                              SEARCH_DOWN           )\
  X1('?',                              SEARCH_UP             )\
  X1('n',                              SEARCH_NEXT           )\
  X1('N',                              SEARCH_PREV           )\
  X1('o',                              PLAYLIST_GOTO_CURRENT )\
  X1('C',                              PLAYLIST_CLEAR        )\
  X1('d',                              PLAYLIST_DELETE       )\

#define BINDINGS_PAD \
  X2('k',               KEY_UP,        UP                    )\
  X2('j',               KEY_DOWN,      DOWN                  )\
  X2(Control('u'),      KEY_PPAGE,     PAGE_UP               )\
  X2(Control('d'),      KEY_NPAGE,     PAGE_DOWN             )\
  X2('g',               KEY_HOME,      TOP                   )\
  X2('G',               KEY_END,       BOTTOM                )\

#define X1(KEY, ACTION) \
  template<> struct CURRENT_KEYMAP<KEY> { static const Actions::ActionID value = Actions::ACTION; };
#define X2(KEY1, KEY2, ACTION)         X1(KEY1, ACTION) X1(KEY2, ACTION)
#define X3(KEY1, KEY2, KEY3, ACTION)   X1(KEY1, ACTION) X2(KEY2, KEY3, ACTION)

namespace Bindings {

static constexpr int Control(int key) { return key % 32; }

template<size_t> struct pad_keys_      { static const Actions::ActionID value = Actions::NONE; };
template<size_t> struct global_keys_   { static const Actions::ActionID value = Actions::NONE; };
template<size_t> struct playlist_keys_ { static const Actions::ActionID value = Actions::NONE; };

#define CURRENT_KEYMAP global_keys_
BINDINGS_GLOBAL
#undef CURRENT_KEYMAP
#define CURRENT_KEYMAP pad_keys_
BINDINGS_PAD
#undef CURRENT_KEYMAP
#define CURRENT_KEYMAP playlist_keys_
BINDINGS_PLAYLIST
#undef CURRENT_KEYMAP

Actions::ActionID (&global)[KEY_MAX]   = XArray<KEY_MAX, Actions::ActionID, global_keys_>::Xdata::Values;
Actions::ActionID (&playlist)[KEY_MAX] = XArray<KEY_MAX, Actions::ActionID, playlist_keys_>::Xdata::Values;
Actions::ActionID (&pad)[KEY_MAX]      = XArray<KEY_MAX, Actions::ActionID, pad_keys_>::Xdata::Values;

#if 0
Actions::ActionID playlist[KEY_MAX];
Actions::ActionID pad[KEY_MAX];
Actions::ActionID global[KEY_MAX];

inline void init() noexcept {
  {
  auto& _ = global;
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
  _[Control('l')]                         = Actions::REDRAW;
  _['U']                                  = Actions::UPDATE;
  _['q']                                  = Actions::QUIT;
  _['I']                                  = Actions::SHOW_COVER;
//_[Control('r')]                         = Actions::RELOAD;
  }

  {
  auto& _ = playlist;
  _['\n']    = _['\r']    = _[KEY_ENTER]  = Actions::PLAYLIST_PLAY;
  _['$']                                  = Actions::PLAYLIST_DOWNLOAD;
  _['g']                  = _[KEY_HOME]   = Actions::TOP;
  _['G']                  = _[KEY_END]    = Actions::BOTTOM;
  _['k']                  = _[KEY_UP]     = Actions::UP;
  _['j']                  = _[KEY_DOWN]   = Actions::DOWN;
  _[Control('u')]         = _[KEY_PPAGE]  = Actions::PAGE_UP;
  _[Control('d')]         = _[KEY_NPAGE]  = Actions::PAGE_DOWN;
  _['/']                                  = Actions::SEARCH_DOWN;
  _['?']                                  = Actions::SEARCH_UP;
  _['n']                                  = Actions::SEARCH_NEXT;
  _['N']                                  = Actions::SEARCH_PREV;
  _['o']                                  = Actions::PLAYLIST_GOTO_CURRENT;
  _['C']                                  = Actions::PLAYLIST_CLEAR;
  _['d']                                  = Actions::PLAYLIST_DELETE;
  }

  {
  auto& _ = pad;
  _['k']                  = _[KEY_UP]     = Actions::UP;
  _['j']                  = _[KEY_DOWN]   = Actions::DOWN;
  _[Control('u')]         = _[KEY_PPAGE]  = Actions::PAGE_UP;
  _[Control('d')]         = _[KEY_NPAGE]  = Actions::PAGE_DOWN;
  _['g']                  = _[KEY_HOME]   = Actions::TOP;
  _['G']                  = _[KEY_END]    = Actions::BOTTOM;
  }
}
#endif

} // namespace Bindings

#if 0
@bindings[:playlist]
  :'playlist.toggle_selection' => ['^v'                            ]
  :'playlist.reload'           => [?r                              ]

@bindings[:browser]
  :'browser.add_to_playlist'   => [' ', ?a                         ]
  :'browser.enter'             => [         ICurses::KEY_ENTER     ]
  :'browser.back'              => [?B,      ICurses::KEY_BACKSPACE ]
#endif
