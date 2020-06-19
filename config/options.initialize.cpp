log_file = Filesystem::expand("~/.config/ektoplayer/ektoplayer.log");
cache_dir = Filesystem::expand("~/.cache/ektoplayer");
archive_dir = Filesystem::expand("~/.config/ektoplayer/archives");
database_file = Filesystem::expand("~/.config/ektoplayer/meta.db");
browser_columns = opt_parse_playlist_columns("number{fg=magenta size=3} artist{fg=blue size=25%} album{fg=red size=30%} title {fg=yellow size=33%} styles{fg=cyan size=20%} bpm{fg=green size=3 right}");
browser_columns_256 = opt_parse_playlist_columns("number{fg=97 size=3} artist{fg=24 size=25%} album{fg=160 size=30%} title {fg=178 size=33%} styles{fg=37 size=20%} bpm{fg=28 size=3 right}");
