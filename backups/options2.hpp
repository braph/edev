reg("database_file",
    "Database file for storing ektoplazm metadata",
    CONFIG_DIR + PATH_SEP "meta.db",
    Filesystem::expand);

reg("log_file",
    "File used for logging",
    CONFIG_DIR + PATH_SEP "ektoplayer.log",
    Filesystem::expand);

reg("temp_dir",
   "Temporary dir for downloading mp3 files.\n"
   "They will be moved to `cache_dir` after the download completed and was successful.\n"
   "Directory will be created if it does not exist, parent directories will not be created.",
   "/tmp/.ektoplazm",
   Filesystem::expand);

reg("cache_dir",
   "Directory for storing cached mp3 files",
   HOME + PATH_SEP ".cache" PATH_SEP "ektoplayer",
   Filesystem::expand);

reg("archive_dir",
   "Where to search for downloaded MP3 archives",
   CONFIG_DIR + PATH_SEP "archives",
   Filesystem::expand);

reg("download_dir",
   "Where to store downloaded MP3 archives",
   "/tmp",
   Filesystem::expand);

reg("auto_extract_to_archive_dir",
    "Enable/disable automatic extraction of downloaded MP3\n"
    "archives from `download_dir` to `archive_dir`",
    "true",
    opt_parse_bool);

reg("delete_after_extraction",
   "In combination `with auto_extract_to_archive_dir`:\n"
   "Delete zip archive after successful extraction",
   "true",
   opt_parse_bool);

reg("playlist_load_newest",
   "How many tracks from database should be added to the playlist on application start.",
   "1000",
   opt_parse_int);

reg("use_cache",
    "Enable/disable local mp3 cache.\n"
    "If this option is disabled, the downloaded mp3 files won't be moved"
    " from `cache_dir`. Instead they will reside in `temp_dir` and will be deleted on application exit.",
    "true",
    opt_parse_bool);

reg("prefetch",
   "Enable prefetching the next track do be played",
   "true",
   opt_parse_bool);

reg("small_update_pages",
   "How many pages should be fetched after start",
   "5",
   opt_parse_int);

reg("use_colors",
   "Choose color capabilities. auto|mono|8|256",
   "auto",
   validate_use_colors);

reg("audio_system",
    "Set output audio system. See option `-o` in mpg123(1)",
    "pulse,alsa,jack,oss",
    opt_parse_string);

reg("threads",
   "Number of download threads during database update",
   "20",
   validate_threads);

// - Playlist
reg("playlist.format",
    "Format of playlist columns",
   DEFAULT_PLAYLIST_FORMAT,
   parse_playlist_format);

reg("playlist.format_256",
    "Format of playlist columns (256 colors)",
     DEFAULT_PLAYLIST_FORMAT_256,
     parse_playlist_format);

// - Browser
reg("browser.format",
    "Format of browser columns",
   DEFAULT_PLAYLIST_FORMAT,
   parse_playlist_format);

reg("browser.format_256",
    "Format of browser columns (256 colors)",
   DEFAULT_PLAYLIST_FORMAT_256,
   parse_playlist_format);

// - Progressbar
reg("progressbar.display",
   "Enable/disable progressbar",
   "true",
   opt_parse_bool);

reg("progressbar.progress_char",
   "Character used for displaying playing progress",
   "~",
   opt_parse_string); // TODO: is a single character

reg("progressbar.rest_char",
   "Character used for the rest of the line",
   "~",
   opt_parse_string); // TODO: is a single character

// - Playinginfo
reg("playinginfo.display",
   "Enable/display playinginfo",
   "true",
   opt_parse_bool);

reg("playinginfo.format_top",
    "Format of first line in playinginfo",
    DEFAULT_PLAYINGINFO_FORMAT_TOP,
    opt_parse_playinginfo_format);

reg("playinginfo.format_top_256",
    "Format of first line in playinginfo (256 colors)",
    DEFAULT_PLAYINGINFO_FORMAT_TOP_256,
    opt_parse_playinginfo_format);

reg("playinginfo.format_bottom",
    "Format of second line in playinginfo",
    DEFAULT_PLAYINGINFO_FORMAT_BOTTOM,
    opt_parse_playinginfo_format);

reg("playinginfo.format_bottom_256",
    "Format of second line in playinginfo (256 colors)",
    DEFAULT_PLAYINGINFO_FORMAT_BOTTOM_256,
    opt_parse_playinginfo_format);

// - Tabbar
reg("tabbar.display",       
   "Enable/disable tabbar",
   "true",
   opt_parse_bool);

reg("tabs.widgets",
    "Specify widget order of tabbar (left to right)",
    "splash,playlist,browser,info,help",
    opt_parse_string); // TODO

reg("main.widgets",
    "Specify widgets to show (up to down)",
    "playinginfo,tabbar,windows,progressbar",
    opt_parse_string); // TODO
