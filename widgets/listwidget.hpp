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
      /* Primitive default renderer
      std::stringstream ss; ss << item;
      char marker = ' ';
      if (cursor && active) marker = 'X';
      else if (cursor)      marker = '>';
      else if (active)      marker = 'x';
      waddch(win, marker);
      waddnstr(win, ss.str().c_str(), m_width - 1);
 */
    }
};

template<typename ItemType>
class ListWidget : public UI::Window {
  private:
    std::vector<ItemType> *m_list;
    ListItemRenderer<ItemType> m_item_renderer;
    int m_cursor;
    int m_active;
    int m_top_index;
   
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

#if TEST_LISTWIDGET
#include <unistd.h>
#include <string>
#include <iostream>

void testListItemRenderer() {
  // TODO: test descrip
  // Draw the whole screen with numbers, the 3rd line should be selected

  ListItemRenderer<int> intRenderer(COLS);
  for (int i = 0; i < LINES; ++i) {
    wmove(stdscr, i, 0);
    intRenderer.render(stdscr, i, i, i == 3, i == 2);
  }
  refresh();
}

template<typename ItemType>
void testListWidget(
    ListWidget<ItemType> listWidget,
    std::vector<ItemType> &testData)
{
  listWidget.setList(&testData);
  listWidget.layout(LINES, COLS);
  for (;;) {
    switch (getch()) {
      case 'k': listWidget.up();        break;
      case 'j': listWidget.down();      break;
      case 'K': listWidget.page_up();   break;
      case 'J': listWidget.page_down(); break;
      case 'g': listWidget.top();       break;
      case 'G': listWidget.bottom();    break;
      case 'l': listWidget.draw();      break;
      case 'q': return;
    }
    listWidget.refresh();
  }
}

int main() {
  initscr();
  noecho();
  curs_set(0);
  start_color();
  use_default_colors();
  //UI::Color::init();
  //UI::Colors::init();
  //UI::Attribute::init();
  //Ektoplayer::Theme.load(256);
  try {
    ListItemRenderer<std::string> renderer(COLS);
    ListWidget<std::string> listWidget(renderer);
    std::vector<std::string> testData = { "Hello", "This is a test string", "Foo Bar", "Muhahaha" };
    for (int i = 0; i < 50; ++i) testData.push_back(std::to_string(i));
    testListWidget<std::string>(listWidget, testData);
    pause();
  } catch (const std::exception &e) {
    std::cout << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
#endif

#if not_so_weid
class ListWidget : public UI::Window {
  private:
    std::vector<item> list;
    int m_cursor;
    int m_selected;
    ListItemRenderer item_renderer;
   //attr_reader :selected, :cursor, :selection
   //attr_accessor :item_renderer
   
  public:
    void mv_top()       { selected(0); }
    void mv_bottom()    { selected(index_last); }
    void mv_page_up()   { scroll_up(size.height); }
    void mv_page_down() { scroll_down(size.height); }
    void mv_up(N)       { scroll_cursor_up(1);      }
    void mv_down(N)     { scroll_cursow_down(1);    }
    void center()       { force_cursorpos(size.height / 2); }

    void layout(int height, int width) {
      m_item_renderer.setWidth(size.width); // if item_renderer?
      //super;
    }

    int index_last() { return (list.size() ? list.size() - 1 : 0); }
    int cursor_max() {
      return list.size() ? std::min(list.size(), size.height) - 1 : 0;
    }

    void draw() {
      werase(win);
      m_selected = clamp(m_selected, 0, index_last());
      m_cursor   = clamp(m_cursor,   0, size.height -1 );

      for (int y = 0; y < size.height; ++y) {
        wmove(y, 0);

      }

      for (int line = 0; line < m_cursor; ++line) {
        if (m_selected - (m_cursor - line) >= list.size()) {
          m_cursor = line;
          break;
        }

        wmove(line, 0);
        render(m_selected - (m_cursor - cursor), false);
      }

      wmove(m_cursor, 0);
      render(m_selected, true);

      for (int line = m_cursor + 1; line < size.height; ++line) {
      }

      (@cursor + 1).upto(@size.height - 1).each_with_index do |c, i|
         break unless @list[@selected + i + 1]
         write_at(c); render(@selected + i + 1)
      end
   end


   def initialize(list: [], item_renderer: nil, **opts)
      super(**opts)
      self.list=(list)
      @item_renderer = (item_renderer or ListItemRenderer.new)
      @cursor = @selected = 0
      @search = ListSearch.new
      @selection = ListSelector.new
   end

   def render(index, **opts)
      return unless @item_renderer
      return Ektoplayer::Application.log(self, index, caller) unless @list[index]

      opts[:selection] = (@selection.started? and
            index.between?(
               [@selection.start_pos, @selected].min,
               [@selection.start_pos, @selected].max
            )
      )

      @item_renderer.render(@win, @list[index], index, **opts)
   end

   def list=(list)
      with_lock do
         @list = list
         @cursor = @selected = 0
         self.selected=(0)
         self.force_cursorpos(0)
         want_redraw
      end
   end

   def scroll_cursor_down(n)
      fail ArgumentError unless n
      fail ArgumentError if n.negative?
      n = n.clamp(0, items_after_cursor)
      return if n == 0
      new_cursor, new_selected = @cursor + n, @selected + n

      self.lock

      // it's faster to redraw the whole screen
      if n >= @size.height
         new_cursor = cursor_max
         want_redraw
      else
         # new cursor resides in current screen
         if new_cursor <= cursor_max
            if @selection.started?
               want_redraw
            else
               write_at(@cursor); render(@selected)
               write_at(new_cursor); render(new_selected, selected: true)
               want_refresh
            end
         else
            new_cursor = cursor_max

            if @selection.started?
               want_redraw
            else
               write_at(@cursor); render(@selected)

               (index_bottom + 1).upto(new_selected - 1).each do |index|
                  @win.append_bottom; render(index)
               end

               @win.append_bottom; render(new_selected, selected: true)

               want_refresh
            end
         end
      end

      @cursor, @selected = new_cursor, new_selected
   ensure
      self.unlock
   end

   def scroll_cursor_up(n)
      fail ArgumentError unless n
      fail ArgumentError if n.negative?
      n = n.clamp(0, items_before_cursor)
      return if n == 0
      new_cursor, new_selected = @cursor - n, @selected - n

      self.lock

      if n >= @size.height
         new_cursor = 0
         want_redraw
      else
         # new cursor resides in current screen
         if new_cursor >= 0
            if @selection.started?
               want_redraw
            else
               write_at(@cursor); render(@selected)
               write_at(new_cursor); render(new_selected, selected: true)
               want_refresh
            end
         else
            new_cursor = 0

            if @selection.started?
               want_redraw
            else
               write_at(@cursor); render(@selected)

               (index_top - 1).downto(new_selected + 1).each do |index|
                  @win.insert_top; render(index)
               end

               @win.insert_top; render(new_selected, selected: true)

               want_refresh
            end
         end
      end

      @cursor, @selected = new_cursor, new_selected
   ensure
      self.unlock
   end

   def selected=(new_index)
      fail ArgumentError unless new_index
      fail ArgumentError.new('negative index') if new_index.negative?
      new_index = new_index.clamp(0, index_last)

      with_lock do
         @selected = new_index
         self.force_cursorpos(@cursor)
         want_redraw
      end
   end

   # select an item by its current cursor pos
   def select_from_cursorpos(new_cursor)
      fail unless new_cursor.between?(0, cursor_max)
      return if (new_cursor == @cursor) or @list.empty?

      with_lock do
         old_cursor, @cursor = @cursor, new_cursor
         old_selected, @selected = @selected, (@selected - (old_cursor - @cursor)).clamp(0, index_last)

         if @selection.started?
            want_redraw
         else
            write_at(old_cursor); render(old_selected)
            write_at(new_cursor); render(@selected, selected: true)
            want_refresh
         end
      end
   end

   def force_cursorpos(new_cursor)
      with_lock do
         if @selected <= cursor_max / 2
            @cursor = @selected
         elsif (diff = (index_last - @selected)) < cursor_max / 2
            @cursor = @size.height - diff - 1
         else
            @cursor = new_cursor.clamp(cursor_max / 2, cursor_max)
         end

         want_redraw
      end
   end

   def scroll_list_up(n=1)
      fail ArgumentError unless n
      n = n.clamp(0, items_before_cursor)
      return if n == 0 or @list.empty?
      self.lock

      if index_top == 0
         # list is already on top
         select_from_cursorpos((@cursor - n).clamp(0, cursor_max))
      elsif n < @size.height
         old_index_top = index_top
         old_selected, @selected = @selected, @selected - n

         if lines_after_cursor > n
            write_at(@cursor); render(old_selected) 
         end

         (old_index_top - 1).downto(old_index_top - n).each do |index|
            @win.insert_top; render(index)
         end

         write_at(@cursor); render(@selected, selected: true)

         want_refresh
      else
         @selected -= n
         force_cursorpos(@cursor)
         want_redraw
      end

      self.unlock
   end

   def scroll_list_down(n=1)
      fail ArgumentError unless n
      n = n.clamp(0, items_after_cursor)
      return if n == 0 or @list.empty?
      self.lock

      if index_bottom == index_last
         select_from_cursorpos((@cursor + n).clamp(0, cursor_max))
      elsif n < @size.height
         old_index_bottom = index_bottom
         old_selected, @selected = @selected, @selected + n

         if lines_before_cursor > n
            write_at(@cursor); render(old_selected)
         end

         (old_index_bottom + 1).upto(old_index_bottom + n).each do |index|
            @win.append_bottom; render(index)
         end

         write_at(@cursor); render(@selected, selected: true)

         want_refresh
      else
         @selected += n
         force_cursorpos(@cursor)
         want_redraw
      end

      self.unlock
   end

   def on_mouse_click(mevent, mevent_transformed)
      if new_mouse = mouse_event_transform(mevent)
         select_from_cursorpos(new_mouse.y)
      end
      super(mevent)
   end

   protected

   def write_at(pos)   @win.move(pos, 0)               end

   def index_first;   0                                end
   def index_top;     @selected - @cursor              end
   def index_bottom
      [@selected + @size.height - @cursor, @list.size].min - 1
   end

   def lines_before_cursor;  @cursor                      end
   def lines_after_cursor;   @size.height - cursor - 1    end
   def items_before_cursor;  @selected;                   end
   def items_after_cursor;   @list.size - @selected - 1   end
};
#endif
