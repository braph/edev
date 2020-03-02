# ektoplayer - developement - super unstable

- A very memory efficient database:
- String interning: Strings are only held exactly one by instance in a string buffer, strings with a common ending are also vanished
- Those strings are referenced by an offset instead of a charpointer, thus saving a full sized char pointer
- The IDs are stored in a bitpacked vector, so each ID takes only like 17bits instead of 32 :)
- The HTML Markup is translated into Markdown
- Result: Half of the database size compared to ektoplayer-ruby :)

