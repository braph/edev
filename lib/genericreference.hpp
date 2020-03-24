#ifndef GENERICREFERENCE_HPP
#define GENERICREFERENCE_HPP

/**
 * Generic reference for classes that provides:
 * - get(size_t index)
 * - set(size_t index, const TContainer::value_type& value)
 */
template<typename TContainer>
class GenericReference {
public:
  using reference = GenericReference;
  using value_type = typename TContainer::value_type;

  GenericReference(TContainer& container, size_t index)
  : _container(container)
  , _index(index)
  {}

  reference& operator=(const value_type& value) {
    _container.set(_index, value);
    return *this;
  }

  reference& operator=(const reference& rhs) {
    _container.set(_index, rhs._container[rhs._index]);
    return *this;
  }

  operator value_type() const {
    return _container.get(_index);
  }

private:
  TContainer& _container;
  size_t _index;
};

#endif
