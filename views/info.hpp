#ifndef _INFO_HPP
#define _INFO_HPP

#include "../ui.hpp"
#include "../database.hpp"

namespace Views {

class Info : public UI::Pad {
public:
  Info(Database&);

  void draw();
  void layout(UI::Pos, UI::Size);

  void setCurrentTrack(Database::Tracks::Track);

private:
  Database& db;
  Database::Tracks::Track currentTrack;

  void drawURL(const char*, const char*);
  void drawHeading(int, const char*);
  void drawTag(int, const char*);
  void drawInfo(int, const char*);
};

} // namespace Views

#endif
