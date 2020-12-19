#ifndef WIDGETS_LISTWIDGET_HPP
#define WIDGETS_LISTWIDGET_HPP

#include "../ui/core.hpp"
#include <lib/algorithm.hpp> // clamp

#include <sstream>
#include <climits>
#include <functional>
#include <algorithm>

// TODO: center->center_cursor, scroll_{items,cursor}(optim.), cursor_index(keep curs)

/* ============================================================================
 * ListWidget - Template for displaying containers as a ncurses list
 * ==========================================================================*/

template<typename TContainer>
class ListWidget : public UI::Window {
  int m_cursor;
  int m_top_list_idx;
  int m_active;
  TContainer* m_list;

public:
  using value_type = typename TContainer::value_type;
  using size_type  = typename TContainer::size_type;

  std::function<void(WINDOW*, int, const value_type&, int, bool, bool)> itemRenderer;

  ListWidget()
  : m_cursor(0)
  , m_top_list_idx(0)
  , m_active(-1)
  , m_list(NULL)
  {
    idlok(win, TRUE);
    scrollok(win, TRUE);
  }

  // === navigation ===========================================================
  inline void up(int n = 1)    { scroll_cursor(-n);                           }
  inline void down(int n = 1)  { scroll_cursor(n);                            }
  inline void page_up()        { scroll_items(-size.height / 2);              }
  inline void page_down()      { scroll_items(+size.height / 2);              }
  inline void top()            { m_cursor = m_top_list_idx = 0;       draw(); }
  inline void bottom()         { m_cursor = m_top_list_idx = INT_MAX; draw(); }
  inline void center()         { m_cursor = size.height / 2;          draw(); }
  inline void goto_selected()  {
    cursor_index(m_active);
    center();
  }

  // === accessors ============================================================
  TContainer* list()          const noexcept { return m_list; }
  void list(TContainer *list)       noexcept { m_list = list; }

  inline bool empty() const noexcept
  { return container_size() == 0; }

  inline int container_size() const noexcept
  { return int(m_list ? m_list->size() : 0); }

  // === cursor_* =============================================================
  inline int cursor_index() const noexcept
  { return empty() ? -1 : m_top_list_idx + m_cursor; }

  inline void cursor_index(int idx) {
    m_top_list_idx = idx;
    m_cursor = 0;
    draw();
  }

  /// Only if cursor_index() != -1
  value_type cursor_item() const {
    return (*m_list)[size_type(cursor_index())];
  }

  // === active_* =============================================================
  int  active_index()         const noexcept { return m_active;        }
  void active_index(int idx)        noexcept { m_active = idx; draw(); }

  /// Only if active_index() != -1
  value_type active_item() const {
    return (*m_list)[size_type(m_active)];
  }

  // === widget stuff =========================================================

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

  void draw() override {
    erase();
    _clamp();

    if (empty())
      return;

    int line = 0;
    int max_line = max_cursor();
    int list_idx = m_top_list_idx;
    int end_list_idx = container_size();
    for (; line <= max_line && list_idx < end_list_idx; ++list_idx, ++line)
      render_item(list_idx, line, line == m_cursor);

    redrawwin(win);
  }

  void scroll_cursor(int n) {
    m_cursor += n;
    if (m_cursor < 0) {
      m_top_list_idx += n;
    }
    else if (m_cursor > max_cursor()) {
      m_top_list_idx += (m_cursor - max_cursor());
    }

    draw();
  }

  void scroll_items(int n) {
    m_top_list_idx += n;
    draw();
  }

  bool handle_mouse(MEVENT& m) override {
    if (wmouse_trafo(win, &m.y, &m.x, false)) {
      scroll_cursor(m.y - m_cursor);
      return true;
    }
    return false;
  }

private:
  void _clamp() {
    clamp(&m_top_list_idx,  0, container_size() - 1 - max_cursor());
    clamp(&m_cursor,        0, std::min(container_size() - m_top_list_idx - 1, max_cursor()));
    clamp(&m_active,       -1, container_size() - 1);
  }

  inline int max_cursor() { return size.height - 1; }

  inline void render_item(int item_idx, int line, bool cursor) {
    if (! itemRenderer)
      return;
    move(line, 0);
    itemRenderer(win, size.width, (*m_list)[size_t(item_idx)], item_idx, cursor, item_idx == m_active);
  }

  inline void unselect_item() {
    render_item(m_top_list_idx + m_cursor, m_cursor, false);
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
