
// We dont use a main window!

namespace Ektoplayer {
  namespace Views {
    class MainWindow : public UI::VerticalContainer {
      public:
#if 0
        ProgressBar progressbar;
        PlayingInfo playinginfo;
        TabBar      tabbar;
        attr_reader :progressbar, :playinginfo, :tabbar
        attr_reader :windows, :splash, :playlist, :browser, :info, :help
#endif
        MainWindow() {

         def initialize(**opts)
            super(**opts)
        }
    }
  }
}

MainWindow(unsigned int height, unsigned int width)
: playingInfo(2, width)
, progressBar(1, width)
, tabBar(1, width)
, windowStack(height - 4, width)
, help(height - 4, width)
, info(height - 4, width)
, splash(height - 4, width)
, browser(height - 4, width)
, playlist(height - 4, width)
{
  help.setVisible(false);
  info.setVisible(false);
  splash.setVisible(false);
  browser.setVisible(false);
  playlist.setVisible(false);

  for (auto widget : Config::getStringList("tabs.widgets")) {
    tabbar.add(widget);
    if (widget == "help")
      windowStack.add(help);
    // ...
  }
}

            Config[:'main.widgets'].each { |w| add(send(w)) }

            self.selected=(@windows)
            @windows.selected=(@splash)
         end

         def layout
            height = @size.height

            @playinginfo.size=(@size.update(height: 2))
            @progressbar.size=(@size.update(height: 1))
            @tabbar.size=(@size.update(height: 1))

            height -= 2 if @playinginfo.visible?
            height -= 1 if @progressbar.visible?
            height -= 1 if @tabbar.visible?

            @windows.size=(@size.update(height: height))

            super
         end
      end
   end
end
