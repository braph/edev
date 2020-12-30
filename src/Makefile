WARNINGS =  -Wall -Wextra -Wpedantic -pedantic -Wmissing-declarations -Wredundant-decls
WARNINGS += -Winit-self -Woverloaded-virtual -Wctor-dtor-privacy -Wstrict-null-sentinel
WARNINGS += -Wundef -Wuninitialized
WARNINGS += -Wlogical-op
WARNINGS += -Wold-style-cast -Wcast-align -Wcast-qual -Wnoexcept -Wno-unused
WARNINGS += -Wsign-promo -Wsign-compare -Wsign-conversion
WARNINGS += -Wmissing-include-dirs
WARNINGS += -Wpessimizing-move -Wredundant-move
WARNINGS += -Wno-variadic-macros
#WARNINGS += -Werror # -Wfatal-errors
#WARNINGS += -Wdisabled-optimization -Wformat=2 -Wshadow -Wswitch-default -Wstrict-overflow=5 
#-D_GLIBCXX_ASSERTIONS

STD = c++11
CURSES_INC = "<ncurses.h>"

CXXFLAGS := -std=$(STD) -fno-rtti -O2 -s -DNDEBUG $(WARNINGS) -Wfatal-errors
#CXXFLAGS := -std=$(STD) -fno-rtti -O2 -pg -g $(WARNINGS)
#CXXFLAGS := -std=$(STD) -fno-rtti -O2 -g $(WARNINGS)
#CXXFLAGS := -std=$(STD) -fno-rtti -O2 -DNDEBUG $(WARNINGS) # -s -g
CPPFLAGS := $(shell xml2-config --cflags) -I/usr/include/readline -I.. -DCURSES_INC=$(CURSES_INC)
LDLIBS   := -lreadline -lncursesw -lboost_system -lboost_filesystem -lpthread -lcurl $(shell xml2-config --libs)

CONFIG.deps   	= ../lib/shellsplit.o ../lib/filesystem.o ../lib/xml.o
DATABASE.deps 	= ../lib/stringchunk.o ../lib/packedvector.o
BROWSEPAGE.deps = ../lib/base64.o
PLAYER.deps   	= ../lib/process.o
THEME.deps    	= ui/colors.o
UPDATER.deps  	= ../lib/downloads.o markdown.o
VIEWS         	= $(addprefix views/, splash.o infoline.o progressbar.o tabbar.o mainwindow.o help.o info.o playlist.o)
VIEWS         	+= widgets/listwidget.hpp widgets/readline.o

# Unity build
build:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDLIBS) build.cpp

application: config.o $(CONFIG.deps) database.o $(DATABASE.deps) theme.o $(THEME.deps) \
	browsepage.o $(BROWSEPAGE.deps) updater.o $(UPDATER.deps) $(VIEWS) ui/container.o \
	 player.o $(PLAYER.deps) actions.o bindings.o ../lib/downloads.o ektoplayer.o trackloader.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDLIBS) application.cpp $^

clean:
	rm -f {views,ui,widgets,lib,.}/*.o
	rm -f {views,ui,widgets,lib,.}/*.gch
	rm -f ../lib/*.o # TODO
	rm -f a.out

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

noexcept_objs = actions.o bindings.o ektoplayer.o theme.o markdown.o player.o ui/colors.o \
								$(addprefix ../lib/, downloads.o filesystem.o shellsplit.o stringchunk.o  process.o) \
								$(addprefix views/, splash.o infoline.o progressbar.o tabbar.o mainwindow.o help.o info.o playlist.o)
$(noexcept_objs): %.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -fno-exceptions -c $< -o $@

VALGRIND = valgrind --tool=memcheck --errors-for-leak-kinds=definite --leak-check=full --error-exitcode=1

# ============================================================================
# Tests....
# ============================================================================

TERMINAL = xterm -e # Used for running ncurses tests

tests: \
	test_ektoplayer \
	test_colors test_theme \
	test_updater test_database

# ============================================================================
# Core
# ============================================================================

#test_common:
#	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_COMMON common.cpp $^
#	$(VALGRIND) ./a.out

test_ektoplayer: ../lib/filesystem.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_EKTOPLAYER ektoplayer.cpp $^
	$(VALGRIND) ./a.out

test_colors:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_COLORS ui/colors.cpp $^
	$(VALGRIND) ./a.out

test_theme: $(THEME.deps)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_THEME theme.cpp $^
	$(VALGRIND) ./a.out

test_config: $(CONFIG.deps) ui/colors.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_CONFIG ektoplayer.cpp config.cpp $^
	$(VALGRIND) ./a.out

test_player:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_PLAYER player.cpp $^
	$(VALGRIND) ./a.out

#test_generic:
#	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_GENERIC generic.cpp $^
#	$(VALGRIND) ./a.out

test_browsepage: $(BROWSEPAGE.deps)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_BROWSEPAGE browsepage.cpp $^
	$(VALGRIND) ./a.out

test_updater: $(UPDATER.deps) database.o $(DATABASE.deps) $(BROWSEPAGE.deps) browsepage.o ektoplayer.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_UPDATER updater.cpp $^
	perf stat ./a.out

test_database: $(DATABASE.deps)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_DATABASE database.cpp $^
	perf stat ./a.out

test_trackloader: database.o $(DATABASE.deps) lib/downloads.o ektoplayer.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_TRACKLOADER trackloader.cpp $^
	$(VALGRIND) ./a.out

# ============================================================================
# Widgets
# ============================================================================

test_widgets: test_readline test_listwidget

test_readline:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_READLINE widgets/readline.cpp $^
	$(TERMINAL) ./a.out

test_listwidget: theme.o $(THEME.deps) config.o $(CONFIG.deps)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_LISTWIDGET widgets/listwidget.cpp $^
	$(TERMINAL) ./a.out

# ============================================================================
# Views
# ============================================================================

# test_help: dependency horror...
test_views: test_splash test_progressbar test_tabbar test_infoline test_playlist

test_splash: theme.o $(THEME.deps)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_SPLASH views/splash.cpp $^
	$(TERMINAL) ./a.out

test_help: theme.o $(THEME.deps) bindings.o actions.o player.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_HELP views/help.cpp $^
	$(TERMINAL) ./a.out

test_progressbar: config.o $(CONFIG.deps) theme.o $(THEME.deps)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_PROGRESSBAR views/progressbar.cpp $^
	$(TERMINAL) ./a.out

test_tabbar: config.o $(CONFIG.deps) theme.o $(THEME.deps)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_TABBAR views/tabbar.cpp $^
	$(TERMINAL) ./a.out

test_infoline: theme.o $(THEME.deps) config.o $(CONFIG.deps) database.o $(DATABASE.deps)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_INFOLINE views/infoline.cpp $^
	$(TERMINAL) ./a.out

test_playlist: theme.o $(THEME.deps) config.o $(CONFIG.deps) database.o $(DATABASE.deps)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_PLAYLIST views/playlist.cpp $^
	$(TERMINAL) ./a.out

