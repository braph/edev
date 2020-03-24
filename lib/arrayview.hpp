#ifndef ARRAYVIEW_HPP
#define ARRAYVIEW_HPP

/**
 * Holds a reference to an array.
 * - Provides size()
 * - Maybe add begin() and end()?
 */
template<typename T>
class ArrayView {
public:
  using value_type = T;

  ArrayView()
  : _array(NULL)
  , _size(0)
  {}

  template<size_t N>
  ArrayView(T (&array)[N])
  : _array(&array[0])
  , _size(N)
  {}

  value_type& operator[](size_t index) noexcept
  { return _array[index]; }

  size_t size() const noexcept
  { return _size; }

private:
  T*     _array;
  size_t _size;
};

#endif
