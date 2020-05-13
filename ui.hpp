#ifndef UI_HPP
#define UI_HPP

#include "lib/algorithm.hpp" // clamp XXX

#include CURSES_INC

#include <string>
#include <cstdio>//XXX

#ifdef __cpp_exceptions
#include <stdexcept>
#endif

#ifndef NDEBUG
#include <cstdio> // sprintf
// Using variadic args (va_list) increases binary size but enables compiler
// warnings with -Wformat. We enable them if not 'explicity' disabled with -NDEBUG.
#define USE_C_VARIADIC_ARGS
#endif

#ifdef USE_C_VARIADIC_ARGS
#include <cstdarg>
#endif

/* Notes:
 * - `waddstr`
 * - Prefer waddnstr(s, strlen(s)) rather waddstr(s), as wadd
 */

namespace UI {

struct Pos {
  int y;
  int x;

  inline Pos()                               noexcept : y(0),   x(0)   {}
  inline Pos(int y, int x)                   noexcept : y(y),   x(x)   {}
  inline Pos(const MEVENT& m)                noexcept : y(m.y), x(m.x) {}
  inline bool operator==(const Pos& p) const noexcept { return y == p.y && x == p.x; }
  inline bool operator!=(const Pos& p) const noexcept { return y != p.y || x != p.x; }
  inline bool operator>=(const Pos& p) const noexcept { return y >= p.y && x >= p.x; }
  inline bool operator<=(const Pos& p) const noexcept { return y <= p.y && x <= p.x; }
  inline bool operator< (const Pos& p) const noexcept { return y <  p.y && x <  p.x; }
  inline bool operator> (const Pos& p) const noexcept { return y <  p.y && x <  p.x; }

#ifndef NDEBUG
  inline operator const char*() noexcept {
    static char _[50];
    sprintf(_, "UI::Pos(%d,%d)", y, x);
    return _;
  }
#endif
};

struct Size {
  int height;
  int width;

  inline Size()                               noexcept : height(0),      width(0)     {}
  inline Size(int height, int width)          noexcept : height(height), width(width) {}
  inline bool operator==(const Size& s) const noexcept { return s.height == height && s.width == width; }
  inline bool operator!=(const Size& s) const noexcept { return s.height != height || s.width != width; }

  inline Size calc(int height, int width) const noexcept
  { return Size(this->height + height, this->width + width); }

#ifndef NDEBUG
  inline operator const char*() noexcept {
    static char _[50];
    sprintf(_, "UI::Size(%d,%d)", height, width);
    return _;
  }
#endif
};

class Widget {
public:
  Pos  pos;
  Size size;
  bool visible;

  virtual void draw() = 0;
  virtual void layout(Pos pos, Size size) = 0;
  virtual void noutrefresh() = 0;
  virtual bool handleKey(int) { return false; }
  virtual bool handleMouse(MEVENT&) { return false; }
  virtual WINDOW* getWINDOW() const noexcept = 0;

  Widget()
  : pos(0,0), size(0,0), visible(true)
  {
  }

  virtual ~Widget() {};
};

/* Drawable: Windows and Pads, they share the same functionality */
struct WidgetDrawable : public Widget {
  WINDOW *win;

  ~WidgetDrawable()
  { if (win) delwin(win); }

  inline WINDOW *getWINDOW() const noexcept
  { return win; }

  inline Pos cursorPos() const noexcept
  { Pos pos; getyx(win, pos.y, pos.x); return pos; }

  // ==========================================================================
  // Stream << operators
  // ==========================================================================

  // char / wchar_t
  inline WidgetDrawable& operator<<(char c) noexcept
  { waddch(win, static_cast<chtype>(c)); return *this; }

  inline WidgetDrawable& operator<<(wchar_t c) noexcept
  { waddnwstr(win, &c, 1); return *this; }

