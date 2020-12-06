#include <ncurses.h>
#include <climits> // INT_MAX

#undef  NCURSES_OK_ADDR
#define NCURSES_OK_ADDR(WIN) TRUE

// TODO: printw: to string everything

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

template<class Chr> inline int addch_CPP(Chr ch) { return waddch_generic(stdscr, ch); }
template<class Chr> inline int addch_CPP(WINDOW* w, Chr ch) { return waddch_generic(w, ch); }
template<class Chr> inline int addch_CPP(int y, int x, Chr ch) { return wmove(stdscr, y, x) == ERR ? ERR : waddch_generic(stdscr, ch); }
template<class Chr> inline int addch_CPP(WINDOW* w, int y, int x, Chr ch) { return wmove(w, y, x) == ERR ? ERR : waddch_generic(w, ch); }
inline int getch_CPP() { return wgetch(stdscr); }
inline int getch_CPP(WINDOW* w) { return wgetch(w); }
inline int getch_CPP(int y, int x) { return wmove(stdscr, y, x) == ERR ? ERR : wgetch(stdscr); }
inline int getch_CPP(WINDOW* w, int y, int x) { return wmove(w, y, x) == ERR ? ERR : wgetch(w); }
inline int getch_CPP(wint_t* ch) { return wget_wch(stdscr, ch); }
inline int getch_CPP(WINDOW* w, wint_t* ch) { return wget_wch(w, ch); }
inline int getch_CPP(int y, int x, wint_t* ch) { return wmove(stdscr, y, x) == ERR ? ERR : wget_wch(stdscr, ch); }
inline int getch_CPP(WINDOW* w, int y, int x, wint_t* ch) { return wmove(w, y, x) == ERR ? ERR : wget_wch(w, ch); }
inline int delch_CPP() { return wdelch(stdscr); }
inline int delch_CPP(WINDOW* w) { return wdelch(w); }
inline int delch_CPP(int y, int x) { return wmove(stdscr, y, x) == ERR ? ERR : wdelch(stdscr); }
inline int delch_CPP(WINDOW* w, int y, int x) { return wmove(w, y, x) == ERR ? ERR : wdelch(w); }
template<class Str> inline int addstr_CPP(const Str& s) { return waddnstr_generic(stdscr, cstr(s), len(s)); }
template<class Str> inline int addstr_CPP(WINDOW* w, const Str& s) { return waddnstr_generic(w, cstr(s), len(s)); }
template<class Str> inline int addstr_CPP(int y, int x, const Str& s) { return wmove(stdscr, y, x) == ERR ? ERR : waddnstr_generic(stdscr, cstr(s), len(s)); }
template<class Str> inline int addstr_CPP(WINDOW* w, int y, int x, const Str& s) { return wmove(w, y, x) == ERR ? ERR : waddnstr_generic(w, cstr(s), len(s)); }
template<class Str> inline int addstr_CPP(const Str& s, int n) { return waddnstr_generic(stdscr, cstr(s), n); }
template<class Str> inline int addstr_CPP(WINDOW* w, const Str& s, int n) { return waddnstr_generic(w, cstr(s), n); }
template<class Str> inline int addstr_CPP(int y, int x, const Str& s, int n) { return wmove(stdscr, y, x) == ERR ? ERR : waddnstr_generic(stdscr, cstr(s), n); }
template<class Str> inline int addstr_CPP(WINDOW* w, int y, int x, const Str& s, int n) { return wmove(w, y, x) == ERR ? ERR : waddnstr_generic(w, cstr(s), n); }
template<class Str> inline int insstr_CPP(const Str& s) { return winsnstr_generic(stdscr, cstr(s), len(s)); }
template<class Str> inline int insstr_CPP(WINDOW* w, const Str& s) { return winsnstr_generic(w, cstr(s), len(s)); }
template<class Str> inline int insstr_CPP(int y, int x, const Str& s) { return wmove(stdscr, y, x) == ERR ? ERR : winsnstr_generic(stdscr, cstr(s), len(s)); }
template<class Str> inline int insstr_CPP(WINDOW* w, int y, int x, const Str& s) { return wmove(w, y, x) == ERR ? ERR : winsnstr_generic(w, cstr(s), len(s)); }
template<class Str> inline int insstr_CPP(const Str& s, int n) { return winsnstr_generic(stdscr, cstr(s), n); }
template<class Str> inline int insstr_CPP(WINDOW* w, const Str& s, int n) { return winsnstr_generic(w, cstr(s), n); }
template<class Str> inline int insstr_CPP(int y, int x, const Str& s, int n) { return wmove(stdscr, y, x) == ERR ? ERR : winsnstr_generic(stdscr, cstr(s), n); }
template<class Str> inline int insstr_CPP(WINDOW* w, int y, int x, const Str& s, int n) { return wmove(w, y, x) == ERR ? ERR : winsnstr_generic(w, cstr(s), n); }
template<class Str> inline int getstr_CPP(Str& s) { return wgetnstr_generic(stdscr, cstr(s), len(s)); }
template<class Str> inline int getstr_CPP(WINDOW* w, Str& s) { return wgetnstr_generic(w, cstr(s), len(s)); }
template<class Str> inline int getstr_CPP(int y, int x, Str& s) { return wmove(stdscr, y, x) == ERR ? ERR : wgetnstr_generic(stdscr, cstr(s), len(s)); }
template<class Str> inline int getstr_CPP(WINDOW* w, int y, int x, Str& s) { return wmove(w, y, x) == ERR ? ERR : wgetnstr_generic(w, cstr(s), len(s)); }
template<class Str> inline int getstr_CPP(Str& s, int n) { return wgetnstr_generic(stdscr, cstr(s), n); }
template<class Str> inline int getstr_CPP(WINDOW* w, Str& s, int n) { return wgetnstr_generic(w, cstr(s), n); }
template<class Str> inline int getstr_CPP(int y, int x, Str& s, int n) { return wmove(stdscr, y, x) == ERR ? ERR : wgetnstr_generic(stdscr, cstr(s), n); }
template<class Str> inline int getstr_CPP(WINDOW* w, int y, int x, Str& s, int n) { return wmove(w, y, x) == ERR ? ERR : wgetnstr_generic(w, cstr(s), n); }
inline int hline_CPP(chtype ch, int n) { return whline(stdscr, ch, n); }
inline int hline_CPP(WINDOW* w, chtype ch, int n) { return whline(w, ch, n); }
inline int hline_CPP(int y, int x, chtype ch, int n) { return wmove(stdscr, y, x) == ERR ? ERR : whline(stdscr, ch, n); }
inline int hline_CPP(WINDOW* w, int y, int x, chtype ch, int n) { return wmove(w, y, x) == ERR ? ERR : whline(w, ch, n); }
inline int vline_CPP(chtype ch, int n) { return wvline(stdscr, ch, n); }
inline int vline_CPP(WINDOW* w, chtype ch, int n) { return wvline(w, ch, n); }
inline int vline_CPP(int y, int x, chtype ch, int n) { return wmove(stdscr, y, x) == ERR ? ERR : wvline(stdscr, ch, n); }
inline int vline_CPP(WINDOW* w, int y, int x, chtype ch, int n) { return wmove(w, y, x) == ERR ? ERR : wvline(w, ch, n); }
inline int hline_CPP(const cchar_t* ch, int n) { return whline_set(stdscr, ch, n); }
inline int hline_CPP(WINDOW* w, const cchar_t* ch, int n) { return whline_set(w, ch, n); }
inline int hline_CPP(int y, int x, const cchar_t* ch, int n) { return wmove(stdscr, y, x) == ERR ? ERR : whline_set(stdscr, ch, n); }
inline int hline_CPP(WINDOW* w, int y, int x, const cchar_t* ch, int n) { return wmove(w, y, x) == ERR ? ERR : whline_set(w, ch, n); }
inline int vline_CPP(const cchar_t* ch, int n) { return wvline_set(stdscr, ch, n); }
inline int vline_CPP(WINDOW* w, const cchar_t* ch, int n) { return wvline_set(w, ch, n); }
inline int vline_CPP(int y, int x, const cchar_t* ch, int n) { return wmove(stdscr, y, x) == ERR ? ERR : wvline_set(stdscr, ch, n); }
inline int vline_CPP(WINDOW* w, int y, int x, const cchar_t* ch, int n) { return wmove(w, y, x) == ERR ? ERR : wvline_set(w, ch, n); }
inline int border_CPP(chtype ls, chtype rs, chtype ts, chtype bs, chtype tl, chtype tr, chtype bl, chtype br) { return wborder(stdscr, ls, rs, ts, bs, tl, tr, bl, br); }
inline int border_CPP(WINDOW* w, chtype ls, chtype rs, chtype ts, chtype bs, chtype tl, chtype tr, chtype bl, chtype br) { return wborder(w, ls, rs, ts, bs, tl, tr, bl, br); }
inline int border_CPP(const cchar_t *ls, const cchar_t *rs, const cchar_t *ts, const cchar_t *bs, const cchar_t *tl, const cchar_t *tr, const cchar_t *bl, const cchar_t *br) { return wborder_set(stdscr, ls, rs, ts, bs, tl, tr, bl, br); }
inline int border_CPP(WINDOW* w, const cchar_t *ls, const cchar_t *rs, const cchar_t *ts, const cchar_t *bs, const cchar_t *tl, const cchar_t *tr, const cchar_t *bl, const cchar_t *br) { return wborder_set(w, ls, rs, ts, bs, tl, tr, bl, br); }
inline int box_CPP(WINDOW* w, chtype verch, chtype horch) { return box(w, verch, horch); }
inline int box_CPP(WINDOW* w, const cchar_t *verch, const cchar_t *horch) { return box_set(w, verch, horch); }
inline int erase_CPP() { return werase(stdscr); }
inline int erase_CPP(WINDOW* w) { return werase(w); }
inline int clear_CPP() { return werase(stdscr); }
inline int clear_CPP(WINDOW* w) { return werase(w); }
inline int clrtobot_CPP() { return wclrtobot(stdscr); }
inline int clrtobot_CPP(WINDOW* w) { return wclrtobot(w); }
inline int clrtoeol_CPP() { return wclrtoeol(stdscr); }
inline int clrtoeol_CPP(WINDOW* w) { return wclrtoeol(w); }
inline int getcurx_CPP() { return getcurx(stdscr); }
inline int getcurx_CPP(WINDOW* w) { return getcurx(w); }
inline int getcury_CPP() { return getcury(stdscr); }
inline int getcury_CPP(WINDOW* w) { return getcury(w); }
inline int getbegx_CPP() { return getbegx(stdscr); }
inline int getbegx_CPP(WINDOW* w) { return getbegx(w); }
inline int getbegy_CPP() { return getbegy(stdscr); }
inline int getbegy_CPP(WINDOW* w) { return getbegy(w); }
inline int getmaxx_CPP() { return getmaxx(stdscr); }
inline int getmaxx_CPP(WINDOW* w) { return getmaxx(w); }
inline int getmaxy_CPP() { return getmaxy(stdscr); }
inline int getmaxy_CPP(WINDOW* w) { return getmaxy(w); }
inline int getparx_CPP() { return getparx(stdscr); }
inline int getparx_CPP(WINDOW* w) { return getparx(w); }
inline int getpary_CPP() { return getpary(stdscr); }
inline int getpary_CPP(WINDOW* w) { return getpary(w); }
inline int getyx_CPP(int& y, int& x) { return getyx(stdscr, y, x); }
inline int getyx_CPP(WINDOW* w, int& y, int& x) { return getyx(w, y, x); }
inline int getbegyx_CPP(int& y, int& x) { return getbegyx(stdscr, y, x); }
inline int getbegyx_CPP(WINDOW* w, int& y, int& x) { return getbegyx(w, y, x); }
inline int getmaxyx_CPP(int& y, int& x) { return getmaxyx(stdscr, y, x); }
inline int getmaxyx_CPP(WINDOW* w, int& y, int& x) { return getmaxyx(w, y, x); }
inline int getparyx_CPP(int& y, int& x) { return getparyx(stdscr, y, x); }
inline int getparyx_CPP(WINDOW* w, int& y, int& x) { return getparyx(w, y, x); }
template<class Window> inline bool is_cleared_CPP(Window* w) { return is_cleared(w); }
template<class Window> inline bool is_idcok_CPP(Window* w) { return is_idcok(w); }
template<class Window> inline bool is_idlok_CPP(Window* w) { return is_idlok(w); }
template<class Window> inline bool is_immedok_CPP(Window* w) { return is_immedok(w); }
template<class Window> inline bool is_keypad_CPP(Window* w) { return is_keypad(w); }
template<class Window> inline bool is_leaveok_CPP(Window* w) { return is_leaveok(w); }
template<class Window> inline bool is_nodelay_CPP(Window* w) { return is_nodelay(w); }
template<class Window> inline bool is_notimeout_CPP(Window* w) { return is_notimeout(w); }
template<class Window> inline bool is_pad_CPP(Window* w) { return is_pad(w); }
template<class Window> inline bool is_scrollok_CPP(Window* w) { return is_scrollok(w); }
template<class Window> inline bool is_subwin_CPP(Window* w) { return is_subwin(w); }
template<class Window> inline bool is_syncok_CPP(Window* w) { return is_syncok(w); }
template<class Window> inline int wgetdelay_CPP(Window* w) { return wgetdelay(w); }
template<class Window> inline int wgetscrreg_CPP(Window* w, int* top, int* bottom) { return wgetscrreg(w, top, bottom); }
template<class Window> inline WINDOW* wgetparent_CPP(Window* w) { return wgetparent(w); }

