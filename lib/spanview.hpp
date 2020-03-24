#ifndef SPANVIEW_HPP
#define SPANVIEW_HPP

/* SpanView:
 * [0] [1] [2] [3] <- index
 * [a] [b]         <- Underlying container
 * [a] [a] [b] [b] <- get(4, index)
 * [a] [b] [b] [a] <- get2(4, index)
 */
template<typename TContainer>
struct SpanView {
  SpanView() : _container(NULL) {}
  SpanView(TContainer& container) : _container(&container) {}
  SpanView& operator=(const SpanView& rhs) { _container = rhs._container; return *this; }

  typename TContainer::value_type get(size_t size, size_t index) {
    size_t i = _container->size() * index / size;
    return (*_container)[i];
  }

  typename TContainer::value_type get2(size_t size, size_t index) {
    size_t i = _container->size() * index * 2 / size;
    if (i >= _container->size())
      i = _container->size() - (i - _container->size() + 1);
    return (*_container)[i];
  }

private:
  TContainer* _container;
};

#endif
