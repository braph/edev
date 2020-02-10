#include "container.hpp"
using namespace UI;

/* ============================================================================
 * GenericContainer
 * ==========================================================================*/

GenericContainer :: GenericContainer()
: active(0)
{
}

void GenericContainer :: draw() {
  for (auto w : widgets) {
    if (w->visible)
      w->draw();
  }
}

void GenericContainer :: refresh() {
  for (auto w : widgets) {
    if (w->visible)
      w->refresh();
  }
}

void GenericContainer :: add(Widget* widget) {
  widgets.push_back(widget);
}

WINDOW* GenericContainer :: active_win() {
  return widgets[active]->active_win();
}

/* ============================================================================
 * VerticalContainer
 * ==========================================================================*/

void VerticalContainer :: layout(Pos pos, Size size) {
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

