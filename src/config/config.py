#!/usr/bin/python

import sys, os
os.chdir(os.path.dirname(sys.argv[0]))

default   = object()
c_default = object()
lateinit  = object()

# Parameters:
# - type: The C storage type of the option
# - set:  The function that is called on the input string to retrieve the config type
# - help: Option description
# - default: String that is passed to the `set` function
# - c_default: Default value converted to the C storage type
# - lateinit:  If set to True, $set($default) is called to initialize the value

colfmt = lambda s: s.replace("column(", "static_cast<Database::ColumnID>(Database::")


INFOLINE_FORMAT_TOP_MONO = '''"\
'<< ' title{bold} ' >>'"'''

INFOLINE_FORMAT_TOP_MONO_CVALUE = colfmt('''{
{column(COLUMN_NONE), "<< "},
{column(TRACK_TITLE), "",  -1, -1, A_BOLD},
{column(COLUMN_NONE), " >>"}}''')

INFOLINE_FORMAT_BOTTOM_MONO = '''"\
artist{bold} ' - ' album{bold} ' (' year{} ')'"'''

INFOLINE_FORMAT_BOTTOM_MONO_CVALUE = colfmt('''{
{column(TRACK_ARTIST),  "",     -1, -1, A_BOLD},
{column(COLUMN_NONE),   " - "},
{column(ALBUM_TITLE),   "",     -1, -1, A_BOLD},
{column(COLUMN_NONE),   " ("},
{column(ALBUM_YEAR),    "",     -1, -1, 0},
{column(COLUMN_NONE),   ")"}
}''')


INFOLINE_FORMAT_TOP_8 = '''"\
'<< '{fg=black} title{fg=yellow,bold} ' >>'{fg=black}"'''

INFOLINE_FORMAT_TOP_8_CVALUE = colfmt('''{
{column(COLUMN_NONE), "<< ", COLOR_BLACK},
{column(TRACK_TITLE), "",    COLOR_YELLOW, -1, A_BOLD},
{column(COLUMN_NONE), " >>", COLOR_BLACK}}''')

INFOLINE_FORMAT_BOTTOM_8 = '''"\
artist{fg=blue,bold} ' - ' album{fg=red,bold} ' (' year{fg=cyan} ')'"'''

INFOLINE_FORMAT_BOTTOM_8_CVALUE = colfmt('''{
{column(TRACK_ARTIST),  "",     COLOR_BLUE, -1, A_BOLD},
{column(COLUMN_NONE),   " - "},
{column(ALBUM_TITLE),   "",     COLOR_RED, -1, A_BOLD},
{column(COLUMN_NONE),   " ("},
{column(ALBUM_YEAR),    "",     COLOR_CYAN, -1, 0},
{column(COLUMN_NONE),   ")"}
}''')


INFOLINE_FORMAT_TOP_256 = '''"\
'<< '{fg=236} title{fg=178,bold} ' >>'{fg=236}"'''

INFOLINE_FORMAT_TOP_256_CVALUE = colfmt('''{
{column(COLUMN_NONE), "<< ", 236},
{column(TRACK_TITLE), "",    178, -1, A_BOLD},
{column(COLUMN_NONE), " >>", 236}}''')

INFOLINE_FORMAT_BOTTOM_256 = '''"\
artist{fg=24,bold} ' - ' album{fg=160,bold} ' (' year{fg=37} ')'"'''

INFOLINE_FORMAT_BOTTOM_256_CVALUE = colfmt('''{
{column(TRACK_ARTIST),  "",     24, -1, A_BOLD},
{column(COLUMN_NONE),   " - "},
{column(ALBUM_TITLE),   "",     160, -1, A_BOLD},
{column(COLUMN_NONE),   " ("},
{column(ALBUM_YEAR),    "",     37, -1, 0},
{column(COLUMN_NONE),   ")"}
}''')


PLAYLIST_COLUMNS = '''"\
number{fg=magenta size=3} artist{fg=blue size=25%} album{fg=red size=30%} \
title {fg=yellow size=33%} styles{fg=cyan size=20%} bpm{fg=green size=3 right}"'''

PLAYLIST_COLUMNS_CVALUE = colfmt('''{
{column(TRACK_NUMBER),  COLOR_MAGENTA,  -1, 3},
{column(TRACK_ARTIST),  COLOR_BLUE,     -1, 25, true},
{column(ALBUM_TITLE),   COLOR_RED,      -1, 30, true},
{column(TRACK_TITLE),   COLOR_YELLOW,   -1, 33, true},
{column(ALBUM_STYLES),  COLOR_CYAN,     -1, 20, true},
{column(TRACK_BPM),     COLOR_GREEN,    -1, 3}}''')

