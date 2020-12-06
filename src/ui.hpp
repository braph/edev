#ifndef UI_HPP
#define UI_HPP

#include CURSES_INC
#undef  NCURSES_OK_ADDR // See ncurses.h ...
#define NCURSES_OK_ADDR(WIN) (TRUE)

#include "curs.hpp"

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
  inline bool operator> (const Pos& p) const noexcept { return y >  p.y && x >  p.x; }

  inline Pos calc(int y, int x) const noexcept
  { return Pos(this->y + y, this->x + x); }

#ifndef NDEBUG
  inline operator const char*() const noexcept {
    static char _[32];
    return std::sprintf(_, "UI::Pos(%d,%d)", y, x), _;
  }
#endif
};

struct Size {
  int height;
  int width;

  inline Size()                               noexcept : height(0),        width(0)       {}
  inline Size(int height, int width)          noexcept : height(height),   width(width)   {}
  inline bool operator==(const Size& s) const noexcept { return height == s.height && width == s.width; }
  inline bool operator!=(const Size& s) const noexcept { return height != s.height || width != s.width; }
  inline bool operator>=(const Size& s) const noexcept { return height >= s.height && width >= s.width; }
  inline bool operator<=(const Size& s) const noexcept { return height <= s.height && width <= s.width; }
  inline bool operator< (const Size& s) const noexcept { return height <  s.height && width <  s.width; }
  inline bool operator> (const Size& s) const noexcept { return height >  s.height && width >  s.width; }

  inline Size calc(int height, int width) const noexcept
  { return Size(this->height + height, this->width + width); }

#ifndef NDEBUG
  inline operator const char*() const noexcept {
    static char _[32];
    return std::sprintf(_, "UI::Size(%d,%d)", height, width), _;
  }
#endif
};

struct Rectangle {
  Pos start;
  Pos stop;

  inline Rectangle(const Pos& start, const Pos& stop) noexcept
  : start(start)
  , stop(stop)
  {}

  inline bool encloses(const Pos& pos) const noexcept {
    return pos >= start && pos <= stop;
  }

#ifndef NDEBUG
  inline operator const char*() const noexcept {
    static char _[96];
    return std::sprintf(_, "UI::Rectangle(%d,%d, %d,%d)", start.y, start.x, stop.y, stop.x), _;
  }
#endif
};

class Widget {
public:
  Pos  pos;
  Size size;
  bool visible;

  Widget() noexcept
  : pos(0,0), size(0,0), visible(true)
  {}

  virtual ~Widget()              {};
  virtual void draw()            = 0;
  virtual void layout(Pos, Size) = 0;
  virtual void noutrefresh()     = 0;
  virtual bool handle_key(int)       { return false; }
  virtual bool handle_mouse(MEVENT&) { return false; }
  virtual WINDOW* getWINDOW() const noexcept = 0;
};

/* Drawable: Windows and Pads, they share the same functionality */
struct WidgetDrawable : public Widget, public CursesWindow {
  //WINDOW *win;

  WidgetDrawable() {}

  ~WidgetDrawable()
  { if (win) delwin(win); }

  inline WINDOW *getWINDOW() const noexcept
  { return win; }

  // add-methods ==============================================================
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

  // cursor-methods ===========================================================
  inline int  moveCursor(int y, int x)          noexcept { return wmove(win, y, x); }
  inline Pos  cursorPos()                 const noexcept
  { Pos p; getyx(p.y, p.x); return p; }

  // misc methods =============================================================
  inline int resize(int height, int width) noexcept
  { return wresize(win, height, width); }

  inline int resize(UI::Size new_size) noexcept
  { return wresize(win, new_size.height, new_size.width); }

  inline int setPos(UI::Pos new_pos) noexcept
  { return mvwin(win, new_pos.y, new_pos.x); }
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
  void bottom()     { pad_minrow = getmaxy() - size.height; noutrefresh(); }
  void page_up()    { up(size.height / 2);   }
  void page_down()  { down(size.height / 2); }

  void up(int n = 1) {
    pad_minrow -= n;
    if (pad_minrow < 0)
      pad_minrow = 0;
    else {
      int max_y = getmaxy();
      if (pad_minrow > max_y)
        pad_minrow = max_y;
    }
    noutrefresh();
  }

  void down(int n = 1) {
    pad_minrow += n;
    if (pad_minrow < 0)
      pad_minrow = 0;
    else {
      int max_y = getmaxy() - size.height;
      if (pad_minrow > max_y)
        pad_minrow = max_y;
    }
    noutrefresh();
  }

protected:
  int pad_minrow;
  int pad_mincol;
};

} // namespace UI

#endif
