#ifndef _UI_HPP
#define _UI_HPP

#include "unistd.h"
#include "curses.h"

#include <iostream> // XXX

namespace UI {

struct Size {
  enum { KEEP = -1 };
  int height;
  int width;

  Size()                      : height(0),      width(0)     { }
  Size(int height, int width) : height(height), width(width) { }

  bool operator==(const Size& s) const { return s.height == height && s.width == width; }
  bool operator!=(const Size& s) const { return s.height != height || s.width != width; }

  Size calc(int height, int width) {
    return Size(this->height + height, this->width + width);
  }

  Size duplicate(int height, int width = KEEP) {
    return Size(
        height == KEEP ? this->height : height,
        width  == KEEP ? this->width  : width
    );
  }
  //def to_s;  "[(Size) height=#{height}, width=#{width}]"  end
};

struct Pos {
  int y;
  int x;

  bool operator==(const Pos& p) const { return p.y == y && p.x == x; }
  bool operator!=(const Pos& p) const { return p.y != y || p.x != x; }
  // TODO: is rhs right?
  //bool operator>=(const Pos& p) const { return x >= p.x && y >= p.y; }
  //bool operator<=(const Pos& p) const { return x <= p.x && y <= p.y; }
};

class Widget {
public:
  Pos  pos;
  Size size;
  bool visible;
  virtual void    draw() = 0;
  virtual void    layout(Pos pos, Size size) = 0;
  virtual void    noutrefresh() = 0;
  virtual WINDOW* active_win() = 0;
  virtual bool    handleMouse(MEVENT &m) { (void)m; return false; }

  Widget(Pos pos = {0,0}, Size size = {0,0}, bool visible = true)
  : pos(pos), size(size), visible(visible)
  {
  }

#ifndef NDEBUG
  virtual ~Widget() {};
#endif
};

/* Drawable: Windows and Pads, they share the same functionality */
struct WidgetDrawable : public Widget {
  WINDOW *win;

#ifndef NDEBUG
  ~WidgetDrawable()
  { delwin(win); }
#endif

  WINDOW *active_win()
  { return win; }

  inline WidgetDrawable& operator<<(char c)
  { waddch(win, c); return *this; }

  inline WidgetDrawable& operator<<(char* s)
  { waddstr(win, s); return *this; }

  inline WidgetDrawable& operator<<(const char* s)
  { waddstr(win, s); return *this; }

  inline WidgetDrawable& operator<<(wchar_t c)
  { waddnwstr(win, &c, 1); return *this; }

  inline WidgetDrawable& operator<<(wchar_t* s)
  { waddwstr(win, s); return *this; }

  inline WidgetDrawable& operator<<(const wchar_t* s)
  { waddwstr(win, s); return *this; }

  inline WidgetDrawable& operator<<(const std::string& s)
  { waddstr(win, s.c_str()); return *this; }

  inline WidgetDrawable& operator<<(const std::wstring& s)
  { waddwstr(win, s.c_str()); return *this; }

  template<typename T>
  inline WidgetDrawable& operator<<(T value)
  { return (*this << std::to_string(value)); }
};

class Window : public WidgetDrawable {
public:
  Window() {
    win = newwin(0, 0, 0, 0);
    keypad(win, true);
    getmaxyx(win, size.height, size.width);
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
  Pad() {
    win = newpad(1,1);
    keypad(win, true);
    size.height = 1;
    size.width  = 1;
    pos.x = pos.y = 0;
    pad_minrow = 0;
    pad_mincol = 0;
  }

  void noutrefresh() {
    pnoutrefresh(win, pad_minrow, pad_mincol, pos.y, pos.x,
        pos.y + size.height - 1, pos.x + size.width - 1);
  }

protected:
  int pad_minrow;
  int pad_mincol;
};

} // namespace UI

#endif
