#include "container.hpp"
#include "../common.hpp"
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

void GenericContainer :: noutrefresh() {
  for (auto w : widgets) {
    if (w->visible)
      w->noutrefresh();
  }
}

void GenericContainer :: add(Widget* widget) {
  widgets.push_back(widget);
}

WINDOW* GenericContainer :: active_win() {
  assert(widgets.size());
  return widgets[active]->active_win();
}

/* ============================================================================
 * VerticalContainer
 * ==========================================================================*/

VerticalContainer :: VerticalContainer()
: GenericContainer()
{
}

void VerticalContainer :: layout(Pos pos, Size size) {
  this->pos  = pos;
  this->size = size;

  for (auto w : widgets) {
    if (w->visible) {
      w->layout(pos, w->size.duplicate(UI::Size::KEEP, size.width));
      pos.y       += w->size.height;
    }
  }
}

/* ============================================================================
 * StackedContainer
 * ==========================================================================*/

StackedContainer :: StackedContainer()
: GenericContainer()
{
}

void StackedContainer :: layout(Pos pos, Size size) {
  this->pos  = pos;
  this->size = size;

  for (auto w : widgets) {
    if (w->visible)
      w->layout(pos, size);
  }
}

