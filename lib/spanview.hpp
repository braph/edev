#ifndef LIB_SPANVIEW_HPP
#define LIB_SPANVIEW_HPP

#include <cstddef>

/* SpanView:
 * [0] [1] [2] [3] <- index
 * [a] [b]         <- Underlying container
 * [a] [a] [b] [b] <- get(4, index)
 * [a] [b] [b] [a] <- get2(4, index)
 */
template<typename TContainer>
struct SpanView {
  using size_type  = size_t;
  using value_type = typename TContainer::value_type;

  inline SpanView() noexcept
    : _container(NULL)
  {}

  inline SpanView(TContainer& container) noexcept
    : _container(&container)
  {}

  inline SpanView& operator=(const SpanView& rhs) noexcept {
    _container = rhs._container;
    return *this;
  }

  inline value_type get(const size_type size, const size_type index) {
    size_t i = _container->size() * index / size;
    return (*_container)[i];
  }

  inline value_type get2(const size_type size, const size_type index) {
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
