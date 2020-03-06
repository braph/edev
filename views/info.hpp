#ifndef _INFO_HPP
#define _INFO_HPP

#include "../ui.hpp"
#include "../player.hpp"
#include "../database.hpp"

#include <string>
#include <utility>

namespace Views {

class Info : public UI::Pad {
public:
  Info(Database&, Mpg123Player&);

  void draw();
  void layout(UI::Pos, UI::Size);
  bool handleMouse(MEVENT&);

  void setCurrentTrack(Database::Tracks::Track);

private:
  Database& db;
  Mpg123Player& player;
  Database::Tracks::Track currentTrack;
  using UrlAndTitle = std::pair<std::string, std::string>;
  UI::MouseEvents<UrlAndTitle> clickableURLs;

  void drawLink(const std::string&, const std::string&);
  void drawHeading(int, const char*);
  void drawTag(int, const char*);
  void drawInfo(int, const char*);
};

} // namespace Views

#endif
