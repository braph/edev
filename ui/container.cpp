#include "container.hpp"
using namespace UI;

#define CURRENT_WIDGET _widgets[size_t(_current)]

/* ============================================================================
 * GenericContainer
 * ==========================================================================*/

GenericContainer :: GenericContainer() noexcept
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
  return empty() ? false : CURRENT_WIDGET->handleKey(key);
}

WINDOW* GenericContainer :: getWINDOW() const noexcept {
  return empty() ? NULL : CURRENT_WIDGET->getWINDOW();
}

void GenericContainer :: addWidget(Widget* widget) {
  _widgets.push_back(widget);
}

int GenericContainer :: currentIndex() const noexcept {
  return _current;
}

void GenericContainer :: currentIndex(int index) noexcept {
  if (index >= 0 && index < count())
    _current = index;
  draw();
}

Widget* GenericContainer :: currentWidget() const noexcept {
  return empty() ? NULL : CURRENT_WIDGET;
}

void GenericContainer :: currentWidget(Widget* widget) noexcept {
  for (size_t i = 0; i < _widgets.size(); ++i)
    if (_widgets[i] == widget)
      _current = int(i);
}

int GenericContainer :: indexOf(Widget* widget) const noexcept {
  for (size_t i = 0; i < _widgets.size(); ++i)
    if (_widgets[i] == widget)
      return i;

  return -1;
}

int GenericContainer :: count() const noexcept {
  return int(_widgets.size());
}

bool GenericContainer :: empty() const noexcept {
  return _widgets.empty();
}

/* ============================================================================
 * VerticalContainer
 * ==========================================================================*/

VerticalContainer :: VerticalContainer() noexcept
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

StackedContainer :: StackedContainer() noexcept
: GenericContainer()
{
}

void StackedContainer :: draw() {
  if (! empty() && CURRENT_WIDGET->visible)
    CURRENT_WIDGET->draw();
}

void StackedContainer :: noutrefresh() {
  if (! empty() && CURRENT_WIDGET->visible)
    CURRENT_WIDGET->noutrefresh();
}

bool StackedContainer :: handleMouse(MEVENT& m) {
  if (! empty() && CURRENT_WIDGET->visible)
    return CURRENT_WIDGET->handleMouse(m);
  return false;
}

void StackedContainer :: layout(Pos pos, Size size) {
  this->pos  = pos;
  this->size = size;

  for (const auto& w : _widgets)
    if (w->visible)
      w->layout(pos, size);
}

