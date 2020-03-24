#ifndef GENERICITERATOR_HPP
#define GENERICITERATOR_HPP

#include <ostream>
#include <iterator>

/**
 * Make anything iterable that provides operator[].
 * - Returns TContainer::reference
 */
template<typename TContainer>
class GenericIterator
//: public std::iterator<std::random_access_iterator_tag, typename TContainer::reference>
{
public:
  using iterator_category = std::random_access_iterator_tag;
  using value_type        = typename TContainer::reference;
  using difference_type   = std::ptrdiff_t;
  using pointer           = typename TContainer::reference;//*;
  using reference         = typename TContainer::reference;//&;

  typedef GenericIterator iterator;

  GenericIterator() : container(NULL), idx(0) {}
  GenericIterator(TContainer& container, size_t idx) : container(&container), idx(idx) {}
  GenericIterator(const GenericIterator& rhs) : container(rhs.container), idx(rhs.idx) {}

  iterator& operator=(const iterator&it) noexcept {
    container = it.container;
    idx = it.idx;
    return *this;
  } 

  bool operator==(const iterator&it) const noexcept { return idx == it.idx; }
  bool operator!=(const iterator&it) const noexcept { return idx != it.idx; }
  bool operator< (const iterator&it) const noexcept { return idx <  it.idx; }
  bool operator> (const iterator&it) const noexcept { return idx >  it.idx; }
  bool operator<=(const iterator&it) const noexcept { return idx <= it.idx; }
  bool operator>=(const iterator&it) const noexcept { return idx >= it.idx; }

  reference operator*()              const { return (*container)[idx]; }
  reference operator[](ptrdiff_t n)  const { return *(*this + n);      }

  iterator& operator++() noexcept    { ++idx; return *this; }
  iterator& operator--() noexcept    { --idx; return *this; }
  iterator  operator++(int) noexcept { iterator old = *this; ++idx; return old; }
  iterator  operator--(int) noexcept { iterator old = *this; --idx; return old; }
  iterator& operator+=(ptrdiff_t n) noexcept       { idx += n; return *this;            }
  iterator& operator-=(ptrdiff_t n) noexcept       { idx -= n; return *this;            }
  iterator  operator+ (ptrdiff_t n) const noexcept { iterator i = *this; return i += n; }
  iterator  operator- (ptrdiff_t n) const noexcept { iterator i = *this; return i -= n; }

  ptrdiff_t operator- (const iterator&it) const noexcept
  { return static_cast<ptrdiff_t>(idx) - static_cast<ptrdiff_t>(it.idx); }

  ptrdiff_t operator+ (const iterator&it) const noexcept
  { return static_cast<ptrdiff_t>(idx) + static_cast<ptrdiff_t>(it.idx); }

  inline friend std::ostream& operator<<(std::ostream& os, const iterator& it) {
    os << "Iterator(" << it.idx << ')';
    return os;
  }

private:
  TContainer *container;
  size_t idx;
};

#endif