  // const char* / const wchar_t*
  inline WidgetDrawable& operator<<(const char* s) noexcept
  { waddstr(win, s); return *this; }

  inline WidgetDrawable& operator<<(const wchar_t* s) noexcept
  { waddwstr(win, s); return *this; }

  // char[] / wchar_t[] --> `N` may NOT be actual strlen!
  template<size_t N>
  inline WidgetDrawable& operator<<(char (&s)[N]) noexcept
  { waddstr(win, s); return *this; }

  template<size_t N>
  inline WidgetDrawable& operator<<(wchar_t (&s)[N]) noexcept
  { waddwstr(win, s); return *this; }

  // const char[] / const wchar_t[] --> Pretty safe to use `N`
  template<size_t N>
  inline WidgetDrawable& operator<<(const char (&s)[N]) noexcept
  { waddnstr(win, s, N-1); return *this; }

  template<size_t N>
  inline WidgetDrawable& operator<<(const wchar_t (&s)[N]) noexcept
  { waddnwstr(win, s, N-1); return *this; }

  // std::string / std::wstring
  inline WidgetDrawable& operator<<(const std::string& s) noexcept
  { waddnstr(win, s.c_str(), s.size()); return *this; }

  inline WidgetDrawable& operator<<(const std::wstring& s) noexcept
  { waddnwstr(win, s.c_str(), s.size()); return *this; }

  // Integer types
  inline WidgetDrawable& operator<<(int i) noexcept
  { wprintw(win, "%d", i); return *this; }

  inline WidgetDrawable& operator<<(size_t s) noexcept
  { wprintw(win, "%zu", s); return *this; }

  inline WidgetDrawable& operator<<(float f) noexcept
  { wprintw(win, "%f", f); return *this; }

  // add-methods ==============================================================
  inline int addCh(chtype c) noexcept
  { return waddch(win, c); }

  // const char* / const wchar_t*
  inline int addStr(const char* s) noexcept
  { return waddstr(win, s); }

  inline int addStr(const wchar_t* s) noexcept
  { return waddwstr(win, s); }

  // char[] / wchar_t[] --> `N` may NOT be actual strlen!
  template<size_t N>
  inline int addStr(char (&s)[N]) noexcept
  { return waddstr(win, s); }

  template<size_t N>
  inline int addStr(wchar_t (&s)[N]) noexcept
  { return waddwstr(win, s); }

  // const char[] / const wchar_t[] --> Pretty safe to use `N`
  template<size_t N>
  inline int addStr(const char (&s)[N]) noexcept
  { return waddnstr(win, s, N-1); }

  template<size_t N>
  inline int addStr(const wchar_t (&s)[N]) noexcept
  { return waddnwstr(win, s, N-1); }

  // std::string / std::wstring
  inline int addStr(const std::string& s) noexcept
  { return waddnstr(win, s.c_str(), s.size()); }

  inline int addStr(const std::wstring& s) noexcept
  { return waddnwstr(win, s.c_str(), s.size()); }

#ifdef USE_C_VARIADIC_ARGS
#if defined(__GNUC__) || defined(__clang__)
  __attribute__((__format__(__printf__, 2, 3)))
#endif
  int printW(const char* fmt, ...) noexcept {
    va_list ap;
    va_start(ap, fmt);
    int ret = vw_printw(win, fmt, ap); 
    va_end(ap);
    return ret;
  }
#else
  template<typename... Args>
  int printW(const char* fmt, Args... args) noexcept
  { return wprintw(win, fmt, args...); }
#endif

  // mv-methods ===============================================================
  inline int moveCursor(int y, int x) noexcept
  { return wmove(win, y, x); }

  inline int mvAddCh(int y, int x, chtype c) noexcept
  { return mvwaddch(win, y, x, c); }

  // const char* / const wchar_t*
  inline int mvAddStr(int y, int x, const char* s) noexcept
  { return mvwaddstr(win, y, x, s); }

