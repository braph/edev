#if 0
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
  using iterator          = GenericIterator;

  inline GenericIterator() noexcept
    : container(NULL)
    , idx(0)
  {}

  inline GenericIterator(TContainer* container, size_t idx) noexcept
    : container(container)
    , idx(idx)
  {}

  inline GenericIterator(const GenericIterator& rhs) noexcept
    : container(rhs.container)
    , idx(rhs.idx)
  {}

  inline iterator& operator=(const iterator&it) noexcept {
    container = it.container;
    idx = it.idx;
    return *this;
  }

  inline bool operator==(const iterator&it) const noexcept { return idx == it.idx; }
  inline bool operator!=(const iterator&it) const noexcept { return idx != it.idx; }
  inline bool operator< (const iterator&it) const noexcept { return idx <  it.idx; }
  inline bool operator> (const iterator&it) const noexcept { return idx >  it.idx; }
  inline bool operator<=(const iterator&it) const noexcept { return idx <= it.idx; }
  inline bool operator>=(const iterator&it) const noexcept { return idx >= it.idx; }

  inline reference operator*()              const noexcept { return (*container)[idx]; }
  inline reference operator[](ptrdiff_t n)  const noexcept { return *(*this + n);      }

  inline iterator& operator++()                   noexcept { ++idx; return *this;                     }
  inline iterator& operator--()                   noexcept { --idx; return *this;                     }
  inline iterator  operator++(int)                noexcept { iterator old = *this; ++idx; return old; }
  inline iterator  operator--(int)                noexcept { iterator old = *this; --idx; return old; }
  inline iterator& operator+=(ptrdiff_t n)        noexcept { idx += n; return *this;                  }
  inline iterator& operator-=(ptrdiff_t n)        noexcept { idx -= n; return *this;                  }
  inline iterator  operator+ (ptrdiff_t n)  const noexcept { iterator i = *this; return i += n;       }
  inline iterator  operator- (ptrdiff_t n)  const noexcept { iterator i = *this; return i -= n;       }

  inline ptrdiff_t operator- (const iterator&it) const noexcept
  { return static_cast<ptrdiff_t>(idx) - static_cast<ptrdiff_t>(it.idx); }

  inline ptrdiff_t operator+ (const iterator&it) const noexcept
  { return static_cast<ptrdiff_t>(idx) + static_cast<ptrdiff_t>(it.idx); }

  inline size_t index() const noexcept
  { return idx; }

private:
  TContainer *container;
  size_t idx;
};

#endif

