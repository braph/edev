#ifndef VIEWS_PLAYLIST_HPP
#define VIEWS_PLAYLIST_HPP

#include "../application.hpp"
#include "../database.hpp" // XXX ger rid of this
#include "../ui/container.hpp"
#include "../widgets/listwidget.hpp" // XXX
#include "../lib/steppablesearch.hpp"

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

class Playlist : public ListWidget<std::vector<Database::Tracks::Track>> {
public:
  Playlist(Context&);
  std::vector<Database::Tracks::Track> playlist;
  bool handleKey(int);
private:
  Context& ctxt;
  TrackRenderer trackRenderer;
  SteppableSearch<std::vector<Database::Tracks::Track>> trackSearch;
};

} // namespace Views

#endif
