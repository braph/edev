
class VerticalContainer : public GenericContainer {
  void layout() {
    unsigned int yoff = 0;

    for (auto widget : getVisibleWidgets()) {
      widget.with_lock; {
        widget.width = width;

      }

           widget.size=(widget.size.update(width: @size.width))
           widget.pos=(@pos.calc(y: yoff))
           fail WidgetSizeError if widget.size.width > @size.width
           fail WidgetSizeError if yoff + widget.size.height > @size.height
           yoff += widget.size.height
        end
    }

     super
  end
end

