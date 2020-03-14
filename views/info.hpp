#ifndef VIEWS_INFO_HPP
#define VIEWS_INFO_HPP

#include "../ui.hpp"
#include "../common.hpp"
#include "../player.hpp"
#include "../database.hpp"

#include <string>

namespace Views {

class Info : public UI::Pad {
public:
  Info(Database::Database&, Mpg123Player&);

  void draw();
  void layout(UI::Pos, UI::Size);
  bool handleMouse(MEVENT&);

  void setCurrentTrack(Database::Tracks::Track);

private:
  struct UrlAndTitle { std::string url; std::string title; };

  Database::Database& db;
  Mpg123Player& player;
  Database::Tracks::Track currentTrack;
  UI::MouseEvents<UrlAndTitle> clickableURLs;

  void drawLink(CString, CString);
  void drawHeading(int, CString);
  void drawTag(int, CString);
  void drawInfo(int, CString);
};

} // namespace Views

#endif
