/**
 * Generic reference for classes that provide:
 * - get(size_t index)
 * - set(size_t index, const TContainer::value_type& value)
 */
template<typename TContainer>
class GenericReference {
public:
  using reference  = GenericReference;
  using value_type = typename TContainer::value_type;

  inline GenericReference(TContainer* container, size_t index) noexcept
    : _container(container)
    , _index(index)
  {}

  inline reference& operator=(const value_type& value) {
    _container->set(_index, value);
    return *this;
  }

  inline reference& operator=(const reference& rhs) {
    _container->set(_index, rhs._container[rhs._index]);
    return *this;
  }

  inline operator value_type() const noexcept {
    return _container->get(_index);
  }

private:
  TContainer* _container;
  size_t      _index;
};

template<typename TContainer>
class GenericConstReference {
public:
  using reference  = GenericConstReference;
  using value_type = typename TContainer::value_type;

  inline GenericConstReference(const TContainer* container, size_t index) noexcept
    : _container(container)
    , _index(index)
  {}

  inline operator value_type() const noexcept {
    return _container->get(_index);
  }

private:
  const TContainer* _container;
  size_t            _index;
};
