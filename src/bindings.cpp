#include "bindings.hpp"

#include "../lib/tmp/array_generator.hpp"

#define bind1(KEYMAP, KEY, ACTION) \
  template<> struct KEYMAP ## _keys_ <KEY> { static const Actions::ActionID value = Actions::ACTION; }

#define bind2(KEYMAP, KEY1, KEY2, ACTION) \
  bind1(KEYMAP, KEY1, ACTION); \
  bind1(KEYMAP, KEY2, ACTION)

#define bind3(KEYMAP, KEY1, KEY2, KEY3, ACTION) \
  bind1(KEYMAP, KEY1,       ACTION); \
  bind2(KEYMAP, KEY2, KEY3, ACTION)

namespace Bindings {

static constexpr int Control(int key) { return key % 32; }

template<size_t> struct pad_keys_      { static const Actions::ActionID value = Actions::NONE; };
template<size_t> struct global_keys_   { static const Actions::ActionID value = Actions::NONE; };
template<size_t> struct browser_keys_  { static const Actions::ActionID value = Actions::NONE; };
template<size_t> struct playlist_keys_ { static const Actions::ActionID value = Actions::NONE; };

bind2(global,    'f',               KEY_RIGHT,     PLAYER_FORWARD     );
bind2(global,    'b',               KEY_LEFT,      PLAYER_BACKWARD    );
bind2(global,    '`', '^',                         SPLASH_SHOW        );
bind1(global,    '1',                              PLAYLIST_SHOW      );
bind1(global,    '2',                              BROWSER_SHOW       );
bind1(global,    '3',                              INFO_SHOW          );
bind2(global,    '4',                KEY_F(1),     HELP_SHOW          );
bind3(global,    '!', '\\',          KEY_F(2),     PLAYINGINFO_TOGGLE );
bind3(global,    '%', '~',           KEY_F(3),     PROGRESSBAR_TOGGLE );
bind2(global,    '=',                KEY_F(4),     TABBAR_TOGGLE      );
bind1(global,    's',                              PLAYER_STOP        );
bind2(global,    'p', ' ',                         PLAYER_TOGGLE      );
bind1(global,    '>',                              PLAYLIST_NEXT      );
bind1(global,    '<',                              PLAYLIST_PREV      );
bind3(global,    'l', '}',  '\t',                  TABS_NEXT          );
bind3(global,    'h', '{',          KEY_BTAB,      TABS_PREV          );
bind1(global,    Control('l'),                     REDRAW             );
bind1(global,    'U',                              UPDATE             );
bind1(global,    'q',                              QUIT               );
bind1(global,    'I',                              SHOW_COVER         );

bind3(playlist,  '\n', '\r',        KEY_ENTER,     PLAYLIST_PLAY         );
bind1(playlist,  '$',                              PLAYLIST_DOWNLOAD     );
bind2(playlist,  'g',               KEY_HOME,      TOP                   );
bind2(playlist,  'G',               KEY_END,       BOTTOM                );
bind2(playlist,  'k',               KEY_UP,        UP                    );
bind2(playlist,  'j',               KEY_DOWN,      DOWN                  );
bind2(playlist,  Control('u'),      KEY_PPAGE,     PAGE_UP               );
bind2(playlist,  Control('d'),      KEY_NPAGE,     PAGE_DOWN             );
bind1(playlist,  '/',                              SEARCH_DOWN           );
bind1(playlist,  '?',                              SEARCH_UP             );
bind1(playlist,  'n',                              SEARCH_NEXT           );
bind1(playlist,  'N',                              SEARCH_PREV           );
bind1(playlist,  'o',                              PLAYLIST_GOTO_CURRENT );
bind1(playlist,  'C',                              PLAYLIST_CLEAR        );
bind1(playlist,  'd',                              PLAYLIST_DELETE       );

bind3(browser,  '\n', '\r',        KEY_ENTER,     BROWSER_ENTER         );
bind2(browser,  ' ', 'a',                         BROWSER_ENQUEUE       );
bind2(browser,  'g',               KEY_HOME,      TOP                   );
bind2(browser,  'G',               KEY_END,       BOTTOM                );
bind2(browser,  'k',               KEY_UP,        UP                    );
bind2(browser,  'j',               KEY_DOWN,      DOWN                  );
bind2(browser,  Control('u'),      KEY_PPAGE,     PAGE_UP               );
bind2(browser,  Control('d'),      KEY_NPAGE,     PAGE_DOWN             );
bind1(browser,  '/',                              SEARCH_DOWN           );
bind1(browser,  '?',                              SEARCH_UP             );
bind1(browser,  'n',                              SEARCH_NEXT           );
bind1(browser,  'N',                              SEARCH_PREV           );

bind2(pad,      'k',               KEY_UP,        UP                    );
bind2(pad,      'j',               KEY_DOWN,      DOWN                  );
bind2(pad,      Control('u'),      KEY_PPAGE,     PAGE_UP               );
bind2(pad,      Control('d'),      KEY_NPAGE,     PAGE_DOWN             );
bind2(pad,      'g',               KEY_HOME,      TOP                   );
bind2(pad,      'G',               KEY_END,       BOTTOM                );

Actions::ActionID (&global)[KEY_MAX]   = XArray<KEY_MAX, Actions::ActionID, global_keys_>::Xdata::Values;
Actions::ActionID (&playlist)[KEY_MAX] = XArray<KEY_MAX, Actions::ActionID, playlist_keys_>::Xdata::Values;
Actions::ActionID (&pad)[KEY_MAX]      = XArray<KEY_MAX, Actions::ActionID, pad_keys_>::Xdata::Values;

}
