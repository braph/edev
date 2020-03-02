#ifndef _LISTWIDGET_HPP
#define _LISTWIDGET_HPP

#include "../ui.hpp"
#include "../common.hpp"

#include <sstream>
#include <climits>
#include <functional>

/* ============================================================================
 * ListWidget - Template for displaying containers as a ncurses list
 * ==========================================================================*/

template<typename TContainer>
class ListWidget : public UI::Window {
public:
  using value_type = typename TContainer::value_type;
  using size_type  = typename TContainer::size_type;

  std::function<void(WINDOW*, int, const value_type&, int, bool, bool)> itemRenderer;

  ListWidget()
  : m_cursor(0)
  , m_active(0)
  , m_top_index(0)
  , m_list(NULL)
  {
  }

  TContainer* getList() const { return m_list; }

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
  }

  void draw() {
    redrawwin(win);
    moveCursor(0,0);

    if (! m_list) return;
    m_cursor    = clamp(m_cursor,    0, size.height - 1);
    m_active    = clamp(m_active,    0, containerSize() - 1);
    m_top_index = clamp(m_top_index, 0, containerSize() - 1);
    m_top_index = clamp(m_top_index, 0, containerSize() - size.height);

    int line = 0;
    int idx  = m_top_index;
    for (; line < size.height && idx < containerSize(); ++line, ++idx) {
      moveCursor(line, 0);
      render_item(idx, line == m_cursor);
    }
  }

  void unselect_item() {
    moveCursor(m_cursor, 0);
    render_item(m_top_index + m_cursor, false); // Unselect old line
  }

  /* Cursor down */
  void down() {
    if (m_top_index + m_cursor + 1 >= containerSize())
      return;

    unselect_item();
    if (m_cursor >= size.height - 1) {
      ++m_top_index;
      append_bottom();
    } else {
      ++m_cursor;
    }
    moveCursor(m_cursor, 0);
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
    moveCursor(m_cursor, 0);
    render_item(m_top_index + m_cursor, true);
  }

  void scroll_down() {
    int n = size.height / 2;

    m_top_index += n;
    if (m_top_index + size.height - 1 >= containerSize()) {
      m_top_index = containerSize() - size.height;
    }

    draw();
  }

  void scroll_up() {
    int n = size.height / 2;

    m_top_index -= n;
    if (m_top_index < 0)
      m_top_index = 0;

    draw();
  }

  void insert_top() { // XXX rename + mv to UI::Window
    moveCursor(0, 0);
    winsertln(win);
  }

  void append_bottom() { // XXX rename + mv to UI::Window
    moveCursor(0, 0);
    wdeleteln(win);
  }

  // === Navigation === //
  void top()       { m_cursor = m_top_index = 0;        draw(); }
  void bottom()    { m_cursor = m_top_index = INT_MAX;  draw(); }
  void page_up()   { scroll_up(); }
  void page_down() { scroll_down(); }
  void gotoSelected() {
    m_cursor = size.height / 2;
    m_top_index = m_active - m_cursor;
    draw();
  }

  /*
  void page_up()   { scroll_up(size.height); }
  void page_down() { scroll_down(size.height); }
  void up(N)       { scroll_cursor_up(1);      }
  void down(N)     { scroll_cursow_down(1);    }
  //void center()    { force_cursorpos(size.height / 2); }
  */

  bool handleMouse(MEVENT& m) {
    if (wmouse_trafo(win, &m.y, &m.x, false)) {
      m_cursor = m.y;
      draw();
      return true;
    }
    return false;
  }

  int getSelected() const { return m_top_index + m_cursor; }

  value_type getItem() const {
    if (! empty())
      return (*m_list)[static_cast<size_type>(m_top_index+m_cursor)];
    throw std::out_of_range("getItem()");
  }

  value_type getActiveItem() const {
    if (! empty())
      return (*m_list)[static_cast<size_type>(m_active)];
    throw std::out_of_range("getActiveItem()");
  }

  int getActiveIndex() const   { return m_active; }
  void setActiveIndex(int idx) { m_active = idx; draw(); }

  inline bool empty() const
  { return containerSize() == 0; }

  inline int containerSize() const
  { return static_cast<int>(m_list ? m_list->size() : 0); }

private:
  int m_cursor;
  int m_active;
  int m_top_index;
  TContainer* m_list;

  inline void render_item(int idx, bool cursor) {
    if (itemRenderer)
      itemRenderer(win, size.width, (*m_list)[static_cast<size_t>(idx)], idx, cursor, idx == m_active);
  }
};

// === Testing ================================================================
template<typename TContainer, typename TRender>
void testListWidget(
  TContainer& testData,
  TRender& render
) {
  ListWidget<TContainer> listWidget;
  listWidget.itemRenderer = render;
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

template<typename TContainer, typename TRender>
void testListItemRenderer(TContainer& container, TRender& render) {
  int cursor = LINES / 2;

  for (int y = 0; y < container.size(); ++y) {
    if (y >= LINES)
      break;

    wmove(stdscr, y, 0);
    render(stdscr, COLS, container[y], y, y == 3, y == cursor);
  }
  refresh();
  getch();
}
// ============================================================================

#endif