  inline int mvAddStr(int y, int x, const wchar_t* s) noexcept
  { return mvwaddwstr(win, y, x, s); }

  // char[] / wchar_t[] --> `N` may NOT be actual strlen!
  template<size_t N>
  inline int mvAddStr(int y, int x, char (&s)[N]) noexcept
  { return mvwaddstr(win, y, x, s); }

  template<size_t N>
  inline int mvAddStr(int y, int x, wchar_t (&s)[N]) noexcept
  { return mvwaddwstr(win, y, x, s); }

  // std::string / std::wstring
  inline int mvAddStr(int y, int x, const std::string& s) noexcept
  { return mvwaddnstr(win, y, x, s.c_str(), s.size()); }

  inline int mvAddStr(int y, int x, const std::wstring& s) noexcept
  { return mvwaddnwstr(win, y, x, s.c_str(), s.size()); }

#ifdef USE_C_VARIADIC_ARGS
#if defined(__GNUC__) || defined(__clang__)
  __attribute__((__format__(__printf__, 4, 5)))
#endif
  int mvPrintW(int y, int x, const char* fmt, ...) noexcept {
    int ret = ERR;
    if (OK == wmove(win, y, x)) {
      va_list ap;
      va_start(ap, fmt);
      ret = vw_printw(win, fmt, ap); 
      va_end(ap);
    }
    return ret;
  }
#else
  template<typename... Args>
  int mvPrintW(int y, int x, const char* fmt, Args... args) noexcept
  { return mvwprintw(win, y, x, fmt, args...); }
#endif

  // attr-methods =============================================================
  inline int attrSet(unsigned int attrs) noexcept
  { return wattrset(win, attrs); }

  // misc methods =============================================================
  inline int clear() noexcept
  { return wclear(win); }

  inline int erase() noexcept
  { return werase(win); }
};

class Window : public WidgetDrawable {
public:
  Window(Pos pos_ = {0,0}, Size size_ = {1,1})
  {
    pos = pos_;
    size = size_;
    win = newwin(size.height, size.width, pos.y, pos.x);
    if (win) {
      keypad(win, true);
      return;
    }
#ifdef __cpp_exceptions
    throw std::runtime_error("newwin()");
#endif
  }

  void layout(Pos pos, Size size) {
    if (size != this->size) {
      this->size = size;
      wresize(win, size.height, size.width);
    }
    if (pos != this->pos) {
      this->pos = pos;
      mvwin(win, pos.y, pos.x);
    }
    draw();
  }

  void noutrefresh() {
    wnoutrefresh(win);
  }
};

class Pad : public WidgetDrawable {
public:
  Pad(Pos pos_ = {0,0}, Size size_ = {1,1})
  {
    pad_minrow = 0;
    pad_mincol = 0;
    pos = pos_;
    size = size_;
    win = newpad(size.height, size.width);
    if (win) {
      mvwin(win, pos.y, pos.x);
      keypad(win, true);
      return;
    }
#ifdef __cpp_exceptions
    throw std::runtime_error("newpad()");
#endif
  }

  void noutrefresh() {
    pnoutrefresh(win, pad_minrow, pad_mincol, pos.y, pos.x,
        pos.y + size.height - 1, pos.x + size.width - 1);
  }

  void top()        { pad_minrow = 0; noutrefresh(); }
  void bottom()     { pad_minrow = getmaxy(win) - size.height; noutrefresh(); }
  void page_up()    { up(size.height / 2);   }
  void page_down()  { down(size.height / 2); }

  void up(int n = 1) {
    pad_minrow = clamp(pad_minrow - n, 0, getmaxy(win));
    noutrefresh();
  }

  void down(int n = 1) {
    pad_minrow = clamp(pad_minrow + n, 0, getmaxy(win) - size.height);
    noutrefresh();
  }

protected:
  int pad_minrow;
  int pad_mincol;
};

} // namespace UI

#endif
