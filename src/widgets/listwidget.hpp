#ifndef WIDGETS_LISTWIDGET_HPP
#define WIDGETS_LISTWIDGET_HPP

#include "../ui.hpp"
#include <lib/algorithm.hpp> // clamp

#include <sstream>
#include <climits>
#include <functional>

// TODO: Clamp active index to -1, MAX

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
  , m_selected(0)
  , m_active(-1)
  , m_list(NULL)
  {
    idlok(win, TRUE);
    scrollok(win, TRUE);
  }

  TContainer* list()          const noexcept { return m_list;          }
  void list(TContainer *list)       noexcept { m_list = list;          }

  int  active_index()         const noexcept { return m_active;        }
  void active_index(int idx)        noexcept { m_active = idx; draw(); }

  int cursor_index() const noexcept
  { return empty() ? -1 : m_selected; }

  void cursor_index(int idx) {
    m_selected = idx;
    m_cursor = size.height / 2;
    draw();
  }

  /// Only if cursor_index() != -1
  value_type getCursorItem() const {
    return (*m_list)[size_type(m_selected)];
  }

  /// Only if active_index() != -1
  value_type active_item() const {
    return (*m_list)[size_type(m_active)];
  }

  void layout(UI::Pos pos, UI::Size size) override {
    if (size != this->size) {
      this->size = size;
      wresize(win, size.height, size.width);
    }
    if (pos != this->pos) {
      this->pos = pos;
      mvwin(win, pos.y, pos.x);
    }
  }

  void _clamp() {
    m_cursor   = clamp(m_cursor,    0, max_cursor());
    m_cursor   = clamp(m_cursor,    0, container_size() - 1);
    m_active   = clamp(m_active,   -1, container_size() - 1);
    m_selected = clamp(m_selected,  0, container_size() - 1);
  }

  void draw() override {
    erase();
    _clamp();

    if (empty())
      return;

    int idx  =  clamp(m_selected - m_cursor, 0, container_size() - 1); // TODO
    int line = 0;
    for (; line < size.height && idx < container_size(); ++line, ++idx) {
      render_item(idx, line, line == m_cursor);
    }

    redrawwin(win);
  }

  /// Scroll and change cursor position
  void scroll_cursor(int n) {
    if (empty())
      return;

    n = clamp(m_selected + n, 0, container_size() - 1) - m_selected;

    render_item(m_selected, m_cursor, false);

    // New cursor resides in the current window
    if (m_cursor + n >= 0 && m_cursor + n < size.height)
      goto CHANGE_CURSOR_POSITION;

    if (n > 0) { // Scrolling down
      m_selected += max_cursor() - m_cursor;
      n -= max_cursor() - m_cursor;
      wscrl(win, n);
      while (n-- > 1)
        render_item(++m_selected, max_cursor(), false);
      render_item(++m_selected, max_cursor(), true);
      return;
    }
    else { // Scrolling up
      m_selected -= m_cursor;
      n += m_cursor;
      wscrl(win, n);
      while (n++ < -1)
        render_item(--m_selected, 0, false);
      render_item(--m_selected, 0, true);
      return;
    }

REDRAW:
    m_cursor   += n;
    m_selected += n;
    draw();
    return;
CHANGE_CURSOR_POSITION:
    m_cursor   += n;
    m_selected += n;
    render_item(m_selected, m_cursor, true);
  }

  void scroll_items(int n) {
    m_selected += n;
    draw();
  }

  // === Navigation === //
  void up(int n = 1)   { scroll_cursor(-n);                        }
  void down(int n = 1) { scroll_cursor(n);                         }
  void top()           { m_cursor = m_selected = 0;        draw(); }
  void bottom()        { m_cursor = m_selected = INT_MAX;  draw(); }
  void page_up()       { scroll_items(-size.height / 2);           }
  void page_down()     { scroll_items(size.height / 2);            }
  void goto_selected() {
    m_cursor = size.height / 2;
    m_selected = m_active;
    draw();
  }

  /*
  void center()    { force_cursorpos(size.height / 2); }
  */

  bool handle_mouse(MEVENT& m) override {
    if (wmouse_trafo(win, &m.y, &m.x, false)) {
      scroll_cursor(m.y - m_cursor);
      return true;
    }
    return false;
  }

  bool empty() const noexcept
  { return container_size() == 0; }

  int container_size() const noexcept
  { return int(m_list ? m_list->size() : 0); }

private:
  int m_cursor;
  int m_selected;
  int m_active;
  TContainer* m_list;

  inline int max_cursor() { return size.height - 1; }

  inline void render_item(int item_idx, int line, bool cursor) {
    if (! itemRenderer)
      return;
    moveCursor(line, 0);
    itemRenderer(win, size.width, (*m_list)[size_t(item_idx)], item_idx, cursor, item_idx == m_active);
  }

  inline void unselect_item() {
    render_item(m_selected, m_cursor, false);
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
  listWidget.list(&testData);
  listWidget.layout({0,0}, {LINES,COLS});
  listWidget.draw();
  listWidget.noutrefresh();
  doupdate();

  for (;;) {
    switch (wgetch(listWidget.getWINDOW())) {
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
