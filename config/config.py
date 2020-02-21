#!/usr/bin/python

import sys, os

default = object()
c_default = object()
lateinit = object()

# Parameters:
# - type: The C storage type of the option
# - set:  The function that is called on the input string to retrieve the config type
# - help: Option description
# - default: String that is passed to the `set` function
# - c_default: Default value converted to the C storage type
# - lateinit:  If set to True, set(default) is called to initialize the value

DEFAULT_PLAYINGINFO_FORMAT_TOP = '''"\
  '<< '{fg=black} title{fg=yellow,bold} ' >>'{fg=black}"'''

DEFAULT_PLAYINGINFO_FORMAT_BOTTOM = '''"\
  artist{fg=blue,bold} ' - ' album{fg=red,bold} ' (' year{fg=cyan} ')'"'''

DEFAULT_PLAYINGINFO_FORMAT_TOP_256 = '''"\
  '<< '{fg=236} title{fg=178,bold} ' >>'{fg=236}"'''

DEFAULT_PLAYINGINFO_FORMAT_BOTTOM_256 = '''"\
  artist{fg=24,bold} ' - ' album{fg=160,bold} ' (' year{fg=37} ')'"'''

DEFAULT_PLAYLIST_COLUMNS = '''"\
  number{fg=magenta size=3} artist{fg=blue size=25%} album{fg=red size=30%} \
  title {fg=yellow size=33%} styles{fg=cyan size=20%} bpm{fg=green size=3 right}"'''

DEFAULT_PLAYLIST_COLUMNS_256 = '''"\
  number{fg=97 size=3} artist{fg=24 size=25%} album{fg=160 size=30%} \
  title {fg=178 size=33%} styles{fg=37 size=20%} bpm{fg=28 size=3 right}"'''

options = [
    ('database_file', {
        type: 'std::string', set: 'Filesystem::expand',
        default: '"~/.config/ektoplayer/meta.db"',
        help: 'Database file for storing ektoplazm metadata',
        lateinit: True
        }),
    ('log_file', {
        type: 'std::string', set: 'Filesystem::expand',
        default: '"~/.config/ektoplayer/ektoplayer.log"',
        help: 'File used for logging',
        lateinit: True
        }),
    ('temp_dir', {
        type: 'std::string', set: 'Filesystem::expand',
        default: '"/tmp/.ektoplazm"',
        help: 'Temporary dir for downloading mp3 files.\nThey will be moved to `cache_dir` after the download completed and was successful.\nDirectory will be created if it does not exist, parent directories will not be created.',
        }),
    ('cache_dir', {
        type: 'std::string', set: 'Filesystem::expand',
        default: '"~/.cache/ektoplayer"',
        help: 'Directory for storing cached mp3 files',
        lateinit: True
        }),
    ('archive_dir', {
        type: 'std::string', set: 'Filesystem::expand',
        default: '"~/.config/ektoplayer/archives"',
        help: 'Where to search for downloaded MP3 archives',
        lateinit: True
        }),
    ('download_dir', {
        type: 'std::string', set: 'Filesystem::expand',
        default: '"/tmp"',
        help: 'Where to store downloaded MP3 archives',
        }),
    ('auto_extract_to_archive_dir', {
        type: 'bool', set: 'opt_parse_bool',
        default: 'true',
        help: 'Enable/disable automatic extraction of downloaded MP3\narchives from `download_dir` to `archive_dir`',
        }),
    ('delete_after_extraction', {
        type: 'bool', set: 'opt_parse_bool',
        default: 'true',
        help: 'In combination `with auto_extract_to_archive_dir`:\nDelete zip archive after successful extraction',
        }),
    ('playlist_load_newest', {
        type: 'int', set: 'opt_parse_int',
        default: '1000',
        help: 'How many tracks from database should be added to the playlist on application start.',
        }),
    ('use_cache', {
        type: 'bool', set: 'opt_parse_bool',
        default: 'true',
        help: "Enable/disable local mp3 cache.\nIf this option is disabled, the downloaded mp3 files won't be moved from `cache_dir`.\nInstead they will reside in `temp_dir` and will be deleted on application exit.",
        }),
    ('prefetch', {
        type: 'bool', set: 'opt_parse_bool',
        default: 'true',
        help: 'Enable prefetching the next track do be played',
        }),
    ('small_update_pages', {
        type: 'int', set: 'opt_parse_int',
        default: '5',
        help: 'How many pages should be fetched after start',
        }),
    ('use_colors', {
        type: 'int', set: 'opt_parse_use_colors',
        default: '"auto"',
        c_default: '-1',
        help: 'Choose color capabilities. auto|mono|8|256',
        }),
    ('audio_system', {
        type: 'std::string', set: 'std::string',
        default: '"pulse,alsa,jack,oss"',
        help: 'Set output audio system. See option `-o` in mpg123(1)',
        }),
    ('threads', {
        type: 'int', set: 'opt_parse_threads',
        default: '20',
        help: 'Number of download threads during database update',
        }),
    ('playlist.columns', {
        type: 'PlaylistColumns', set: 'opt_parse_playlist_columns',
        default: DEFAULT_PLAYLIST_COLUMNS,
        help: 'Columns of playlist',
        lateinit: True
        }),
    ('playlist.columns_256', {
        type: 'PlaylistColumns', set: 'opt_parse_playlist_columns',
        default: DEFAULT_PLAYLIST_COLUMNS_256,
        help: 'Columns of playlist (256 colors)',
        lateinit: True
        }),
    ('browser.columns', {
        type: 'PlaylistColumns', set: 'opt_parse_playlist_columns',
        default: DEFAULT_PLAYLIST_COLUMNS,
        help: 'Columns of browser',
        lateinit: True
        }),
    ('browser.columns_256', {
        type: 'PlaylistColumns', set: 'opt_parse_playlist_columns',
        default: DEFAULT_PLAYLIST_COLUMNS_256,
        help: 'Columns of browser (256 colors)',
        lateinit: True
        }),
    ('progressbar.display', {
        type: 'bool', set: 'opt_parse_bool',
        default: 'true',
        help: 'Enable/disable progressbar',
        }),
    ('progressbar.progress_char', {
        type: 'wchar_t', set: 'opt_parse_char',
        default: "'~'",
        help: 'Character used for displaying playing progress',
        }),
    ('progressbar.rest_char', {
        type: 'wchar_t', set: 'opt_parse_char',
        default: "'~'",
        help: 'Character used for the rest of the line',
        }),
    ('playinginfo.display', {
        type: 'bool', set: 'opt_parse_bool',
        default: 'true',
        help: 'Enable/display playinginfo',
        }),
    ('playinginfo.format_top', {
        type: 'PlayingInfoFormat', set: 'opt_parse_playinginfo_format',
        default: DEFAULT_PLAYINGINFO_FORMAT_TOP,
        help: 'Format of first line in playinginfo',
        lateinit: True
        }),
    ('playinginfo.format_top_256', {
        type: 'PlayingInfoFormat', set: 'opt_parse_playinginfo_format',
        default: DEFAULT_PLAYINGINFO_FORMAT_TOP_256,
        help: 'Format of first line in playinginfo (256 colors)',
        lateinit: True
        }),
    ('playinginfo.format_bottom', {
        type: 'PlayingInfoFormat', set: 'opt_parse_playinginfo_format',
        default: DEFAULT_PLAYINGINFO_FORMAT_BOTTOM,
        help: 'Format of second line in playinginfo',
        lateinit: True
        }),
    ('playinginfo.format_bottom_256', {
        type: 'PlayingInfoFormat', set: 'opt_parse_playinginfo_format',
        default: DEFAULT_PLAYINGINFO_FORMAT_BOTTOM_256,
        help: 'Format of second line in playinginfo (256 colors)',
        lateinit: True
        }),
    ('tabbar.display', {
        type: 'bool', set: 'opt_parse_bool',
        default: 'true',
        help: 'Enable/disable tabbar TODO: do we need this?',
        }),
    ('tabs.widgets', {
        type: 'std::vector<std::string>', set: 'opt_parse_tabs_widgets',
        default: '"splash,playlist,browser,info,help"',
        c_default: '{"splash","playlist","browser","info","help"}',
        help: 'Specify widget order of tabbar (left to right)',
        }),
    ('main.widgets', {
        type: 'std::vector<std::string>', set: 'opt_parse_main_widgets',
        default: '"playinginfo,tabbar,windows,progressbar"',
        c_default: '{"playinginfo","tabbar","windows","progressbar"}',
        help: 'Specify widgets to show (up to down)',
    }),
]

