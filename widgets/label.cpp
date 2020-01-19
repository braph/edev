class LabelWidget : public Window {
private:
  struct {
    short top, bottom;
    short left, right;
  } pad;

  int attributes;

public:

  void draw() {
    erase();
    wmove(pad.top, pad.left);
    attron(attribute/**/);
    this << label;
  }

  void attributes(int attrs) {
    if (_attributes == attrs) return;
    _attributes = attrs;
    want_redraw();
  }

  void text=

};

      attr_reader :text, :pad, :attributes

      def initialize(text: '', attributes: 0, pad: {}, **opts)
         super(**opts)
         @text, @attributes, @pad = text.to_s, attributes, Hash.new(0)
         @pad.update pad
      end

      def text=(new)
         return if @text == new
         with_lock { @text = new.to_s; want_redraw }
      end

      def pad=(new)
         return if @pad == new
         with_lock { @pad.update!(new); want_redraw }
      end

      def fit
         self.size=(Size.new(
            height: @pad[:top] + @pad[:bottom] + 1 + @text.count(?\n),
            width:  @pad[:left] + @pad[:right] + @text.split(?\n).max.size
         ))

         self
      end
   end
