/* Unity build */

#if 0
//#include <cstdlib>
//#include <bits/functexcept.h>
#define __throw_bad_exception(...)         abort()
#define __throw_bad_alloc(...)             abort()
#define __throw_bad_cast(...)              abort()
#define __throw_bad_typeid(...)            abort()
#define __throw_logic_error(...)           abort()
#define __throw_domain_error(...)          abort()
#define __throw_invalid_argument(...)      abort()
#define __throw_length_error(...)          abort()
#define __throw_out_of_range(...)          abort()
#define __throw_out_of_range_fmt(...)      abort()
#define __throw_runtime_error(...)         abort()
#define __throw_range_error(...)           abort()
#define __throw_overflow_error(...)        abort()
#define __throw_underflow_error(...)       abort()
#define __throw_ios_failure(...)           abort()
#define __throw_system_error(...)          abort()
#define __throw_future_error(...)          abort()
#define __throw_bad_function_call(...)     abort()
#endif

#define __cpp_exceptions 0
#include<string>
#include<vector>
#include<memory>

#include "../lib/base64.cpp"
#include "../lib/downloads.cpp"
#include "../lib/filesystem.cpp"
#include "../lib/shellsplit.cpp"
#include "../lib/stringchunk.cpp"
#include "../lib/process.cpp"
#include "../lib/xml.cpp"
#define __cpp_exceptions 200202

#include "ui/colors.cpp"
#include "ui/container.cpp"
#include "widgets/readline.cpp"

#include "ektoplayer.cpp"
#include "mpg123playback.cpp"
#include "markdown.cpp"
#include "database.cpp"
#include "config.cpp"
#include "theme.cpp"
#include "trackloader.cpp"
#include "browsepage.cpp"
#include "updater.cpp"
#include "bindings.cpp"
#include "actions.cpp"

#include "views/splash.cpp"
#include "views/infoline.cpp"
#include "views/progressbar.cpp"
#include "views/tabbar.cpp"
#include "views/mainwindow.cpp"
#include "views/help.cpp"
#include "views/info.cpp"
#include "views/playlist.cpp"
#include "views/browser.cpp"

#include "application.cpp"
