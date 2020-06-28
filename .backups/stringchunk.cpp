// Old, ineffective shrink algorithm

void StringChunk :: shrink_to_fit(IDRemap& old_id_new_id) {
  StringChunk newchunk;
  newchunk.reserve(_data.size());

  struct IDAndLength { int id; int length; };
  std::vector<IDAndLength> idAndLengthsByLastChar[256];

  // Assuming most of the strings end with [A-Za-z0-9]
  const size_t avg = old_id_new_id.size() / (26 + 26 + 10);
  for (int c = 'a'; c <= 'z'; ++c) idAndLengthsByLastChar[c].reserve(avg);
  for (int c = 'A'; c <= 'Z'; ++c) idAndLengthsByLastChar[c].reserve(avg);
  for (int c = '0'; c <= '9'; ++c) idAndLengthsByLastChar[c].reserve(avg);

  for (auto& pair : old_id_new_id) {
    const char* s = this->get(pair.first);
    const int len = std::strlen(s);
    if (len) {
      const int lastChar = reinterpret_cast<const unsigned char*>(s)[len-1];
      idAndLengthsByLastChar[lastChar].push_back({pair.first, len});
    }
    else {
      pair.second = 0;
    }
  }

  // Sort by length
  for (auto& idAndLengths : idAndLengthsByLastChar)
    if (idAndLengths.size())
      std::sort(idAndLengths.begin(), idAndLengths.end(),
          [](const IDAndLength& a, const IDAndLength& b){ return a.length > b.length; });

  // Add strings in the right order to the stringchunk and store the new ID
  int chunkSearchPos = 0;
  for (const auto& idAndLengths : idAndLengthsByLastChar)
    if (idAndLengths.size()) {
      for (const auto& idAndLength : idAndLengths) {
        CString s(this->get(idAndLength.id), size_t(idAndLength.length));
        int newId = newchunk.find(s, chunkSearchPos);
        if (!newId)
          newId = newchunk.add_unchecked(s);
        old_id_new_id[idAndLength.id] = newId;
      }

      chunkSearchPos = newchunk.size();
    }

  _data = std::move(newchunk._data);
  _data.shrink_to_fit();
}