#undef  mvwaddch
#define mvwaddch addch_CPP

#undef  mvaddch
#define mvaddch addch_CPP

#undef  waddch
#define waddch addch_CPP

#undef  addch
#define addch addch_CPP

#undef  mvwgetch
#define mvwgetch getch_CPP

#undef  mvgetch
#define mvgetch getch_CPP

#undef  wgetch
#define wgetch getch_CPP

#undef  getch
#define getch getch_CPP

#undef  mvwdelch
#define mvwdelch delch_CPP

#undef  mvdelch
#define mvdelch delch_CPP

#undef  wdelch
#define wdelch delch_CPP

#undef  delch
#define delch delch_CPP

#undef  mvwaddnwstr
#define mvwaddnwstr addstr_CPP

#undef  mvwaddnstr
#define mvwaddnstr addstr_CPP

#undef  mvwaddwstr
#define mvwaddwstr addstr_CPP

#undef  mvwaddstr
#define mvwaddstr addstr_CPP

#undef  mvaddnwstr
#define mvaddnwstr addstr_CPP

#undef  mvaddnstr
#define mvaddnstr addstr_CPP

#undef  mvaddwstr
#define mvaddwstr addstr_CPP

#undef  mvaddstr
#define mvaddstr addstr_CPP

#undef  waddnwstr
#define waddnwstr addstr_CPP

