#ifndef _TABBAR_CPP
#define _TABBAR_CPP

namespace Ektoplayer {
  namespace Views {
    typedef void (*changed)(int);

    class TabBar : public UI::Window {
      private;
        std::vector<std::string> m_tabs;
      public:
        void add(const std::string&);
        void select(unsigned int);
    };
  }
}

void Tabbar :: add(const std::string &label) {
  //with_lock...
  tabs.push_back(label);
  want_redraw();
}

void Tabbar :: select(unsigned int index) {
  index = index % tabs.size();
  return if index == current;

  //with_lock...
  current = index;
  want_redraw();
}

void Tabbar :: draw() {
    self.pad_size=(@size.update(height: 1));
    werase(win);
    wmove(win, 0, 0);

    unsigned int i = 0;
    for (const auto &label : tabs) {
      if (i == current)
        wattrset(win, Theme::get("tabbar.selected"));
      else
        wattrset(win, Theme::get("tabbar.unselected"));

      waddstr(win, label.c_str());
      waddch(' ');
    }
}

void clicked(...) {
  int x, y;

  // TODO: check for button?

  unsigned int i = 0;
  unsigned int p = 0;
  for (const auto &label : tabs) {
    if (x >= p && x <= p + label.size()) {
      if (changed)
        changed(...);
      break;
    }
    i++;
    p += label.size() + 1;
  }
}
