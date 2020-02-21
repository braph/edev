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
  for (auto w : _widgets)
    if (w->visible)
      w->draw();
}

void GenericContainer :: noutrefresh() {
  for (auto w : _widgets)
    if (w->visible)
      w->noutrefresh();
}

bool GenericContainer :: handleMouse(MEVENT& m) {
  for (auto w : _widgets)
    if (w->visible && w->handleMouse(m))
      return true;

  return false;
}

void GenericContainer :: addWidget(Widget* widget) {
  _widgets.push_back(widget);
}

WINDOW* GenericContainer :: active_win() {
  assert(static_cast<unsigned>(_current) < _widgets.size());
  return _widgets[_current]->active_win();
}

int GenericContainer :: currentIndex() {
  return _current;
}

int GenericContainer :: indexOf(Widget* widget) {
  int idx = 0;
  for (auto w : _widgets) {
    if (w == widget)
      return idx;
    ++idx;
  }
  return -1;
}

size_t GenericContainer :: count() {
  return _widgets.size();
}

void GenericContainer :: setCurrentIndex(int index) {
  assert(static_cast<unsigned>(index) < _widgets.size());
  _current = index;
  draw();
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

  for (auto w : _widgets) {
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

void StackedContainer :: draw() {
  assert(static_cast<unsigned>(_current) < _widgets.size());
  _widgets[_current]->draw();
}

void StackedContainer :: noutrefresh() {
  assert(static_cast<unsigned>(_current) < _widgets.size());
  _widgets[_current]->noutrefresh();
}

bool StackedContainer :: handleMouse(MEVENT& m) {
  assert(static_cast<unsigned>(_current) < _widgets.size());
  return _widgets[_current]->handleMouse(m);
}

void StackedContainer :: layout(Pos pos, Size size) {
  this->pos  = pos;
  this->size = size;

  for (auto w : _widgets) {
    if (w->visible)
      w->layout(pos, size);
  }
}

