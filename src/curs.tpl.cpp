#include <ncurses.h>
#include <climits> // INT_MAX

#undef  NCURSES_OK_ADDR
#define NCURSES_OK_ADDR(WIN) TRUE

// TODO: printw: to string everything
// TODO: getattrs?
// TODO: attroff/on/set/get
// TODO: most of the time call_args==args!

namespace NCursesCPP_Implementation {

// Return a char*/wchar_t*/... from string object/char pointers/...
template<class T> inline T*   cstr(T* s)                          { return s;         }
template<class T> inline auto cstr(T& s) -> decltype(T{}.c_str()) { return s.c_str(); }

// We are returning INT_MAX because that saves us a call to strlen() inside waddnstr()
template<class T, size_t N> inline int len(const T(&s)[N]) { return N;       }
template<class T>           inline int len(T)              { return INT_MAX; }

inline int waddnstr_generic(WINDOW* w, const char* s, int n)    { return waddnstr(w, s, n);   }
inline int waddnstr_generic(WINDOW* w, const wchar_t* s, int n) { return waddnwstr(w, s, n);  }
inline int waddch_generic(WINDOW* w, const chtype ch)           { return waddch(w, ch);       }
inline int waddch_generic(WINDOW* w, const cchar_t* ch)         { return wadd_wch(w, ch);     }
inline int winsnstr_generic(WINDOW* w, const char* s, int n)    { return winsnstr(w, s, n);   }
inline int winsnstr_generic(WINDOW* w, const wchar_t* s, int n) { return wins_nwstr(w, s, n); }
inline int wgetnstr_generic(WINDOW* w, char* s, int n)          { return wgetnstr(w, s, n);   }
inline int wgetnstr_generic(WINDOW* w, wint_t* s, int n)        { return wgetn_wstr(w, s, n); }

namespace Public {

PYTHON:
mvw("class Chr", "int addch_CPP",  "Chr ch",                    "waddch_generic",    "ch")
mvw("",          "int getch_CPP",  "",                          "wgetch",            "")
mvw("",          "int getch_CPP",  "wint_t* ch",                "wget_wch",          "ch")
mvw("",          "int delch_CPP",  "",                          "wdelch",            "")
mvw("class Str", "int addstr_CPP", "const Str& s",              "waddnstr_generic",  "cstr(s), len(s)")
mvw("class Str", "int addstr_CPP", "const Str& s, int n",       "waddnstr_generic",  "cstr(s), n")
mvw("class Str", "int insstr_CPP", "const Str& s",              "winsnstr_generic",  "cstr(s), len(s)")
mvw("class Str", "int insstr_CPP", "const Str& s, int n",       "winsnstr_generic",  "cstr(s), n")
mvw("class Str", "int getstr_CPP", "Str& s",                    "wgetnstr_generic",  "cstr(s), len(s)")
mvw("class Str", "int getstr_CPP", "Str& s, int n",             "wgetnstr_generic",  "cstr(s), n")
mvw("",          "int hline_CPP",  "chtype ch, int n",          "whline",            "ch, n")
mvw("",          "int vline_CPP",  "chtype ch, int n",          "wvline",            "ch, n")
mvw("",          "int hline_CPP",  "const cchar_t* ch, int n",  "whline_set",        "ch, n")
mvw("",          "int vline_CPP",  "const cchar_t* ch, int n",  "wvline_set",        "ch, n")

w("int border_CPP", "chtype ls, chtype rs, chtype ts, chtype bs, chtype tl, chtype tr, chtype bl, chtype br",
  "wborder", "ls, rs, ts, bs, tl, tr, bl, br")

w("int border_CPP",
    "const cchar_t *ls, const cchar_t *rs, const cchar_t *ts, const cchar_t *bs, const cchar_t *tl, const cchar_t *tr, const cchar_t *bl, const cchar_t *br",
   "wborder_set", "ls, rs, ts, bs, tl, tr, bl, br")

# There is no wbox/wbox_set only box/box_set
f("", "int box_CPP", "WINDOW* w, chtype verch, chtype horch",                 "box", "w, verch, horch")
f("", "int box_CPP", "WINDOW* w, const cchar_t *verch, const cchar_t *horch", "box_set", "w, verch, horch")

w("int erase_CPP",    "", "werase",    "")
w("int clear_CPP",    "", "werase",    "")
w("int clrtobot_CPP", "", "wclrtobot", "")
w("int clrtoeol_CPP", "", "wclrtoeol", "")

# getcurx, getcury, getbegx, getbegy, ...
for func in braceexpand("get{cur,beg,max,par}{x,y}"):
  w(f"int {func}_CPP", "", func, "")

# getyx, getbegyx ...
#//TODO return value should be 'void'
for func in braceexpand("get{,beg,max,par}yx"):
  w(f"int {func}_CPP", "int& y, int& x", func, "y, x")

f("class Window", "bool is_cleared_CPP",   "Window* w", "is_cleared",   "w")
f("class Window", "bool is_idcok_CPP",     "Window* w", "is_idcok",     "w")
f("class Window", "bool is_idlok_CPP",     "Window* w", "is_idlok",     "w")
f("class Window", "bool is_immedok_CPP",   "Window* w", "is_immedok",   "w")
f("class Window", "bool is_keypad_CPP",    "Window* w", "is_keypad",    "w")
f("class Window", "bool is_leaveok_CPP",   "Window* w", "is_leaveok",   "w")
f("class Window", "bool is_nodelay_CPP",   "Window* w", "is_nodelay",   "w")
f("class Window", "bool is_notimeout_CPP", "Window* w", "is_notimeout", "w")
f("class Window", "bool is_pad_CPP",       "Window* w", "is_pad",       "w")
f("class Window", "bool is_scrollok_CPP",  "Window* w", "is_scrollok",  "w")
f("class Window", "bool is_subwin_CPP",    "Window* w", "is_subwin",    "w")
f("class Window", "bool is_syncok_CPP",    "Window* w", "is_syncok",    "w")

f("class Window", "int wgetdelay_CPP",     "Window* w", "wgetdelay",    "w")
f("class Window", "int wgetscrreg_CPP",    "Window* w, int* top, int* bottom", "wgetscrreg", "w, top, bottom")
f("class Window", "WINDOW* wgetparent_CPP","Window* w", "wgetparent",   "w")

# TODO!!!! Window class breaks if we drop these
redef("{mv,}{w,}addch",             "addch_CPP")
redef("{mv,}{w,}getch",             "getch_CPP")
redef("{mv,}{w,}delch",             "delch_CPP")
redef("{mv,}{w,}add{n,}{w,}str",    "addstr_CPP")
redef("{mv,}{w,}ins{n,_nw,_w}str",  "insstr_CPP")
redef("{mv,}{w,}get{n,_nw,_w}str",  "getstr_CPP")
redef("hline{,_set}",               "hline_CPP")
redef("vline{,_set}",               "vline_CPP")
redef("box{,_set}",                 "box_CPP")
redef("border{,_set}",              "border_CPP")
#redef("clear",                     "clear_CPP")
#redef("erase",                     "erase_CPP")
redef("{w,}clrtobot",               "clrtobot_CPP")
redef("{w,}clrtoeol",               "clrtoeol_CPP")
redef("wgetscrreg",                 "wgetscrreg_CPP")
redef("wgetdelay",                  "wgetdelay_CPP")
redef("wgetparent",                 "wgetparent_CPP")

for func in braceexpand("get{cur,beg,max,par}{x,y}"): redef(func, f"{func}_CPP")
for func in braceexpand("get{,beg,max,par}yx"):       redef(func, f"{func}_CPP")
NO_PYTHON:

/*
int refresh(void);
int wrefresh(WINDOW *win);
int wnoutrefresh(WINDOW *win);
int doupdate(void);
int redrawwin(WINDOW *win);
int wredrawln(WINDOW *win, int beg_line, int num_lines);
*/

struct CursesWindow {
  WINDOW* win;
  CursesWindow()           : win(NULL) {}
  CursesWindow(WINDOW* w_) : win(w_)   {}
  using Klass = CursesWindow;

