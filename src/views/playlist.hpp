#ifndef VIEWS_PLAYLIST_HPP
#define VIEWS_PLAYLIST_HPP

#include "../application.hpp"
#include "../database.hpp"
#include "../ui/container.hpp"
#include "../widgets/listwidget.hpp"

#include <lib/steppablesearch.hpp>
#include <lib/staticvector.hpp>

struct PlaylistColumnFormat {
  enum class Justify : unsigned char { Left, Right };

  Database::ColumnID tag;
  short fg;
  short bg;
  short size;
  bool relative;
  Justify justify;

  constexpr PlaylistColumnFormat(
    Database::ColumnID tag_ = Database::COLUMN_NONE,
    short fg_ = -1,
    short bg_ = -1,
    short size_ = 0,
    bool relative_ = false,
    Justify justify_ = Justify::Left)
  : tag(tag_)
  , fg(fg_)
  , bg(bg_)
  , size(size_)
  , relative(relative_)
  , justify(justify_)
  {}
};

using PlaylistColumns = StaticVector<PlaylistColumnFormat, 10>;

namespace Views {

class TrackRenderer {
public:
  TrackRenderer(const PlaylistColumns& columns)
    : m_columns(columns)
  {}

  void operator()(WINDOW*, int, int, const Database::Tracks::Track&, int, unsigned);
private:
  const PlaylistColumns& m_columns;
};

class Playlist : public ListWidget<std::vector<Database::Tracks::Track>> {
public:
  Playlist();

  bool handle_key(int) override;

  std::vector<Database::Tracks::Track> playlist;

private:
  TrackRenderer _track_renderer;
  SteppableSearchBidirectional<std::vector<Database::Tracks::Track>> _track_search;
};

} // namespace Views

#endif
