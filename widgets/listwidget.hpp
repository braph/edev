#ifndef _LISTWIDGET_HPP
#define _LISTWIDGET_HPP

#include "../ui.hpp"
#include "../common.hpp"

#include <sstream>
#include <climits>

template<typename TItem>
class ListItemRenderer {
public:
  ListItemRenderer(int width = 0) : m_width(width) { }

  void setWidth(int width) {
    m_width = width;
  }

  virtual void render(WINDOW *win, const TItem &item, int index, bool cursor, bool active) { // marked, selection
#if TEST_LISTWIDGET
    // A primitive default renderer for testing purposes
    std::stringstream ss; ss << item;
    char marker = ' ';
    if (cursor && active) marker = 'X';
    else if (cursor)      marker = '>';
    else if (active)      marker = 'x';
    waddch(win, marker);
    waddnstr(win, ss.str().c_str(), m_width - 1);
#endif
  }
protected:
  int m_width;
};

template<typename TContainer, typename TRenderer>
class ListWidget : public UI::Window {
public:
  ListWidget(TRenderer renderer)
  : m_cursor(0)
  , m_active(0)
  , m_top_index(0)
  , m_item_renderer(renderer)
  {
  }

  void attachList(TContainer *list) {
    m_list = list;
  }

  void layout(UI::Pos pos, UI::Size size) {
    if (size != this->size) {
      this->size = size;
      wresize(win, size.height, size.width);
    }
    if (pos != this->pos) {
      this->pos = pos;
      mvwin(win, pos.y, pos.x);
    }

    m_item_renderer.setWidth(size.width);
  }

  void render_item(int idx, bool cursor) {
    m_item_renderer.render(win, (*m_list)[idx], idx, cursor, idx == m_active);
  }

  void draw() {
    //werase(win);
    wclear(win);
    if (! m_list) return;
    m_cursor    = clamp(m_cursor,    0, size.height - 1);
    m_top_index = clamp(m_top_index, 0, m_list->size() - 1);
    m_top_index = clamp(m_top_index, 0, m_list->size() - size.height);

    //m_selected = clamp(m_selected, 0, index_last());

    int line = 0;
    int idx  = m_top_index;
    for (; line < size.height && idx < m_list->size(); ++line, ++idx) {
      wmove(win, line, 0);
      //wprintw(win, "%.*s", size.width, "");
      render_item(idx, line == m_cursor);
    }
  }

  void unselect_item() {
    wmove(win, m_cursor, 0);
    render_item(m_top_index + m_cursor, false); // Unselect old line
  }

  /* Cursor down */
  void down() {
    if (m_top_index + m_cursor + 1 >= m_list->size())
      return;

    unselect_item();
    if (m_cursor >= size.height - 1) {
      ++m_top_index;
      append_bottom();
    } else {
      ++m_cursor;
    }
    wmove(win, m_cursor, 0);
    render_item(m_top_index + m_cursor, true);
  }

  void up() {
    if (m_top_index + m_cursor - 1 < 0)
      return;

    unselect_item();
    if (m_cursor == 0) {
      --m_top_index;
      insert_top();
    } else {
      --m_cursor;
    }
    wmove(win, m_cursor, 0);
    render_item(m_top_index + m_cursor, true);
  }

  void scroll_down() {
    int n = 0.5 * size.height;

    m_top_index += n;
    if (m_top_index + size.height - 1 >= m_list->size()) {
      m_top_index = m_list->size() - size.height;
    }

    draw();
  }

  void scroll_up() {
    int n = 0.5 * size.height;

    m_top_index -= n;
    if (m_top_index < 0)
      m_top_index = 0;

    draw();
  }

  void insert_top() { // XXX rename + mv to UI::Window
    wmove(win, 0, 0);
    winsertln(win);
  }

  void append_bottom() { // XXX rename + mv to UI::Window
    wmove(win, 0, 0);
    wdeleteln(win);
  }

  // === Navigation === //
  void top()       { m_cursor = m_top_index = 0;        draw(); }
  void bottom()    { m_cursor = m_top_index = INT_MAX;  draw(); }
  void page_up()   { scroll_up(); }
  void page_down() { scroll_down(); }
  /*
  void page_up()   { scroll_up(size.height); }
  void page_down() { scroll_down(size.height); }
  void up(N)       { scroll_cursor_up(1);      }
  void down(N)     { scroll_cursow_down(1);    }
  //void center()    { force_cursorpos(size.height / 2); }
  */

  int getSelected() { return m_top_index + m_cursor; }
  typename TContainer::value_type getItem() { return (*m_list)[m_top_index+m_cursor]; }

private:
  int m_cursor;
  int m_active;
  int m_top_index;
  TRenderer m_item_renderer;
  TContainer *m_list;
};

// === Testing ================================================================
template<typename TContainer, typename TRenderer>
void testListWidget(
  TContainer &testData,
  TRenderer &renderer
) {
  ListWidget<TContainer, TRenderer> listWidget(renderer);
  listWidget.attachList(&testData);
  listWidget.layout({0,0}, {LINES,COLS});
  listWidget.draw();
  listWidget.noutrefresh();
  doupdate();

  for (;;) {
    switch (wgetch(listWidget.active_win())) {
      case 'k': listWidget.up();        break;
      case 'j': listWidget.down();      break;
      case KEY_PPAGE:
      case 'K': listWidget.page_up();   break;
      case KEY_NPAGE:
      case 'J': listWidget.page_down(); break;
      case 'g': listWidget.top();       break;
      case 'G': listWidget.bottom();    break;
      case 'l': listWidget.draw();      break;
      case 'q': return;
    }
    listWidget.noutrefresh();
    doupdate();
  }
}

template<typename TContainer, typename TRenderer>
void testListItemRenderer(TContainer& container, TRenderer& renderer) {
  renderer.setWidth(COLS);
  int cursor = LINES / 2;

  for (int y = 0; y < container.size(); ++y) {
    if (y >= LINES)
      break;

    wmove(stdscr, y, 0);
    renderer.render(stdscr, container[y], y, y == 3, y == cursor);
  }
  refresh();
  getch();
}
// ============================================================================

#endif
