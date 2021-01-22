int                         Config :: use_colors = -1;
int                         Config :: small_update_pages = 3;
int                         Config :: playlist_load_newest = 1000;
bool                        Config :: tabbar_visible = true;
bool                        Config :: infoline_display = true;
bool                        Config :: infoline_visible = true;
bool                        Config :: progressbar_display = true;
bool                        Config :: progressbar_visible = true;
bool                        Config :: delete_after_extraction = true;
bool                        Config :: auto_extract_to_archive_dir = true;
float                       Config :: prefetch = 0.50;
wchar_t                     Config :: progressbar_rest_char = '~';
wchar_t                     Config :: progressbar_progress_char = '~';
std::string                 Config :: log_file /* will be initialized later */;
std::string                 Config :: album_dir /* will be initialized later */;
std::string                 Config :: cache_dir /* will be initialized later */;
std::string                 Config :: archive_dir /* will be initialized later */;
std::string                 Config :: database_file /* will be initialized later */;
InfoLineFormat              Config :: infoline_format_top_8 = {
{static_cast<Database::ColumnID>(Database::COLUMN_NONE), "<< ", COLOR_BLACK},
{static_cast<Database::ColumnID>(Database::TRACK_TITLE), "",    COLOR_YELLOW, -1, A_BOLD},
{static_cast<Database::ColumnID>(Database::COLUMN_NONE), " >>", COLOR_BLACK}};
InfoLineFormat              Config :: infoline_format_top_256 = {
{static_cast<Database::ColumnID>(Database::COLUMN_NONE), "<< ", 236},
{static_cast<Database::ColumnID>(Database::TRACK_TITLE), "",    178, -1, A_BOLD},
{static_cast<Database::ColumnID>(Database::COLUMN_NONE), " >>", 236}};
InfoLineFormat              Config :: infoline_format_bottom_8 = {
{static_cast<Database::ColumnID>(Database::TRACK_ARTIST),  "",     COLOR_BLUE, -1, A_BOLD},
{static_cast<Database::ColumnID>(Database::COLUMN_NONE),   " - "},
{static_cast<Database::ColumnID>(Database::ALBUM_TITLE),   "",     COLOR_RED, -1, A_BOLD},
{static_cast<Database::ColumnID>(Database::COLUMN_NONE),   " ("},
{static_cast<Database::ColumnID>(Database::ALBUM_YEAR),    "",     COLOR_CYAN, -1, 0},
{static_cast<Database::ColumnID>(Database::COLUMN_NONE),   ")"}
};
InfoLineFormat              Config :: infoline_format_top_mono = {
{static_cast<Database::ColumnID>(Database::COLUMN_NONE), "<< "},
{static_cast<Database::ColumnID>(Database::TRACK_TITLE), "",  -1, -1, A_BOLD},
{static_cast<Database::ColumnID>(Database::COLUMN_NONE), " >>"}};
InfoLineFormat              Config :: infoline_format_bottom_256 = {
{static_cast<Database::ColumnID>(Database::TRACK_ARTIST),  "",     24, -1, A_BOLD},
{static_cast<Database::ColumnID>(Database::COLUMN_NONE),   " - "},
{static_cast<Database::ColumnID>(Database::ALBUM_TITLE),   "",     160, -1, A_BOLD},
{static_cast<Database::ColumnID>(Database::COLUMN_NONE),   " ("},
{static_cast<Database::ColumnID>(Database::ALBUM_YEAR),    "",     37, -1, 0},
{static_cast<Database::ColumnID>(Database::COLUMN_NONE),   ")"}
};
InfoLineFormat              Config :: infoline_format_bottom_mono = {
{static_cast<Database::ColumnID>(Database::TRACK_ARTIST),  "",     -1, -1, A_BOLD},
{static_cast<Database::ColumnID>(Database::COLUMN_NONE),   " - "},
{static_cast<Database::ColumnID>(Database::ALBUM_TITLE),   "",     -1, -1, A_BOLD},
{static_cast<Database::ColumnID>(Database::COLUMN_NONE),   " ("},
{static_cast<Database::ColumnID>(Database::ALBUM_YEAR),    "",     -1, -1, 0},
{static_cast<Database::ColumnID>(Database::COLUMN_NONE),   ")"}
};
PlaylistColumns             Config :: browser_columns /* will be initialized later */;
PlaylistColumns             Config :: playlist_columns = {
{static_cast<Database::ColumnID>(Database::TRACK_NUMBER),  COLOR_MAGENTA,  -1, 3},
{static_cast<Database::ColumnID>(Database::TRACK_ARTIST),  COLOR_BLUE,     -1, 25, true},
{static_cast<Database::ColumnID>(Database::ALBUM_TITLE),   COLOR_RED,      -1, 30, true},
{static_cast<Database::ColumnID>(Database::TRACK_TITLE),   COLOR_YELLOW,   -1, 33, true},
{static_cast<Database::ColumnID>(Database::ALBUM_STYLES),  COLOR_CYAN,     -1, 20, true},
{static_cast<Database::ColumnID>(Database::TRACK_BPM),     COLOR_GREEN,    -1, 3}};
PlaylistColumns             Config :: browser_columns_256 /* will be initialized later */;
PlaylistColumns             Config :: playlist_columns_256 = {
{static_cast<Database::ColumnID>(Database::TRACK_NUMBER),  97,   -1, 3},
{static_cast<Database::ColumnID>(Database::TRACK_ARTIST),  24,   -1, 25, true},
{static_cast<Database::ColumnID>(Database::ALBUM_TITLE),   160,  -1, 30, true},
{static_cast<Database::ColumnID>(Database::TRACK_TITLE),   178,  -1, 33, true},
{static_cast<Database::ColumnID>(Database::ALBUM_STYLES),  37,   -1, 20, true},
{static_cast<Database::ColumnID>(Database::TRACK_BPM),     28,   -1, 3}};
packed::TinyArray<Views::TabWidgets, uint32_t, 3> Config :: tabs_widgets = {Views::TabWidgets::SPLASH,Views::TabWidgets::PLAYLIST,Views::TabWidgets::BROWSER,Views::TabWidgets::INFO,Views::TabWidgets::HELP};
packed::TinyArray<Views::MainWidgets, uint32_t, 3> Config :: main_widgets = {Views::MainWidgets::INFOLINE,Views::MainWidgets::TABBAR,Views::MainWidgets::READLINE,Views::MainWidgets::WINDOWS,Views::MainWidgets::PROGRESSBAR};
