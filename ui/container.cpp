#include "../ui.hpp"

namespace UI {
  class GenericContainer : public Widget {
  protected:
    std::vector<Widget*> widgets;
  public:
    void draw()    { for (auto w : widgets) { w->draw();    } }
    void refresh() { for (auto w : widgets) { w->refresh(); } }

    void add(Widget* widget) {
      widgets.push_back(widget);
    }

    WINDOW *active_win() { return widgets[0]->active_win(); } // SEL
  };

  class VerticalContainer : public GenericContainer {
  public:
    void layout(Pos pos, Size size) {
      this->pos  = pos;
      this->size = size;

      for (auto w : widgets) {
        if (! w->visible)
          continue;

        w->layout(pos, size);
        pos.y       += w->size.height;
        size.height -= w->size.height;
      }
    }
  };
}

