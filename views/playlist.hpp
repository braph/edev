#ifndef _PLAYLIST_HPP
#define _PLAYLIST_HPP

#include "../database.hpp"
#include "../widgets/listwidget.hpp"

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

class TrackRenderer : public ListItemRenderer<Database::Tracks::Track> {
public:
  TrackRenderer(const PlaylistColumns& columns)
  : m_columns(columns)
  {
  }

  //void setFormat(format); -> wants layout
  void render(WINDOW *win, const Database::Tracks::Track &item, int index, bool cursor, bool active); // marked, selection
private:
  const PlaylistColumns& m_columns;
};

typedef ListWidget<Database::Result<Database::Tracks>, TrackRenderer> Playlist;

} // namespace Views

#endif
