// These using declarations should be dropped, maybe.
// Or at least provide some template foo that defines them only if undefined.

using size_type              = size_t;
using value_type             = T;
using reference              = T&;
using const_reference        = const T&;
using iterator               = T*;
using const_iterator         = const T*;
using reverse_iterator       = std::reverse_iterator<iterator>;
using const_reverse_iterator = std::reverse_iterator<const_iterator>;

inline size_type       size()               const noexcept { return _size;          }
inline bool            empty()              const noexcept { return  _size == 0;    }

inline reference       operator[](size_t i)       noexcept { return  _data[i];      }
inline const_reference operator[](size_t i) const noexcept { return  _data[i];      }

inline reference       front()                    noexcept { return _data[0];       }
inline const_reference front()              const noexcept { return _data[0];       }

inline reference       back()                     noexcept { return _data[_size-1]; }
inline const_reference back()               const noexcept { return _data[_size-1]; }

inline T*              data()                     noexcept { return &_data[0];      }
inline T const*        data()               const noexcept { return &_data[0];      }

inline iterator         begin()                   noexcept { return &_data[0];      }
inline const_iterator   begin()             const noexcept { return &_data[0];      }
inline const_iterator  cbegin()             const noexcept { return &_data[0];      }

inline iterator         end()                     noexcept { return &_data[_size];  }
inline const_iterator   end()               const noexcept { return &_data[_size];  }
inline const_iterator  cend()               const noexcept { return &_data[_size];  }

inline reverse_iterator         rend()            noexcept { return &_data[0];      }
inline const_reverse_iterator   rend()      const noexcept { return &_data[0];      }
inline const_reverse_iterator  crend()      const noexcept { return &_data[0];      }

inline reverse_iterator         rbegin()          noexcept { return &_data[_size];  }
inline const_reverse_iterator   rbegin()    const noexcept { return &_data[_size];  }
inline const_reverse_iterator  crbegin()    const noexcept { return &_data[_size];  }
