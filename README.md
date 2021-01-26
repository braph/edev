# ektoplayer - developement - super unstable

C++ rewrite of the existing ektoplayer (ruby).

Highlights:
- 256/8/Mono color support
- Mouse support
- Configurable keybindings
- Sane defaults (vi bindings / arrow + symbolic keys)
- Unicode!
- Runs in a single thread
- A very memory efficient database:
  - Only one instance per string is held in the "database"
  - Even substrings with same endings are merged into existing strings (like gcc's -fmerge-constants)
  - The database references strings by an enumerated ID instead of a pointer ...
  - ... those IDs are stored in a bitpacked vector ...
  - ... so each ID takes only about 17bits instead of a full sized 32/64bit integer ;D
  - The meta data itself is also stored in a optimized way:
    - URLs are stripped down to their path (https://ektoplazm.com/free-music/globular-entangled-everything -> globular-entangled-everything)
    - Timestamps are stored in a short form (counted since 2000 instead of 1970, HH:MM:SS stripped of)
    - HTML Description text is translated to a simple markdown format
    - Album genres are stored using bitflags

