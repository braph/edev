#include <string>
#include <cstdio>

namespace Ektoplayer {
  class Application {
    static void log(const char *from, const char *message) {
      time_t t = time(NULL);
      struct tm tm;
      char timestamp[32];
      localtime_r(&t, &tm);
      strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm);
      fprintf(stderr, "%s %s: %s", timestamp, from, message);
      // TODO: backtrace on exception, exception message
      fflush(stderr);
    }

    static void open_log(const char *file) {
      freopen(file, "r", stderr);
      // TODO: stderr.sync=true
    }

    static void run() {
      fputs("\033]0;ektoplayer\007", stdout);

      if (Filesystem::exists(X))
        Config.parse(X, Bindings, Theme); // XXX: catch and fail "Config: "

       FileUtils::mkdir_p Config::CONFIG_DIR rescue (
          fail "Could not create config dir: #{$!}"
       )

       Application.open_log(Config[:log_file]);

         if Config[:use_cache]
            unless File.directory? Config[:cache_dir]
               FileUtils::mkdir Config[:cache_dir] rescue (
                  fail "Could not create cache dir: #{$!}"
               )
            end
         end

         [:temp_dir, :download_dir, :archive_dir].each do |key|
            unless File.directory? Config[key]
               FileUtils::mkdir Config[key] rescue (
                  fail "Could not create #{key}: #{$!}"
               )
            end
         end

#if 0 // continue here
         UI::Canvas.run do
            Application.log(self, "using '#{$USING_CURSES}' with #{ICurses.colors} colors available")

            if Config[:use_colors] == :auto
               Theme.use_colors(ICurses.colors >= 256 ? 256 : 8)
            else
               Theme.use_colors(Config[:use_colors])
            end

            client = Client.new
#endif
    }
  };
}
