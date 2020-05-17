#ifndef VIEWS_INFO_HPP
#define VIEWS_INFO_HPP

#include "../application.hpp"
#include "../database.hpp"
#include "../ui.hpp"
#include "../ui/mouseevents.hpp"

#include <string>

namespace Views {

class Info : public UI::Pad {
public:
  void draw()                         override;
  void layout(UI::Pos, UI::Size)      override;
  bool handleKey(int)                 override;
  bool handleMouse(MEVENT&)           override;

  void setCurrentTrack(Database::Tracks::Track);

private:
  struct UrlAndTitle { std::string url, title; };

  Database::Tracks::Track currentTrack;
  UI::MouseEvents<UrlAndTitle> clickableURLs;

  void drawHeading(int, const char*)       noexcept;
  void drawTag(int, const char*)           noexcept;
  void drawInfo(int, const char*)          noexcept;
  void drawLink(std::string, std::string)  noexcept;
};

} // namespace Views

#endif
