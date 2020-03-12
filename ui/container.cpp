#include "container.hpp"
#include "../common.hpp"
using namespace UI;

/* ============================================================================
 * GenericContainer
 * ==========================================================================*/

GenericContainer :: GenericContainer()
: _current(0)
{
}

void GenericContainer :: draw() {
  for (const auto& w : _widgets)
    if (w->visible)
      w->draw();
}

void GenericContainer :: noutrefresh() {
  for (const auto& w : _widgets)
    if (w->visible)
      w->noutrefresh();
}

bool GenericContainer :: handleMouse(MEVENT& m) {
  for (const auto& w : _widgets)
    if (w->visible && w->handleMouse(m))
      return true;
  return false;
}

bool GenericContainer :: handleKey(int key) {
  if (! empty())
    return currentWidget()->handleKey(key);
  return false;
}

void GenericContainer :: addWidget(Widget* widget) {
  _widgets.push_back(widget);
}

WINDOW* GenericContainer :: getWINDOW() const {
  if (! empty())
    return currentWidget()->getWINDOW();
  return NULL;
}

int GenericContainer :: currentIndex() const {
  return _current;
}

Widget* GenericContainer :: currentWidget() const {
  if (! empty())
    return _widgets[size_t(_current)];
  return NULL;
}

int GenericContainer :: indexOf(Widget* widget) const {
  int idx = 0;
  for (const auto& w : _widgets) {
    if (w == widget)
      return idx;
    ++idx;
  }
  return -1;
}

void GenericContainer :: setCurrentIndex(int index) {
  if (index >= 0 && index < count())
    _current = index;
  draw();
}

int GenericContainer :: count() const {
  return int(_widgets.size());
}

bool GenericContainer :: empty() const {
  return count() == 0;
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

  for (const auto& w : _widgets)
    if (w->visible) {
      w->layout(pos, UI::Size(w->size.height, size.width));
      pos.y += w->size.height;
    }
}

/* ============================================================================
 * StackedContainer
 * ==========================================================================*/

StackedContainer :: StackedContainer()
: GenericContainer()
{
}

void StackedContainer :: draw() {
  if (! empty() && currentWidget()->visible)
    currentWidget()->draw();
}

void StackedContainer :: noutrefresh() {
  if (! empty() && currentWidget()->visible)
    currentWidget()->noutrefresh();
}

bool StackedContainer :: handleMouse(MEVENT& m) {
  if (! empty() && currentWidget()->visible)
    return currentWidget()->handleMouse(m);
  return false;
}

void StackedContainer :: layout(Pos pos, Size size) {
  this->pos  = pos;
  this->size = size;

  for (const auto& w : _widgets)
    if (w->visible)
      w->layout(pos, size);
}

