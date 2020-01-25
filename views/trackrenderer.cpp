namespace Ektoplayer {
  namespace Views {
    class TrackRenderer : public UI::ListItemRenderer {
      private:
        int width;
        format;

      public:
        //void setFormat(format); -> wants layout
    }
  }
}

// assert(column_format)
void render(WINDOW *win, const &ITEM, index, bool selected, bool marked, bool selection) {
  int additional_attributes = 0;
  if (marked)   additional_attributes |= A_BOLD;
  if (selected) additional_attributes |= A_STANDOUT;

  // TODO: Move this to an own renderer?
 if selection
    color = Theme[:'list.item_selection']
 elsif index % 2 == 0
    color = Theme[:'list.item_even']
 else
    color = Theme[:'list.item_odd']
 end

 scr.attrset(color | additional_attributes)
 scr.addstr("[#{item}]".ljust(@width))
 // ---- SNAP
 
 int y = getcury(win);
 int x = 0; // lef_pad;

 int width = m_width - column_format.size() - 1; // Drop separating space char
 int _100Percent = 0;
 for (const auto &column : column_format) {
   width -= column.size; // Remove the fixed sizes
   _100Percent += column.rel; // The sum of all rel's shall be 100, but may be another value
 }

 for (const auto &column : column_format) {
   if (selection)
     wattrset(win, Colors.get("list.item_selection") | additional_attributes);
   else
     wattrset(win, Colors.get("", column.fg, column.bg, column.attributes));

   const char* v = item[column.tag]; // RUBY %.2d
   int len = strlen(v);
   int colwidth;

   if (column.size)
     colwidth = column.size;
   else
     colwidth = width * _100Percent / column.rel;

   if (column.justify == Left)
     mvwaddnstr(win, y, x, v, colwidth);
   else if (column.justify == Right) {
     mvwhline(win, colwidth - len, ' '); // TODO
     mvwaddnstr(win, y, x, v, colwidth); // TODO
   }

   // if not last column
   waddch(win, ' ');
   x += colwidth;
 }
}
