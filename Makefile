DEBUG 				= 1 # 0|1
PEDANTIC_FREE = 1 # 0|1

WARNINGS = -Wall -Wpedantic -pedantic #-Wextra -Winit-self -Wold-style-cast -Woverloaded-virtual -Wuninitialized -Wmissing-declarations

CXXFLAGS   := -std=c++11 -Og -g $(WARNINGS) -pedantic -DDEBUG=$(DEBUG) -DPEDANTIC_FREE=$(PEDANTIC_FREE)
#CXXFLAGS   := -std=c++11 -O2 $(WARNINGS) -pedantic -DDEBUG=$(DEBUG) -DPEDANTIC_FREE=$(PEDANTIC_FREE)
CPPFLAGS := $(shell xml2-config --cflags) -I/usr/include/readline
LDLIBS   := -lreadline -lncursesw -lboost_system -lboost_filesystem -lpthread -lcurl $(shell xml2-config --libs)

application: filesystem.o config.o shellsplit.o colors.o strpool.o database.o theme.o browsepage.o updater.o \
	ui/container.o views/splash.o views/playinginfo.o views/progressbar.o views/tabbar.o views/mainwindow.o \
	views/help.o views/info.o views/playlist.o player.o actions.o bindings.o downloads.o ektoplayer.o trackloader.o common.o \
	packedvector.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) application.cpp $^
	echo "Cannot test ncurses based stuff"

clean:
	rm -f *.o
	rm -f views/*.o
	rm -f ui/*.o
	rm -f widgets/*.o

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

# ============================================================================
# Views
# ============================================================================

test_readline:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_READLINE widgets/readline.cpp $^
	echo "cannot test here"

test_splash: colors.o theme.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_SPLASH views/splash.cpp $^
	echo "Cannot test ncurses based stuff"

test_help: colors.o theme.o bindings.o actions.o player.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_HELP views/help.cpp $^
	echo "Cannot test ncurses based stuff"

test_progressbar: config.o shellsplit.o filesystem.o colors.o theme.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_PROGRESSBAR views/progressbar.cpp $^
	echo "Widgets cannot be tested in Make"

test_database: strpool.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_DATABASE database.cpp $^
	./a.out

test_updater: browsepage.o database.o strpool.o downloads.o ektoplayer.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_UPDATER updater.cpp $^
	perf stat ./a.out

test_trackloader: database.o strpool.o downloads.o ektoplayer.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_TRACKLOADER trackloader.cpp $^
	perf stat ./a.out

test_tabbar: config.o theme.o filesystem.o colors.o shellsplit.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_TABBAR views/tabbar.cpp $^

test_strpool:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_STRPOOL strpool.cpp
	./a.out

test_common:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_COMMON common.cpp
	./a.out

test_filesystem:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_FILESYSTEM filesystem.cpp
	./a.out

test_config: shellsplit.o colors.o xml.o filesystem.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_CONFIG ektoplayer.cpp config.cpp $^
	./a.out

test_colors:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_COLORS colors.cpp
	./a.out

test_shellsplit:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_SHELLSPLIT shellsplit.cpp
	./a.out

test_theme: colors.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_THEME theme.cpp $^
	./a.out

test_playinginfo: colors.o theme.o config.o filesystem.o shellsplit.o database.o strpool.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_PLAYINGINFO views/playinginfo.cpp $^
	echo "Widgets cannot be tested in Make"

test: test_filesystem test_ektoplayer test_config
	echo foo

test_packedvector:
	$(CXX) -DTEST_PACKEDVECTOR $(CXXFLAGS) packedvector.cpp $^
	./a.out

test_playlist: colors.o theme.o config.o filesystem.o shellsplit.o database.o strpool.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_PLAYLIST views/playlist.cpp $^
	echo "Widgets cannot be tested in Make"

test_listwidget: colors.o theme.o config.o filesystem.o shellsplit.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_LISTWIDGET widgets/listwidget.cpp $^
	echo "Widgets cannot be tested in Make"

test_player:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_PLAYER player.cpp
	./a.out

test_ektoplayer:
	$(CXX) -DTEST_EKTOPLAYER ektoplayer.cpp filesystem.cpp
	./a.out

test_colorfader:
	$(CXX) -DTEST_COLORFADER colorfader.cpp
	./a.out

test_ui:
	$(CXX) -DTEST_UI ui.cpp
	./a.out

test_xml:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_XML xml.cpp
	./a.out

test_browsepage:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_BROWSEPAGE browsepage.cpp
	valgrind ./a.out