PLAYLIST_COLUMNS_256 = '''"\
number{fg=97 size=3} artist{fg=24 size=25%} album{fg=160 size=30%} \
title {fg=178 size=33%} styles{fg=37 size=20%} bpm{fg=28 size=3 right}"'''

PLAYLIST_COLUMNS_256_CVALUE = colfmt('''{
{column(TRACK_NUMBER),  97,   -1, 3},
{column(TRACK_ARTIST),  24,   -1, 25, true},
{column(ALBUM_TITLE),   160,  -1, 30, true},
{column(TRACK_TITLE),   178,  -1, 33, true},
{column(ALBUM_STYLES),  37,   -1, 20, true},
{column(TRACK_BPM),     28,   -1, 3}}''')


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
#    ('temp_dir', {
#        type: 'std::string', set: 'Filesystem::expand',
#        default: '"/tmp/.ektoplazm"',
#        help: 'Temporary dir for downloading files. See `cache_dir`, `archive_dir` and `download_dir`.\nDirectory will be created if it does not exist, parent directories will not be created.',
#        }),
    ('cache_dir', {
        type: 'std::string', set: 'Filesystem::expand',
        default: '"~/.cache/ektoplayer"',
        help: 'Directory for storing mp3 files. If empty, the downloaded mp3 files won\'t be moved to `cache_dir`. Instead they will be kept `temp_dir` and will be deleted on application start and exit.\nDirectory will be created if it does not exist, parent directories will not be created.',
        lateinit: True
        }),
#    ('use_cache', {
#        type: 'bool', set: 'parse_bool',
#        default: 'true',
#        help: "Enable/disable local mp3 cache.\nIf this option is disabled, the downloaded mp3 files won't be moved from `cache_dir`.\nInstead they will reside in `temp_dir` and will be deleted on application exit.",
#        }),
    ('album_dir', {
        type: 'std::string', set: 'Filesystem::expand',
        default: '"~/.config/ektoplayer/albums"',
        help: 'Where to unpack downloaded album archives to',
        lateinit: True
        }),
    ('archive_dir', {
        type: 'std::string', set: 'Filesystem::expand',
        default: '"~/.config/ektoplayer/archives"',
        help: 'Where to download album archives (zip/rar)',
        lateinit: True
        }),
    ('auto_extract_to_archive_dir', {
        type: 'bool', set: 'parse_bool',
        default: 'true',
        help: 'Enable/disable automatic extraction of downloaded MP3\narchives from `download_dir` to `archive_dir`',
        }),
    ('delete_after_extraction', {
        type: 'bool', set: 'parse_bool',
        default: 'true',
        help: 'In combination `with auto_extract_to_archive_dir`:\nDelete zip archive after successful extraction',
        }),
    ('playlist_load_newest', {
        type: 'int', set: 'parse_int',
        default: '1000',
        help: 'How many tracks from database should be added to the playlist on application start.',
        }),
    ('prefetch', {
        type: 'float', set: 'parse_float',
        default: '0.50',
        help: 'Specify after how many percent the next track shall be prefetched. Set it to 0 to disable it.',
        }),
    ('small_update_pages', {
        type: 'int', set: 'parse_int',
        default: '3',
        help: 'How many pages should be fetched after start',
        }),
    ('use_colors', {
        type: 'int', set: 'parse_use_colors',
        default: '"auto"',
        c_default: '-1',
        help: 'Choose color capabilities. auto|mono|8|256',
        }),