#undef  waddnstr
#define waddnstr addstr_CPP

#undef  waddwstr
#define waddwstr addstr_CPP

#undef  waddstr
#define waddstr addstr_CPP

#undef  addnwstr
#define addnwstr addstr_CPP

#undef  addnstr
#define addnstr addstr_CPP

#undef  addwstr
#define addwstr addstr_CPP

#undef  addstr
#define addstr addstr_CPP

#undef  mvwinsnstr
#define mvwinsnstr insstr_CPP

#undef  mvwins_nwstr
#define mvwins_nwstr insstr_CPP

#undef  mvwins_wstr
#define mvwins_wstr insstr_CPP

#undef  mvinsnstr
#define mvinsnstr insstr_CPP

#undef  mvins_nwstr
#define mvins_nwstr insstr_CPP

#undef  mvins_wstr
#define mvins_wstr insstr_CPP

#undef  winsnstr
#define winsnstr insstr_CPP

#undef  wins_nwstr
#define wins_nwstr insstr_CPP

#undef  wins_wstr
#define wins_wstr insstr_CPP

#undef  insnstr
#define insnstr insstr_CPP

#undef  ins_nwstr
#define ins_nwstr insstr_CPP

#undef  ins_wstr
#define ins_wstr insstr_CPP

#undef  mvwgetnstr
#define mvwgetnstr getstr_CPP

