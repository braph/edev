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
  Info(Context& ctxt) : ctxt(ctxt) {}

  void draw()                         override;
  void layout(UI::Pos, UI::Size)      override;
  bool handleKey(int)                 override;
  bool handleMouse(MEVENT&)           override;

  void setCurrentTrack(Database::Tracks::Track);

private:
  struct UrlAndTitle { std::string url, title; };

  Context& ctxt;
  Database::Tracks::Track currentTrack;
  UI::MouseEvents<UrlAndTitle> clickableURLs;

  void drawHeading(int, const char*);
  void drawTag(int, const char*);
  void drawInfo(int, const char*);
  void drawLink(const std::string&, const std::string&);
};

} // namespace Views

#endif