#   ('audio_system', {
#       type: 'std::string', set: 'std::string',
#       default: '"pulse,alsa,jack,oss"',
#       help: 'Set output audio system. See option `-o` in mpg123(1)',
#       }),
    ('playlist.columns', {
        type: 'PlaylistColumns', set: 'parse_playlist_columns',
        default: PLAYLIST_COLUMNS,
        c_default: PLAYLIST_COLUMNS_CVALUE,
        help: 'Columns of playlist',
        }),
    ('playlist.columns_256', {
        type: 'PlaylistColumns', set: 'parse_playlist_columns',
        default: PLAYLIST_COLUMNS_256,
        c_default: PLAYLIST_COLUMNS_256_CVALUE,
        help: 'Columns of playlist (256 colors)',
        }),
    ('browser.columns', {
        type: 'PlaylistColumns', set: 'parse_playlist_columns',
        default: PLAYLIST_COLUMNS,
        help: 'Columns of browser',
        lateinit: True
        }),
    ('browser.columns_256', {
        type: 'PlaylistColumns', set: 'parse_playlist_columns',
        default: PLAYLIST_COLUMNS_256,
        help: 'Columns of browser (256 colors)',
        lateinit: True
        }),
    ('progressbar.display', {
        type: 'bool', set: 'parse_bool',
        default: 'true',
        help: 'Enable/disable progressbar',
        }),
    ('progressbar.progress_char', {
        type: 'wchar_t', set: 'parse_char',
        default: "'~'",
        help: 'Character used for displaying playing progress',
        }),
    ('progressbar.rest_char', {
        type: 'wchar_t', set: 'parse_char',
        default: "'~'",
        help: 'Character used for the rest of the line',
        }),
    ('infoline.display', {
        type: 'bool', set: 'parse_bool',
        default: 'true',
        help: 'Enable/display infoline',
        }),
    ('infoline.format_top_mono', {
        type: 'InfoLineFormat', set: 'parse_infoline_format',
        default: INFOLINE_FORMAT_TOP_MONO,
        c_default: INFOLINE_FORMAT_TOP_MONO_CVALUE,
        help: 'Format of first line in infoline (mono colors)',
        }),
    ('infoline.format_bottom_mono', {
        type: 'InfoLineFormat', set: 'parse_infoline_format',
        default: INFOLINE_FORMAT_BOTTOM_MONO,
        c_default: INFOLINE_FORMAT_BOTTOM_MONO_CVALUE,
        help: 'Format of second line in infoline (mono colors)',
        }),
    ('infoline.format_top_8', {
        type: 'InfoLineFormat', set: 'parse_infoline_format',
        default: INFOLINE_FORMAT_TOP_8,
        c_default: INFOLINE_FORMAT_TOP_8_CVALUE,
        help: 'Format of first line in infoline (8 colors)',
        }),
    ('infoline.format_bottom_8', {
        type: 'InfoLineFormat', set: 'parse_infoline_format',
        default: INFOLINE_FORMAT_BOTTOM_8,
        c_default: INFOLINE_FORMAT_BOTTOM_8_CVALUE,
        help: 'Format of second line in infoline (8 colors)',
        }),
    ('infoline.format_top_256', {
        type: 'InfoLineFormat', set: 'parse_infoline_format',
        default: INFOLINE_FORMAT_TOP_256,
        c_default: INFOLINE_FORMAT_TOP_256_CVALUE,
        help: 'Format of first line in infoline (256 colors)',
        }),
    ('infoline.format_bottom_256', {
        type: 'InfoLineFormat', set: 'parse_infoline_format',
        default: INFOLINE_FORMAT_BOTTOM_256,
        c_default: INFOLINE_FORMAT_BOTTOM_256_CVALUE,
        help: 'Format of second line in infoline (256 colors)',
        }),
    ('tabbar.visible', {
        type: 'bool', set: 'parse_bool',
        default: 'true',
        help: 'Enable/disable tabbar visibility',
        }),
    ('infoline.visible', {
        type: 'bool', set: 'parse_bool',
        default: 'true',
        help: 'Enable/disable infoline visibility',
        }),
    ('progressbar.visible', {
        type: 'bool', set: 'parse_bool',
        default: 'true',
        help: 'Enable/disable progressbar visibility',
        }),
    ('tabs.widgets', {
        type: 'std::array<Views::TabWidgets, 5>', set: 'parse_tabs_widgets',
        default: '"splash,playlist,browser,info,help"',
        c_default: '{Views::TabWidgets::SPLASH,Views::TabWidgets::PLAYLIST,Views::TabWidgets::BROWSER,Views::TabWidgets::INFO,Views::TabWidgets::HELP}',
        help: 'Specify widget order of tabbar (left to right)',
        }),
    ('main.widgets', {
        type: 'std::array<Views::MainWidgets, 5>', set: 'parse_main_widgets',
        default: '"infoline,tabbar,readline,windows,progressbar"',
        c_default: '{Views::MainWidgets::INFOLINE,Views::MainWidgets::TABBAR,Views::MainWidgets::READLINE,Views::MainWidgets::WINDOWS,Views::MainWidgets::PROGRESSBAR}',
        help: 'Specify widgets to show (up to down)',
    }),
]

NAME = 0

# Configuration file ==========================================================
with open('ektoplayer.rc', 'w') as fh:
    #options.sort(key=lambda o: o[NAME])
    #options.sort(key=lambda o: len(o[NAME]))
    #options.sort(key=lambda o: o[1][type])
    #options.sort(key=lambda o: len(o[1][type]))

    for name, o in options:
        print('#', o[help].replace('\n', '\n# '), file=fh)
        print('set', name, o[default], file=fh)
        print(file=fh)

    print("# vim: filetype=sh", file=fh)

# CPP files ===================================================================
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
        print('case Hash::djb2(\"%s\"): %s = %s(value); break;' % (
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
