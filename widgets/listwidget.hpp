#ifndef _LISTWIDGET_HPP
#define _LISTWIDGET_HPP

#include "../ui.hpp"
#include "../common.hpp"
#include <sstream>
#include <vector>
#include <climits>

template<typename ItemType>
class ListItemRenderer {
  protected:
    int m_width;

  public:
    ListItemRenderer(int width = 0) : m_width(width) { }

    void setWidth(int width) {
      m_width = width;
    }

    virtual void render(WINDOW *win, const ItemType &item, int index, bool cursor, bool active) { // marked, selection
      // Primitive default renderer
      std::stringstream ss; ss << item;
      char marker = ' ';
      if (cursor && active) marker = 'X';
      else if (cursor)      marker = '>';
      else if (active)      marker = 'x';
      waddch(win, marker);
      waddnstr(win, ss.str().c_str(), m_width - 1);
    }
};

template<typename ItemType>
class ListWidget : public UI::Window {
  private:
    int m_cursor;
    int m_active;
    int m_top_index;
    ListItemRenderer<ItemType> m_item_renderer;
    std::vector<ItemType> *m_list;
   
  public:
    ListWidget(ListItemRenderer<ItemType> renderer)
    : m_cursor(0)
    , m_active(0)
    , m_top_index(0)
    , m_item_renderer(renderer)
    {
    }

    void setList(std::vector<ItemType> *list) {
      m_list = list;
    }

    void layout(int height, int width) {
      size.height = height;
      size.width = width;
      wresize(win, height, width);
      m_item_renderer.setWidth(size.width); // if item_renderer?
      //super;
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
    void top()       { m_cursor = m_top_index = 0; }
    void bottom()    { m_cursor = m_top_index = INT_MAX; }
    void page_up()   { scroll_up(); }
    void page_down() { scroll_down(); }
    /*
    void page_up()   { scroll_up(size.height); }
    void page_down() { scroll_down(size.height); }
    void up(N)       { scroll_cursor_up(1);      }
    void down(N)     { scroll_cursow_down(1);    }
    //void center()    { force_cursorpos(size.height / 2); }
    */
};

#endif
