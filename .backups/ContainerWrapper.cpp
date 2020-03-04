/* The ContainerView that should initially be returned by the database. */

  template<typename TStore>
  class Result {
    TStore *store;
    std::vector<int> indices;
  public:
    Result() : store(NULL) {}

    Result(TStore &store) : store(&store) {
      indices.reserve(store.size());
      for (size_t i = 1 /* Skip NULL Record */; i < store.size(); ++i)
        indices.push_back(i);
    }

    typedef typename TStore::value_type value_type;
    using reference = typename TStore::value_type;

    Result& operator=(const Result& rhs) {
      store   = rhs.store;
      indices = rhs.indices;
      return *this;
    }

    inline size_t size() {
      return indices.size();
    }

    value_type operator[](size_t id) {
      return (*store)[indices[id]];
    }

    GenericIterator<Result> begin() {
      return GenericIterator<Result>(*this, 0);
    }

    GenericIterator<Result> end() {
      return GenericIterator<Result>(*this, size());
    }

    void order_by(ColumnID column, SortOrder order) {
      OrderBy orderBy(column, order);
      std::sort(indices.begin(), indices.end(), [&](size_t a, size_t b) {
        return orderBy((*store)[a], (*store)[b]);
      });
    }

    template<typename TValue>
    void where(ColumnID column, Operator op, TValue value) {
      Where where(column, op, value);
      auto end = std::remove_if(indices.begin(), indices.end(),
          [&](size_t i){ return where((*store)[i]); });
      indices.erase(end, indices.end());
    }
  };

