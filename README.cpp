
// Mockups

// WidgetSizeError?
// Class Input

class Screen {
public:
  void init();
  void enable_resize_detection();
  void endwin();
  void update(bool force_redraw, bool force_resize);
};

struct Point {
  short x, y;
  Point(short _y, short _x) : x(_x), y(_y) {}
  Point calc(short _y, short _x)          { return Point(y+_y, x+_x);    }
  inline bool operator==(const Point& p)  { return x == p.x && y == p.y; }
  inline bool operator>=(const Point& p)  { return x >= p.x && y >= p.y; }
  inline bool operator<=(const Point& p)  { return x <= p.x && y <= p.y; }
  // update()
  // to_s()
};

// Class Size

   class MouseEvents < Events
      def on(mouse_event, &block)
         return on_all(&block) if mouse_event == ICurses::ALL_MOUSE_EVENTS
         super(mouse_event, &block)
      end

      def trigger(mouse_event)
         super(mouse_event.bstate, mouse_event)
      end
   end

class MouseSectionEvent
  def initialize(start=nil, stop=nil)
     @start_pos, @stop_pos, @events = start, stop, MouseEvents.new
  end

  def on(button, &block); @events.on(button, &block)  end

  def trigger(mevent)
     return unless mevent.pos >= @start_pos and mevent.pos <= @stop_pos
     @events.trigger(mevent)
  end
end

class MouseSectionEvents {
  std::vector<Muos> events;
public:
  inline void clear() { events.clear(); }
  inline void add()   {                 }
    
}

   class MouseSectionEvents
      def add(mouse_section_event)
         @events << mouse_section_event
      end

      def trigger(mevent)
         @events.each { |e| e.trigger(mevent) }
      end
   end


class Window {
private:
  Window *win;

public:
  inline int height()           { return getmaxy(win);      }
  inline int width()            { return getmaxx(win);      }

  inline int erase()            { return werase(win);       }
  inline int move(int y, int x) { return wmove(win, y, x);  }

  // def cursor;  UI::Point.new(y: cury, x: curx)           end
  // def pos;     UI::Point.new(y: begy, x: begx)           end
  // def size;    UI::Size.new(height: maxy, width: maxx)   end

      def cursor=(new)
         move(new.y, new.x)  #or warn "Could not set cursor: #{new} #{size}"
      end

      def pos=(new)
         mvwin(new.y, new.x)
      end

      def size=(new)
         resize(new.height, new.width) or fail "Could not resize: #{new}"
      end

      def with_attr(attr)
         attron(attr); yield; attroff(attr)
      end

      def getch1(timeout=-1)
         self.timeout(timeout)
         getch
      end

      def on_line(n)       move(n, curx)                        ;self;end
      def on_column(n)     move(cury, n)                        ;self;end
      def next_line;       move(cury + 1, 0)                    ;self;end
      def mv_left(n)       move(cury, curx - 1)                 ;self;end
      def from_left(size)  move(cury, size)                     ;self;end
      def from_right(size) move(cury, (maxx - size))            ;self;end
      def center(size)     move(cury, (maxx / 2) - (size / 2))  ;self;end

      def center_string(string)
         center(string.size)
         addstr(string)
         self
      end

      def insert_top
         move(0, 0)
         insertln
         self
      end

      def append_bottom
         move(0, 0)
         deleteln
         move(maxy - 1, 0)
         self
      end
   end




   class IMouseEvent
      def pos
         UI::Point.new(x: x, y: y)
      end

      def to_fake
         IMouseEvent.new(self)
      end

      def to_s
         "[(IMouseEvent) bstate=#{bstate}, x=#{x}, y=#{y}, z=#{z}]"
      end
   end




class Widget {
  bool handleKey(k);
};

/*
 * - Widgets are created by the main loop
 *   - Playinginfo
 *      ` ----------- 


void mainloop() {

  while (getch()) {
    active_widget->handle(key);
  }
}


class SomeWidget {
  void handle_key(k) {
    std::map<>;
    keys[key];
  }
}
*/


class Database {
  changed();
};

class Model {

};



                  Views:
                  +--- Mainview -----------------------+
                  |    +--- PlayingInfo -----------+   |
                  |    | [Length] [Artist] [State] |   |
                  |    +---------------------------+   |
                  |                                    |
                  +------------------------------------+


Database:
  changed()
Player:
  play(), pause(), stop(regular), position_change(position?)
Input:
  key_pressed()
Download:
  completed(), failed()