#undef  mvwget_nwstr
#define mvwget_nwstr getstr_CPP

#undef  mvwget_wstr
#define mvwget_wstr getstr_CPP

#undef  mvgetnstr
#define mvgetnstr getstr_CPP

#undef  mvget_nwstr
#define mvget_nwstr getstr_CPP

#undef  mvget_wstr
#define mvget_wstr getstr_CPP

#undef  wgetnstr
#define wgetnstr getstr_CPP

#undef  wget_nwstr
#define wget_nwstr getstr_CPP

#undef  wget_wstr
#define wget_wstr getstr_CPP

#undef  getnstr
#define getnstr getstr_CPP

#undef  get_nwstr
#define get_nwstr getstr_CPP

#undef  get_wstr
#define get_wstr getstr_CPP

#undef  hline
#define hline hline_CPP

#undef  hline_set
#define hline_set hline_CPP

#undef  vline
#define vline vline_CPP

#undef  vline_set
#define vline_set vline_CPP

#undef  box
#define box box_CPP

#undef  box_set
#define box_set box_CPP

#undef  border
#define border border_CPP

#undef  border_set
#define border_set border_CPP

#undef  wclrtobot
#define wclrtobot clrtobot_CPP

#undef  clrtobot
#define clrtobot clrtobot_CPP

#undef  wclrtoeol
#define wclrtoeol clrtoeol_CPP

#undef  clrtoeol
#define clrtoeol clrtoeol_CPP

#undef  wgetscrreg
#define wgetscrreg wgetscrreg_CPP

#undef  wgetdelay
#define wgetdelay wgetdelay_CPP

#undef  wgetparent
#define wgetparent wgetparent_CPP

