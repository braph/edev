#ifndef _UI_HPP
#define _UI_HPP

#include "common.hpp"

#include "curses.h"

#include <vector>
#include <iostream>
#include <algorithm>

namespace UI {

struct Pos {
  int y;
  int x;
  inline Pos() : y(0), x(0) {}
  inline Pos(int y, int x) : y(y), x(x) {}
  inline Pos(const MEVENT& m) : y(m.y), x(m.x) {}
  inline bool operator==(const Pos& p) const { return y == p.y && x == p.x; }
  inline bool operator!=(const Pos& p) const { return y != p.y || x != p.x; }
  inline bool operator>=(const Pos& p) const { return y >= p.y && x >= p.x; }
  inline bool operator<=(const Pos& p) const { return y <= p.y && x <= p.x; }
  inline bool operator< (const Pos& p) const { return y <  p.y && x <  p.x; }
  inline bool operator> (const Pos& p) const { return y <  p.y && x <  p.x; }

  inline friend std::ostream& operator<<(std::ostream& os, const UI::Pos& p) {
    os << "UI::Pos(" << p.y << ',' << p.x << ')';
    return os;
  }
};

struct Size {
  int height;
  int width;

  Size() : height(0), width(0) {}
  Size(int height, int width) : height(height), width(width) {}

  inline bool operator==(const Size& s) const
  { return s.height == height && s.width == width; }

  inline bool operator!=(const Size& s) const
  { return s.height != height || s.width != width; }

  inline Size calc(int height, int width) const
  { return Size(this->height + height, this->width + width); }

  inline friend std::ostream& operator<<(std::ostream& os, const UI::Size& s) {
    os << "UI::Size(" << s.height << ',' << s.width << ')';
    return os;
  }
};

template<typename T>
struct MouseEvents {
  struct Rectangle  {
    UI::Pos start, stop;
    inline Rectangle(const UI::Pos& start, const UI::Pos& stop) : start(start), stop(stop) {}
  };

  struct MouseEvent {
    Rectangle section;
    T data;
    inline MouseEvent(const Rectangle& section, const T& data) : section(section), data(data) {}
  };

  std::vector<MouseEvent> events;

  using iterator = typename std::vector<MouseEvent>::iterator;

  inline iterator begin() { return events.begin(); }
  inline iterator end()   { return events.end();   }
  inline void     clear() { events.clear();        }

  iterator find(const Pos& mousePos) {
    return std::find_if(begin(), end(), [&](const MouseEvent& event) {
        return mousePos >= event.section.start && mousePos <= event.section.stop; });
  }

  void add(const Pos& start, const Pos& stop, const T& data) {
    events.push_back(MouseEvent(Rectangle(start, stop), data));
  }
};

class Widget {
public:
  Pos  pos;
  Size size;
  bool visible;

  virtual void    draw() = 0;
  virtual void    layout(Pos pos, Size size) = 0;
  virtual void    noutrefresh() = 0;
  virtual WINDOW* active_win() const = 0;
  virtual bool    handleKey(int) {return false;}
  virtual bool    handleMouse(MEVENT &m) { (void)m; return false; }

  Widget()
  : pos(0,0), size(0,0), visible(true)
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

  WINDOW *active_win() const
  { return win; }

  inline UI::Pos cursorPos()
  { UI::Pos pos; getyx(win, pos.y, pos.x); return pos; }

  // Char
  inline WidgetDrawable& operator<<(char c)
  { waddch(win, static_cast<chtype>(c)); return *this; }

  inline WidgetDrawable& operator<<(wchar_t c)
  { waddnwstr(win, &c, 1); return *this; }

  // String
  inline WidgetDrawable& operator<<(const char* s)
  { waddstr(win, s); return *this; }

  inline WidgetDrawable& operator<<(const wchar_t* s)
  { waddwstr(win, s); return *this; }

  inline WidgetDrawable& operator<<(const std::string& s)
  { waddstr(win, s.c_str()); return *this; }

  inline WidgetDrawable& operator<<(const std::wstring& s)
  { waddwstr(win, s.c_str()); return *this; }

  // Integer types
  inline WidgetDrawable& operator<<(int i)
  { wprintw(win, "%d", i); return *this; }

  inline WidgetDrawable& operator<<(size_t s)
  { wprintw(win, "%lu", s); return *this; }

  inline WidgetDrawable& operator<<(float f)
  { wprintw(win, "%f", f); return *this; }

  inline bool moveCursor(int y, int x)
  { return wmove(win, y, x); }
};

class Window : public WidgetDrawable {
public:
  Window()
  {
    pos = UI::Pos(0,0);
    size = UI::Size(1,1);
    if (! (win = newwin(1, 1, 0, 0)))
      throw std::runtime_error("newwin()");
    keypad(win, true);
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
    pos = UI::Pos(0,0);
    size = UI::Size(1,1);
    if (! (win = newpad(1, 1)))
      throw std::runtime_error("newpad()");
    keypad(win, true);
    pad_minrow = 0;
    pad_mincol = 0;
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
