#ifndef LIB_GENERICREFERENCE_HPP
#define LIB_GENERICREFERENCE_HPP

#include <type_traits>

/**
 * Generic reference for classes that provide:
 * - get(size_t index)
 * - set(size_t index, const TContainer::value_type& value)
 */
template<typename T, bool IS_CONST>
class GenericReference_Impl {
public:
  using reference  = GenericReference_Impl;
  using value_type = typename std::conditional<IS_CONST, const typename T::value_type, typename T::value_type>::type ;
  using TContainer = typename std::conditional<IS_CONST, const T, T>::type;

  inline GenericReference_Impl(TContainer* container, size_t index) noexcept
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
using GenericReference = GenericReference_Impl<TContainer, false>;

template<typename TContainer>
using GenericConstReference = GenericReference_Impl<TContainer, true>;

#if 0 /* BACKUP */
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
#endif

#endif
