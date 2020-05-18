#ifndef SPANVIEW_HPP
#define SPANVIEW_HPP

#include <cstddef>

/* SpanView:
 * [0] [1] [2] [3] <- index
 * [a] [b]         <- Underlying container
 * [a] [a] [b] [b] <- get(4, index)
 * [a] [b] [b] [a] <- get2(4, index)
 */
template<typename TContainer>
struct SpanView {
  SpanView()
    : _container(NULL)
  {}

  SpanView(TContainer& container)
    : _container(&container)
  {}

  SpanView& operator=(const SpanView& rhs)
  { _container = rhs._container; return *this; }

  typename TContainer::value_type get(const size_t size, const size_t index) {
    size_t i = _container->size() * index / size;
    return (*_container)[i];
  }

  typename TContainer::value_type get2(const size_t size, const size_t index) {
    const size_t container_size = _container->size();
    size_t i = container_size * index * 2 / size;
    if (i >= container_size)
      i = container_size - (i - container_size + 1);
    return (*_container)[i];
  }

private:
  TContainer* _container;
};

#endif
