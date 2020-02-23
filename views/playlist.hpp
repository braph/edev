#ifndef _PLAYLIST_HPP
#define _PLAYLIST_HPP

#include "../database.hpp"
#include "../ui/container.hpp"
#include "../widgets/listwidget.hpp" // XXX

#include <vector>

struct PlaylistColumnFormat {
  enum Justification { Right, Left };

  Database::ColumnID tag;
  short fg;
  short bg;
  short size;
  bool relative;
  enum Justification justify;

  PlaylistColumnFormat()
    : tag(Database::COLUMN_NONE), fg(-1), bg(-1), size(0), relative(false), justify(Left)
  {
  }
};

typedef std::vector<PlaylistColumnFormat> PlaylistColumns;

namespace Views {

class TrackRenderer {
public:
  TrackRenderer(const PlaylistColumns& columns) : m_columns(columns) { }
  void operator()(WINDOW*, int width, const Database::Tracks::Track&, int, bool, bool); // marked, selection
private:
  const PlaylistColumns& m_columns;
};

class Playlist : public ListWidget<Database::Result<Database::Tracks>> {
public:
  Playlist();
  Database::Result<Database::Tracks> playlist;
private:
  TrackRenderer trackRenderer;
};

} // namespace Views

#endif
