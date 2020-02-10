#ifndef _UI_HPP
#define _UI_HPP

#include "unistd.h"
#include "curses.h"

namespace UI {
  struct Size {
    int height;
    int width;

    Size(int height, int width) : height(height), width(width) { }
    Size() :                      height(0),      width(0)     { }

    bool operator==(const Size&rhs) {
      return rhs.height == height && rhs.width == width;
    }

    Size calc(int _width, int _height)
    {
      return Size(width + _width, height + _height);
    }

    /*
    def update(width: nil, height: nil)
       Size.new(width: (width or @width), height: (height or @height))
    end

    def to_s;  "[(Size) height=#{height}, width=#{width}]"  end
    */

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
    virtual void    refresh() = 0;
    virtual WINDOW* active_win() = 0;

    Widget(Pos pos = {0,0}, Size size = {0,0}, bool visible = true)
    : pos(pos), size(size), visible(visible)
    {
    }
  };

  class Window : public Widget {
  protected:
    WINDOW *win;
    int flags;

  public:
    Window() : flags(0) {
      win = newwin(0, 0, 0, 0);
      getmaxyx(win, size.height, size.width);
    }

    void layout(Pos pos, Size size) {
      this->pos = pos;
      this->size = size;
      wresize(win, size.height, size.width);
      mvwin(win, pos.y, pos.x);
    }

    void refresh() {
      wrefresh(win);
    }

    WINDOW *active_win() { return win; }
  };

  class Pad : public Widget {
    protected:
      WINDOW *win;
      Size size;
      Pos  pos;
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

      void refresh() {
        prefresh(win, pad_minrow, pad_mincol, pos.y, pos.x,
            pos.y + size.height - 1, pos.x + size.width - 1);
      }

      WINDOW *active_win() { return win; }
  };
}

#endif
