#ifndef _UI_HPP
#define _UI_HPP

#include "common.hpp"

#include CURSES_INC

#include <vector>
#include <iostream>
#include <algorithm>

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

  bool operator==(const Size& s) const noexcept
  { return s.height == height && s.width == width; }

  bool operator!=(const Size& s) const noexcept
  { return s.height != height || s.width != width; }

  Size calc(int height, int width) const noexcept
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
    Rectangle(const UI::Pos& start, const UI::Pos& stop) : start(start), stop(stop) {}
  };

  struct MouseEvent {
    Rectangle section;
    T data;
    MouseEvent(const Rectangle& section, const T& data) : section(section), data(data) {}
  };

  std::vector<MouseEvent> events;

  using iterator = typename std::vector<MouseEvent>::iterator;

  size_t   size()   noexcept { return events.size();  }
  iterator begin()  noexcept { return events.begin(); }
  iterator end()    noexcept { return events.end();   }
  iterator cbegin() noexcept { return events.begin(); }
  iterator cend()   noexcept { return events.end();   }
  void     clear()  noexcept { events.clear();        }

  iterator find(const Pos& mousePos) noexcept {
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
  virtual WINDOW* getWINDOW() const = 0;
  virtual bool    handleKey(int) {return false;}
  virtual bool    handleMouse(MEVENT &m) { (void)m; return false; }

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

  WINDOW *getWINDOW() const
  { return win; }

  UI::Pos cursorPos() const noexcept
  { UI::Pos pos; getyx(win, pos.y, pos.x); return pos; }

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
  { wprintw(win, "%lu", s); return *this; }

  WidgetDrawable& operator<<(float f) noexcept
  { wprintw(win, "%f", f); return *this; }

  // add-methods ==============================================================
  int addCh(chtype c) noexcept
  { return waddch(win, c); }

  int addStr(const char* s) noexcept
  { return waddstr(win, s); }

  int addStr(const wchar_t* s) noexcept
  { return waddwstr(win, s); }

  template<typename... Args>
  int printW(const char* fmt, Args... args) noexcept
  { return wprintw(win, fmt, args...); }

  // mv-methods ===============================================================
  int moveCursor(int y, int x) noexcept
  { return wmove(win, y, x); }

  int mvAddStr(int y, int x, const char* s) noexcept
  { return mvwaddstr(win, y, x, s); }

  int mvAddStr(int y, int x, const wchar_t* s) noexcept
  { return mvwaddwstr(win, y, x, s); }

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
  Window()
  {
    pos = UI::Pos(0,0);
    size = UI::Size(1,1);
    win = newwin(1, 1, 0, 0);
#ifdef __cpp_exceptions
    if (! win)
      throw std::runtime_error("newwin()");
#endif
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
    win = newpad(1, 1);
#ifdef __cpp_exceptions
    if (! win)
      throw std::runtime_error("newpad()");
#endif
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