#undef  getcurx
#define getcurx getcurx_CPP

#undef  getcury
#define getcury getcury_CPP

#undef  getbegx
#define getbegx getbegx_CPP

#undef  getbegy
#define getbegy getbegy_CPP

#undef  getmaxx
#define getmaxx getmaxx_CPP

#undef  getmaxy
#define getmaxy getmaxy_CPP

#undef  getparx
#define getparx getparx_CPP

#undef  getpary
#define getpary getpary_CPP

#undef  getyx
#define getyx getyx_CPP

#undef  getbegyx
#define getbegyx getbegyx_CPP

#undef  getmaxyx
#define getmaxyx getmaxyx_CPP

#undef  getparyx
#define getparyx getparyx_CPP

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

template<typename... T> int (addstr)(T&&... args)  { return Public::addstr_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (insstr)(T&&... args)  { return Public::insstr_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (getstr)(T&&... args)  { return Public::getstr_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (addch)(T&&... args)  { return Public::addch_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (getch)(T&&... args)  { return Public::getch_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (delch)(T&&... args)  { return Public::delch_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (hline)(T&&... args)  { return Public::hline_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (vline)(T&&... args)  { return Public::vline_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (border)(T&&... args)  { return Public::border_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (box)(T&&... args)  { return Public::box_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (erase)(T&&... args)  { return Public::erase_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (clear)(T&&... args)  { return Public::clear_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (clrtobot)(T&&... args)  { return Public::clrtobot_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (clrtoeol)(T&&... args)  { return Public::clrtoeol_CPP (win, std::forward<T>(args)...); }
template<typename... T> bool (is_cleared)(T&&... args)  { return Public::is_cleared_CPP (win, std::forward<T>(args)...); }
template<typename... T> bool (is_idcok)(T&&... args)  { return Public::is_idcok_CPP (win, std::forward<T>(args)...); }
template<typename... T> bool (is_idlok)(T&&... args)  { return Public::is_idlok_CPP (win, std::forward<T>(args)...); }
template<typename... T> bool (is_immedok)(T&&... args)  { return Public::is_immedok_CPP (win, std::forward<T>(args)...); }
template<typename... T> bool (is_keypad)(T&&... args)  { return Public::is_keypad_CPP (win, std::forward<T>(args)...); }
template<typename... T> bool (is_leaveok)(T&&... args)  { return Public::is_leaveok_CPP (win, std::forward<T>(args)...); }
template<typename... T> bool (is_nodelay)(T&&... args)  { return Public::is_nodelay_CPP (win, std::forward<T>(args)...); }
template<typename... T> bool (is_notimeout)(T&&... args)  { return Public::is_notimeout_CPP (win, std::forward<T>(args)...); }
template<typename... T> bool (is_pad)(T&&... args)  { return Public::is_pad_CPP (win, std::forward<T>(args)...); }
template<typename... T> bool (is_scrollok)(T&&... args)  { return Public::is_scrollok_CPP (win, std::forward<T>(args)...); }
template<typename... T> bool (is_subwin)(T&&... args)  { return Public::is_subwin_CPP (win, std::forward<T>(args)...); }
template<typename... T> bool (is_syncok)(T&&... args)  { return Public::is_syncok_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (wgetdelay)(T&&... args)  { return Public::wgetdelay_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (wgetscrreg)(T&&... args)  { return Public::wgetscrreg_CPP (win, std::forward<T>(args)...); }
template<typename... T> WINDOW* (wgetparent)(T&&... args)  { return Public::wgetparent_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (getcurx)(T&&... args) const { return Public::getcurx_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (getcury)(T&&... args) const { return Public::getcury_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (getbegx)(T&&... args) const { return Public::getbegx_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (getbegy)(T&&... args) const { return Public::getbegy_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (getmaxx)(T&&... args) const { return Public::getmaxx_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (getmaxy)(T&&... args) const { return Public::getmaxy_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (getparx)(T&&... args) const { return Public::getparx_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (getpary)(T&&... args) const { return Public::getpary_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (getyx)(T&&... args) const { return Public::getyx_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (getbegyx)(T&&... args) const { return Public::getbegyx_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (getmaxyx)(T&&... args) const { return Public::getmaxyx_CPP (win, std::forward<T>(args)...); }
template<typename... T> int (getparyx)(T&&... args) const { return Public::getparyx_CPP (win, std::forward<T>(args)...); }
}; // class CursesWindow
} // namespace Public
} // namespace NCursesCPP_Implementation

using namespace NCursesCPP_Implementation::Public;

