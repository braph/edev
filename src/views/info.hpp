#ifndef VIEWS_INFO_HPP
#define VIEWS_INFO_HPP

#include "../application.hpp"
#include "../database.hpp"
#include "../ui/core.hpp"
#include "../ui/mouseevents.hpp"

#include <string>

namespace Views {

class Info : public UI::Pad {
public:
  void draw()                         override;
  void layout(UI::Pos, UI::Size)      override;
  bool handle_key(int)                override;
  bool handle_mouse(MEVENT&)          override;

  void track(Database::Tracks::Track) noexcept;

private:
  struct UrlAndTitle { std::string url, title; };

  Database::Tracks::Track _track;
  UI::MouseEvents<UrlAndTitle> _clickable_urls;
};

} // namespace Views

#endif
