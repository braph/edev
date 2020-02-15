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

  Widget(Pos pos = {0,0}, Size size = {0,0}, bool visible = true)
  : pos(pos), size(size), visible(visible)
  {
  }

#if PEDANTIC_FREE
  virtual ~Widget() {};
#endif
};

class Window : public Widget {
public:
  Window() {
    win = newwin(0, 0, 0, 0);
    keypad(win, true);
    getmaxyx(win, size.height, size.width);
  }

#if PEDANTIC_FREE
 ~Window() {
   delwin(win);
 }
#endif

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

  WINDOW *active_win() { return win; }

protected:
  WINDOW *win;
};

class Pad : public Widget {
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

#if PEDANTIC_FREE
 ~Pad() {
   delwin(win);
 }
#endif

  void noutrefresh() {
    pnoutrefresh(win, pad_minrow, pad_mincol, pos.y, pos.x,
        pos.y + size.height - 1, pos.x + size.width - 1);
  }

  WINDOW *active_win() { return win; }

protected:
  WINDOW *win;
  int  pad_minrow;
  int  pad_mincol;
};

} // namespace UI

#endif
