#include <ncurses.h>
#include <climits> // INT_MAX

#undef  NCURSES_OK_ADDR
#define NCURSES_OK_ADDR(WIN) TRUE

namespace NCursesCPP_Implementation {

// Return a char*/wchar_t*/... from std::string/char*/...
template<class T> inline T*   cstr(T* s)                          { return s;         }
template<class T> inline auto cstr(T& s) -> decltype(T{}.c_str()) { return s.c_str(); }

// For INPUT functions (*inchstr, *getstr).
template<class T, size_t N> inline int capacity(T(&s)[N]) { return N;            }
template<class T>           inline int capacity(T& s)     { return s.capacity(); }

// For OUTPUT functions (*addstr, *insstr).
// Returning INT_MAX instead of T.size() is a little bit faster, but assumes
// that T.c_str() will be NUL-terminated.
template<class T, size_t N> inline int len(const T(&s)[N]) { return N;       }
template<class T>           inline int len(const T&)       { return INT_MAX; }

// OUTPUT functions
inline int waddnstr_generic(WINDOW* w, const char* s, int n)    { return waddnstr(w, s, n);   }
inline int waddnstr_generic(WINDOW* w, const wchar_t* s, int n) { return waddnwstr(w, s, n);  }
inline int winsnstr_generic(WINDOW* w, const char* s, int n)    { return winsnstr(w, s, n);   }
inline int winsnstr_generic(WINDOW* w, const wchar_t* s, int n) { return wins_nwstr(w, s, n); }
// INPUT functions
inline int wgetnstr_generic(WINDOW* w, char* s, int n)          { return wgetnstr(w, s, n);   }
inline int wgetnstr_generic(WINDOW* w, wint_t* s, int n)        { return wgetn_wstr(w, s, n); }
inline int winnstr_generic(WINDOW* w, char* s, int n)           { return winnstr(w, s, n);    }
inline int winnstr_generic(WINDOW* w, chtype* s, int n)         { return winchnstr(w, s, n);  }

namespace Public {
$CURSES_FUNCTIONS

#if defined(USE_C_VARIADIC_ARGS) && (defined(__GNUC__) || defined(__clang__))
__attribute__((__format__(__printf__, 2, 3)))
int __wprintw(const char* fmt, ...) noexcept {
  va_list ap;
  va_start(ap, fmt);
  int ret = vw_printw(win, fmt, ap);
  va_end(ap);
  return ret;
}

__attribute__((__format__(__printf__, 4, 5)))
int __mvwprintw(int y, int x, const char* fmt, ...) noexcept {
  int ret = ERR;
  if (OK == wmove(win, y, x)) {
    va_list ap;
    va_start(ap, fmt);
    ret = vw_printw(win, fmt, ap);
    va_end(ap);
  }
  return ret;
}
#endif

struct CursesWindow {
  WINDOW* win;
  CursesWindow()           : win(NULL) {}
  CursesWindow(WINDOW* w_) : win(w_)   {}
  using K = CursesWindow;

  template<class S>
  inline K& operator<<(const S& s) noexcept { ${prefix}addstr(s);     return *this; }
  inline K& operator<<(char c)     noexcept { waddnstr(win, &c, 1);   return *this; } // XXX don't use waddch, because, eh...
  inline K& operator<<(wchar_t c)  noexcept { waddnwstr(win, &c, 1);  return *this; }
  inline K& operator<<(int i)      noexcept { wprintw(win, "%d", i);  return *this; }
  inline K& operator<<(size_t s)   noexcept { wprintw(win, "%zu", s); return *this; }
  inline K& operator<<(float f)    noexcept { wprintw(win, "%f", f);  return *this; }
  inline K& operator<<(double d)   noexcept { wprintw(win, "%f", d);  return *this; }

  $CURSES_WINDOW_METHODS
};
} // namespace Public
} // namespace NCursesCPP_Implementation

$REDEFS
using namespace NCursesCPP_Implementation::Public;