NAME = 0

with open('options.declare.hpp', 'w') as fh:
    options.sort(key=lambda o: o[NAME])
    options.sort(key=lambda o: len(o[NAME]))
    options.sort(key=lambda o: o[1][type])
    options.sort(key=lambda o: len(o[1][type]))
    for name, o in options:
        print('extern', o[type], name.replace('.', '_'), end=';\n', file=fh)

with open('options.set.cpp', 'w') as fh:
    options.sort(key=lambda o: o[NAME])
    options.sort(key=lambda o: len(o[NAME]))
    for name, o in options:
        print('else if (option == \"%s\") %s = %s(value);' % (
            name, name.replace('.', '_'), o[set]
            ), file=fh)

with open('options.define.cpp', 'w') as fh:
    options.sort(key=lambda o: o[NAME])
    options.sort(key=lambda o: len(o[NAME]))
    options.sort(key=lambda o: o[1][type])
    options.sort(key=lambda o: len(o[1][type]))

    pad = max(map(lambda o: len(o[NAME]), options))
    fmt = '%%-%ds Config :: %%s' % pad

    for name, o in options:
        print(fmt % (o[type], name.replace('.', '_')), end='', file=fh)

        if o.get(lateinit, False):
            print(' /* will be initialized later */', end='', file=fh)
        else:
            if o.get(c_default, None):
                print(' =', o[c_default], end='', file=fh);
            else:
                print(' =', o[default], end='', file=fh);

        print(';', file=fh)

with open('options.initialize.cpp', 'w') as fh:
    options.sort(key=lambda o: o[NAME])
    options.sort(key=lambda o: len(o[NAME]))
    options.sort(key=lambda o: o[1][type])
    options.sort(key=lambda o: len(o[1][type]))

    for name, o in options:
        if o.get(lateinit, False):
            print('%s = %s(%s);' % (
                name.replace('.', '_'), o[set], o[default]
                ), file=fh)

