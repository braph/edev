#ifndef _COLORFADER_HPP
#define _COLORFADER_HPP

#include <vector>

namespace UI {
  class ColorFader {
    private:
      std::vector<int> colors;

    public:
      ColorFader(std::vector<int>);
      int fade(unsigned int pos, unsigned int size);
      int fade2(unsigned int pos, unsigned int size);

      static int fade(const std::vector<int>& colors, unsigned int pos, unsigned int size);
      static int fade2(const std::vector<int>& colors, unsigned int pos, unsigned int size);
  };

#if asdfsdf
  class ColorFaderSimple : public ColorFader {
    private:
      int color;

    public:
      inline ColorFaderSimple(int);
      inline int fade(unsigned int pos, unsigned int size) { return m_color; }
  };
#endif
}

#endif
