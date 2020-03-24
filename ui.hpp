#ifndef UI_HPP
#define UI_HPP

#include "lib/algorithm.hpp" // clamp

#include CURSES_INC

#include <ostream>

namespace UI {

struct Pos {
  int y;
  int x;
  Pos() : y(0), x(0) {}
  Pos(int y, int x) : y(y), x(x) {}
  Pos(const MEVENT& m) : y(m.y), x(m.x) {}
  bool operator==(const Pos& p) const noexcept { return y == p.y && x == p.x; }
  bool operator!=(const Pos& p) const noexcept { return y != p.y || x != p.x; }
  bool operator>=(const Pos& p) const noexcept { return y >= p.y && x >= p.x; }
  bool operator<=(const Pos& p) const noexcept { return y <= p.y && x <= p.x; }
  bool operator< (const Pos& p) const noexcept { return y <  p.y && x <  p.x; }
  bool operator> (const Pos& p) const noexcept { return y <  p.y && x <  p.x; }

  inline friend std::ostream& operator<<(std::ostream& o, const Pos& p) {
    return o << "UI::Pos(" << p.y << ',' << p.x << ')';
  }
};

struct Size {
  int height;
  int width;

  Size() : height(0), width(0) {}
  Size(int height, int width) : height(height), width(width) {}

  bool operator==(const Size& s) const noexcept
  { return s.height == height && s.width == width; }

  bool operator!=(const Size& s) const noexcept
  { return s.height != height || s.width != width; }

  Size calc(int height, int width) const noexcept
  { return Size(this->height + height, this->width + width); }

  inline friend std::ostream& operator<<(std::ostream& o, const Size& s) {
    return o << "UI::Size(" << s.height << ',' << s.width << ')';
  }
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
  virtual WINDOW* getWINDOW() const = 0;

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

  WINDOW *getWINDOW() const noexcept
  { return win; }

  Pos cursorPos() const noexcept
  { Pos pos; getyx(win, pos.y, pos.x); return pos; }

  // Char
  WidgetDrawable& operator<<(char c) noexcept
  { waddch(win, static_cast<chtype>(c)); return *this; }

  WidgetDrawable& operator<<(wchar_t c) noexcept
  { waddnwstr(win, &c, 1); return *this; }

  // String
  WidgetDrawable& operator<<(const char* s) noexcept
  { waddstr(win, s); return *this; }

  WidgetDrawable& operator<<(const wchar_t* s) noexcept
  { waddwstr(win, s); return *this; }

  WidgetDrawable& operator<<(const std::string& s) noexcept
  { waddstr(win, s.c_str()); return *this; }

  WidgetDrawable& operator<<(const std::wstring& s) noexcept
  { waddwstr(win, s.c_str()); return *this; }

  // Integer types
  WidgetDrawable& operator<<(int i) noexcept
  { wprintw(win, "%d", i); return *this; }

  WidgetDrawable& operator<<(size_t s) noexcept
  { wprintw(win, "%zu", s); return *this; }

  WidgetDrawable& operator<<(float f) noexcept
  { wprintw(win, "%f", f); return *this; }

  // add-methods ==============================================================
  int addCh(chtype c) noexcept
  { return waddch(win, c); }

  int addStr(const char* s) noexcept
  { return waddstr(win, s); }

  int addStr(const std::string& s) noexcept
  { return waddstr(win, s.c_str()); }

  int addStr(const wchar_t* s) noexcept
  { return waddwstr(win, s); }

  int addStr(const std::wstring& s) noexcept
  { return waddwstr(win, s.c_str()); }

  template<typename... Args>
  int printW(const char* fmt, Args... args) noexcept
  { return wprintw(win, fmt, args...); }

  // mv-methods ===============================================================
  int moveCursor(int y, int x) noexcept
  { return wmove(win, y, x); }

  int mvAddStr(int y, int x, const char* s) noexcept
  { return mvwaddstr(win, y, x, s); }

  int mvAddStr(int y, int x, const std::string& s) noexcept
  { return mvwaddstr(win, y, x, s.c_str()); }

  int mvAddStr(int y, int x, const wchar_t* s) noexcept
  { return mvwaddwstr(win, y, x, s); }

  int mvAddStr(int y, int x, const std::wstring& s) noexcept
  { return mvwaddwstr(win, y, x, s.c_str()); }

  int mvAddCh(int y, int x, chtype c) noexcept
  { return mvwaddch(win, y, x, c); }

  template<typename... Args>
  int mvPrintW(int y, int x, const char* fmt, Args... args) noexcept
  { return mvwprintw(win, y, x, fmt, args...); }

  // attr-methods =============================================================
  int attrSet(unsigned int attrs) noexcept
  { return wattrset(win, attrs); }

  // misc methods =============================================================
  int clear() noexcept
  { return wclear(win); }

  int erase() noexcept
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

  bool handleKey(int n) {
#define __C(K) (K%32)
#define true false // TODO
    switch (n) {
    case 'k': case KEY_UP:   up();   return true;
    case 'j': case KEY_DOWN: down(); return true;
    case __C('u'): case KEY_PPAGE: page_up(); return true;
    case __C('d'): case KEY_NPAGE: page_down(); return true;
    case 'g': top(); return true;
    case 'G': bottom(); return true;
    }
#undef true
    return false;
  }

protected:
  int pad_minrow;
  int pad_mincol;
};

} // namespace UI

#endif
