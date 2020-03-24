#ifndef VIEWS_INFO_HPP
#define VIEWS_INFO_HPP

#include "../ui.hpp"
#include "../ui/mouseevents.hpp"
#include "../lib/cstring.hpp"
#include "../context.hpp"
#include "../database.hpp" // XXX Get rid of this

#include <string>

namespace Views {

class Info : public UI::Pad {
public:
  Info(Context& ctxt) : ctxt(ctxt) {}

  void draw();
  void layout(UI::Pos, UI::Size);
  bool handleMouse(MEVENT&);

  void setCurrentTrack(Database::Tracks::Track);

private:
  struct UrlAndTitle { std::string url; std::string title; };

  Context& ctxt;
  Database::Tracks::Track currentTrack;
  UI::MouseEvents<UrlAndTitle> clickableURLs;

  void drawLink(CString, CString);
  void drawHeading(int, CString);
  void drawTag(int, CString);
  void drawInfo(int, CString);
};

} // namespace Views

#endif
