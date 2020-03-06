class TrackSearch {
public:
  TrackSearch(const PlaylistColumns& columns);
  bool next();
  bool prev();
  inline size_t getIndex() { return index; }
  void startSearch(const std::string&, std::vector<Database::Tracks::Track>*);

private:
  const PlaylistColumns& m_columns;
  std::vector<Database::Tracks::Track>* list;
  std::string query;
  size_t index;

  bool indexMatchesCriteria();
};

/* ============================================================================
 * TrackSearch - Circular search through what is displayed by the TrackRenderer
 * ==========================================================================*/

TrackSearch :: TrackSearch(const PlaylistColumns& columns) : m_columns(columns), list(NULL) { }

bool TrackSearch :: next() {
  if (list) {
    for (index = clamp<size_t>(index + 1, 0, list->size() - 1); index < list->size(); ++index)
      if (indexMatchesCriteria())
        return true;

    index = std::numeric_limits<size_t>::max();
  }
  return false;
}

bool TrackSearch :: prev() {
  if (list) {
    for (index = clamp<size_t>(index - 1, 0, list->size() - 1); index; --index)
      if (indexMatchesCriteria())
        return true;
  }
  return false;
}

bool TrackSearch :: indexMatchesCriteria() {
  for (const auto &column : m_columns)
    if (boost::algorithm::icontains(trackField((*list)[index], column.tag), query))
      return true;
  return false;
}

void TrackSearch :: startSearch(const std::string& q, std::vector<Database::Tracks::Track>* l) {
  list = l;
  query = q;
  index = std::numeric_limits<size_t>::max();
}

