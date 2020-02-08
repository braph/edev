#include <iostream>

// TODO: keep playlist state
static void app() {
  // models...
  //player      = Models::Player.new(client, Config[:audio_system])
  //browser     = Models::Browser.new(client)
  //playlist    = Models::Playlist.new
  //database    = Models::Database.new(client)
  //trackloader = Models::Trackloader.new(client)
  
  /* From client */
  // @trackloader = Trackloader.new(@database)
}

static void setupUI() {
  /* God thought about the configuration */
  /* Construct the views */
  Views::PlayingInfo        v_PlayingInfo();

  Views::Splash             v_Splash();
  Views::PlayList           v_PlayList();
  Views::Browser            v_Browser();
  Views::Info               v_Info();
  Views::Help               v_Help();

  Views::TabBar             v_TabBar();
  Views::Stacked            v_Stacked();

  Views::ProgressBar        v_ProgressBar();
  Views::VerticalContainer  v_MainWindow();

  for (const auto& w : Config.main_widgets) {
    auto& mw = v_MainWindow;
    if (w == "playinginfo")      mw.add(v_PlayingInfo);
    else if (w == "tabbar")      mw.add(v_TabBar);
    else if (w == "stacked")     mw.add(v_Stacked);
    else if (w == "progressbar") mw.add(v_ProgressBar);
    else assert();
  }

  for (const auto& w : Config.tabs_widgets) {
    UI::Window *widget;
    if (w == "playinginfo")      widget = v_PlayingInfo;
    else if (w == "tabbar")      widget = v_TabBar;
    else if (w == "stacked")     widget = v_Stacked;
    else if (w == "progressbar") widget = v_ProgressBar;
    else assert();

    v_TabBar.add(w);
    v_Stacked.add(widget);
    v_TabBar.changed  = v_Stacked.show; // XXX?
    v_Stacked.changed = v_TabBar.set;   // XXX?
  }

  /* Set up callbacks */
  v_ProgressBar.onClick /*??*/ = [](float percent){
    player.setPercent(percent);
  };

  for (;;) {
    /* Player related stuff */
    player;

    if (player.events) {
      v_PlayingInfo.setState(player.state); /* STOPPED/PAUSED/PLAYING */
      v_PlayingInfo.setPosition(player.position, player.length);
    }

    if (playlist.events) {
      if (playlist.events & Playlist::Event::CurrentChanged)
        v_PlayingInfo.setTrack(playlist[playlist.current_playing]);
    }

    v_ProgressBar.setPercent(player.getPosition());

    time_t now = time(NULL);

    if (Config::prefetch && now % 5 == 0) {
      if (player.state != PLAYING)
        continue;

       current_download_track = nil

       loop do
          sleep 5 // clock() % 5?

          next_track = playlist.get_next_pos
          next if current_download_track == next_track

          if player.length > 30 and player.position_percent > 0.5
             trackloader.get_track_file(playlist[next_track]['url'])
             current_download_track = next_track
             sleep 5
          end
       end
    }
  }
}

#if 0
     UI::Canvas.run do
        # ... operations ...
        operations = Operations::Operations.new

        operations.register(:reload,  &browser.method(:reload))
        operations.register(:update,  &database.method(:update))
        operations.register(:refresh) { UI::Canvas.update_screen(true, true) }
        Operations::Player.new(operations, player)
        Operations::Browser.new(operations, browser, playlist)
        Operations::Playlist.new(operations, playlist, player, trackloader)

        # ... views ...
        main_w = UI::Canvas.sub(Views::MainWindow)

        # next operations may take some time, espacially the ones
        # using the database (browser), so we put this inside a thread

        # ... controllers ...
        view_ops = Operations::Operations.new
        Controllers::MainWindow.new(main_w, view_ops)
        Controllers::Browser.new(main_w.browser, browser, view_ops, operations)
        Controllers::Playlist.new(main_w.playlist, playlist, view_ops, operations)
        Controllers::Help.new(main_w.help, view_ops)
        Controllers::Info.new(main_w.info, player, playlist, trackloader, database, view_ops)
        main_w.progressbar.attach(player)
        main_w.playinginfo.attach(playlist, player)

        # ... events ...
        database.events.on(:update_finished, &browser.method(:reload))
        player.events.on(:stop) do |reason|
         operations.send(:'playlist.play_next') if reason == :track_completed
        end
        
        # ... bindings ...
        Bindings.bind_view(:global, main_w, view_ops, operations)
        %w(splash playlist browser info help).each do |w|
         Bindings.bind_view(w, main_w.send(w), view_ops, operations)
        end

        player.stop#RAELY

        # Preload playlist
        if (n = Config[:playlist_load_newest]) > 0
         r = client.database.select(
            order_by: 'date DESC,album,number',
            limit: n
         )
         playlist.add(*r)
        end

        rescue
         Application.log(self, $!)
        end

        UI::Canvas.update_screen
        UI::Input.start_loop
     end
  rescue
     puts "Error: #{$!}"
     Application.log(self, $!)
  end
#endif
