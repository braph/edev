#ifndef _UI_HPP
#define _UI_HPP

#include "unistd.h"
#include "curses.h"

namespace UI {
  struct Size {
    enum { KEEP = -1 };
    int height;
    int width;

    Size()                      : height(0),      width(0)     { }
    Size(int height, int width) : height(height), width(width) { }

    bool operator==(const Size&rhs) {
      return rhs.height == height && rhs.width == width;
    }

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
  };

  class Window : public Widget {
  protected:
    WINDOW *win;

  public:
    Window() {
      win = newwin(0, 0, 0, 0);
      getmaxyx(win, size.height, size.width);
    }

    void layout(Pos pos, Size size) {
      this->pos = pos;
      this->size = size;
      wresize(win, size.height, size.width);
      mvwin(win, pos.y, pos.x);
    }

    void noutrefresh() {
      wnoutrefresh(win);
    }

    WINDOW *active_win() { return win; }
  };

  class Pad : public Widget {
  protected:
    WINDOW *win;
    int  pad_minrow;
    int  pad_mincol;

  public:
    Pad() {
      win = newpad(1,1);
      size.height = 1;
      size.width  = 1;
      pos.x = pos.y = 0;
      pad_minrow = 0;
      pad_mincol = 0;
    }

    void setSize(int y, int x) {
      size.height = y;
      size.width  = x;
    }

    void noutrefresh() {
      pnoutrefresh(win, pad_minrow, pad_mincol, pos.y, pos.x,
          pos.y + size.height - 1, pos.x + size.width - 1);
    }

    WINDOW *active_win() { return win; }
  };
}

#endif