  template<class S>
  inline Klass& operator<<(const S& s) noexcept { Public::addstr(win, s); return *this; }
  inline Klass& operator<<(char c)     noexcept { Public::addch(win, static_cast<chtype>(c)); return *this; }
  inline Klass& operator<<(wchar_t c)  noexcept { Public::waddnwstr(win, &c, 1); return *this; }
  inline Klass& operator<<(int i)      noexcept { wprintw(win, "%d", i);  return *this; }
  inline Klass& operator<<(size_t s)   noexcept { wprintw(win, "%zu", s); return *this; }
  inline Klass& operator<<(float f)    noexcept { wprintw(win, "%f", f);  return *this; }
  inline Klass& operator<<(double d)   noexcept { wprintw(win, "%f", d);  return *this; }

PYTHON:
def __bind_function(RET, NAME, const=""):
  print(f"template<typename... T> {RET} ({NAME})(T&&... args) {const} {{ return Public::{NAME}_CPP (win, std::forward<T>(args)...); }}")

__bind_function("int", "addstr")
__bind_function("int", "insstr")
__bind_function("int", "getstr")
__bind_function("int", "addch")
__bind_function("int", "getch")
__bind_function("int", "delch")

__bind_function("int", "hline")
__bind_function("int", "vline")
__bind_function("int", "border")
__bind_function("int", "box")

__bind_function("int", "erase")
__bind_function("int", "clear")
__bind_function("int", "clrtobot")
__bind_function("int", "clrtoeol")

__bind_function("bool", "is_cleared", "const")
__bind_function("bool", "is_idcok", "const")
__bind_function("bool", "is_idlok", "const")
__bind_function("bool", "is_immedok", "const")
__bind_function("bool", "is_keypad", "const")
__bind_function("bool", "is_leaveok", "const")
__bind_function("bool", "is_nodelay", "const")
__bind_function("bool", "is_notimeout", "const")
__bind_function("bool", "is_pad", "const")
__bind_function("bool", "is_scrollok", "const")
__bind_function("bool", "is_subwin", "const")
__bind_function("bool", "is_syncok", "const")

__bind_function("int", "wgetdelay", "const")
__bind_function("int", "wgetscrreg", "const")
__bind_function("WINDOW*", "wgetparent", "const")

for func in braceexpand("get{cur,beg,max,par}{x,y}"): __bind_function("int", func, "const")
for func in braceexpand("get{,beg,max,par}yx"):       __bind_function("int", func, "const")
NO_PYTHON:
}; // class CursesWindow
} // namespace Public
} // namespace NCursesCPP_Implementation

using namespace NCursesCPP_Implementation::Public;

