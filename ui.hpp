#ifndef _UI_HPP
#define _UI_HPP

#include "curses.h"

namespace UI {
  class Size {
    public:
      unsigned int width;
      unsigned int height;

      Size(unsigned int _width, unsigned int _height)
        : width(_width), height(_height)
      {
      }

      Size()
        : width(0), height(0)
      {
      }

      /*
      def update(width: nil, height: nil)
         Size.new(width: (width or @width), height: (height or @height))
      end

      def to_s;  "[(Size) height=#{height}, width=#{width}]"  end
      */

      Size calc(int _width, int _height)
      {
        return Size(width + _width, height + _height);
      }

      bool operator==(const Size&rhs) {
        return rhs.height == height && rhs.width == width;
      }
  };

  class Pos {
    public:
      int x;
      int y;
  };

  class Window {
    protected:
      WINDOW *win;
      Size size;
      int flags;

    public:
      Window() : flags(0) {
        win = newwin(0, 0, 0, 0);
        getmaxyx(win, size.height, size.width);
      }

      void refresh() {
        wrefresh(win);
      }
  };

  class Pad {
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
  };
}

#endif
