template<class TChar> inline const TChar* _cstr(const TChar* s) { return s; }
template<class TStr>  inline auto _cstr(const TStr& s) -> decltype(TStr{}.c_str()) { return s.c_str(); }

//template<class T, size_t N> inline int _slen(const T(&s)[N]) { return N;  }
//template<class T>           inline int _slen(T)              { return -1; }
template<class T> inline int _slen(T)                          { return INT_MAX; }

#define NCURSES_NO_MV_CHECK

#if defined(NCURSES_NO_MV_CHECK)
#define NC_WMOVE(win, y, x, cmd) (wmove((win),(y),(x)), (cmd))
#else
#define NC_WMOVE(win, y, x, cmd) (wmove((win),(y),(x)) == ERR ? ERR : (cmd))
#endif

inline int waddnstr_generic(WINDOW* w, const char* s, int n)      { return waddnstr(w, s, n);  }
inline int waddnstr_generic(WINDOW* w, const wchar_t* s, int n)   { return waddnwstr(w, s, n); }

// strings
template<class Str> inline int addstr_CPP(WINDOW* w, const Str& s)
{ return waddnstr_generic(w, _cstr(s), _slen(s)); }

template<class Str> inline int addstr_CPP(WINDOW* w, const Str& s, int n)
{ return waddnstr_generic(w, _cstr(s), n); }

template<class Str> inline int addstr_CPP(WINDOW* w, int y, int x, const Str& s)
{ return NC_WMOVE(w,y,x, addstr_CPP(w, s)); }

template<class Str> inline int addstr_CPP(WINDOW* w, int y, int x, const Str& s, int n)
{ return NC_WMOVE(w,y,x, addstr_CPP(w, s, n)); }

//   stdscr bindings
template<class Str> inline int addstr_CPP(const Str& s)
{ return waddnstr_generic(stdscr, _cstr(s), _slen(s)); }

template<class Str> inline int addstr_CPP(const Str& s, int n)
{ return waddnstr_generic(stdscr, _cstr(s), n); }

template<class Str> inline int addstr_CPP(int y, int x, const Str& s)
{ return NC_WMOVE(stdscr,y,x, waddnstr_generic(stdscr, _cstr(s), _slen(s))); }

template<class Str> inline int addstr_CPP(int y, int x, const Str& s, int n)
{ return NC_WMOVE(stdscr,y,x, waddnstr_generic(stdscr, _cstr(s), n)); }

// for f in {mv,}{w,}add{n,}{w,}str; do printf "#undef $f\\n#define $f addstr_CPP\\n"; done
#undef mvwaddnwstr
#define mvwaddnwstr addstr_CPP
#undef mvwaddnstr
#define mvwaddnstr addstr_CPP
#undef mvwaddwstr
#define mvwaddwstr addstr_CPP
#undef mvwaddstr
#define mvwaddstr addstr_CPP
#undef mvaddnwstr
#define mvaddnwstr addstr_CPP
#undef mvaddnstr
#define mvaddnstr addstr_CPP
#undef mvaddwstr
#define mvaddwstr addstr_CPP
#undef mvaddstr
#define mvaddstr addstr_CPP
#undef waddnwstr
#define waddnwstr addstr_CPP
#undef waddnstr
#define waddnstr addstr_CPP
#undef waddwstr
#define waddwstr addstr_CPP
#undef waddstr
#define waddstr addstr_CPP
#undef addnwstr
#define addnwstr addstr_CPP
#undef addnstr
#define addnstr addstr_CPP
#undef addwstr
#define addwstr addstr_CPP
#undef addstr
#define addstr addstr_CPP

#if 0
// chars
inline int addch(WINDOW* w, chtype ch)                { return waddch(w, ch); }
inline int addch(chtype ch)                           { return waddch(stdscr, ch); }
inline int addch(int y, int x, chtype ch)             { return waddch(stdscr, y, x, ch); }
inline int addch(WINDOW* w, int y, int x, chtype ch)  { return waddch(w, y, x, ch); }

class Win {
  addstr();
};

int addstr(const char *str);
int addnstr(const char *str, int n);
int waddstr(WINDOW *win, const char *str);
int waddnstr(WINDOW *win, const char *str, int n);
int mvaddstr(int y, int x, const char *str);
int mvaddnstr(int y, int x, const char *str, int n);
int mvwaddstr(WINDOW *win, int y, int x, const char *str);
int mvwaddnstr(WINDOW *win, int y, int x, const char *str, int n);
#endif
