#ifndef _GENERIC_HPP
#define _GENERIC_HPP

#include <ostream>
#include <iterator>

/* Make anything iterable that provides operator[].
 * - Returns TContainer::reference
 */
template<typename TContainer>
class GenericIterator
: public std::iterator<std::random_access_iterator_tag, typename TContainer::reference> {
public:
  typedef GenericIterator iterator;
  typedef typename TContainer::reference reference;

  GenericIterator() : container(NULL), idx(0) {}
  GenericIterator(TContainer& container, size_t idx) : container(&container), idx(idx) {}
  GenericIterator(const GenericIterator& rhs) : container(rhs.container), idx(rhs.idx) {}

  iterator& operator=(const iterator&it) {
    container = it.container;
    idx = it.idx;
    return *this;
  } 

  bool operator==(const iterator&it) const { return idx == it.idx; }
  bool operator!=(const iterator&it) const { return idx != it.idx; }
  bool operator< (const iterator&it) const { return idx <  it.idx; }
  bool operator> (const iterator&it) const { return idx >  it.idx; }
  bool operator<=(const iterator&it) const { return idx <= it.idx; }
  bool operator>=(const iterator&it) const { return idx >= it.idx; }

  reference operator*()              const { return (*container)[idx]; }
  reference operator[](ptrdiff_t n)  const { return *(*this + n);      }

  iterator& operator++()                  { ++idx; return *this; }
  iterator& operator--()                  { --idx; return *this; }
  iterator  operator++(int)         { iterator old = *this; ++idx; return old; }
  iterator  operator--(int)         { iterator old = *this; --idx; return old; }
  iterator& operator+=(ptrdiff_t n)       { idx += n; return *this;            }
  iterator& operator-=(ptrdiff_t n)       { idx -= n; return *this;            }
  iterator  operator+ (ptrdiff_t n) const { iterator i = *this; return i += n; }
  iterator  operator- (ptrdiff_t n) const { iterator i = *this; return i -= n; }

  ptrdiff_t operator- (const iterator&it) const { return idx - it.idx; }
  ptrdiff_t operator+ (const iterator&it) const { return idx + it.idx; }

  inline friend std::ostream& operator<<(std::ostream& os, const iterator& it) {
    os << "Iterator(" << it.idx << ")";
    return os;
  }

private:
  TContainer *container;
  size_t idx;
};

/* Generic reference for classes that provides:
 * - get(size_t index)
 * - set(size_t index, const TContainer::value_type& value)
 */
template<typename TContainer>
struct GenericReference {
  TContainer& _container;
  size_t _index;

  typedef typename TContainer::value_type value_type;
  typedef GenericReference reference;

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
};

#endif
