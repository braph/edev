DEBUG = 1 # 0|1

#CXXFLAGS   := -Og -g -Wall -Wpedantic -DDEBUG=$(DEBUG)
CXXFLAGS   := -O2 -Wall -Wpedantic -DDEBUG=$(DEBUG)
CPPFLAGS := $(shell xml2-config --cflags)
LDLIBS   := -lncursesw -lboost_system -lboost_filesystem -lpthread -lcurl $(shell xml2-config --libs)

clean:
	rm -f *.o
	rm -f views/*.o
	rm -f ui/*.o
	rm -f widgets/*.o

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

application: filesystem.o config.o shellsplit.o colors.o strpool.o database.o theme.o browsepage.o updater.o \
	ui/container.o views/splash.o views/playinginfo.o views/progressbar.o views/tabbar.o views/mainwindow.o \
	views/help.o player.o actions.o bindings.o 
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) application.cpp $^
	echo "Cannot test ncurses based stuff"

# ============================================================================
# Views
# ============================================================================

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

test_updater: browsepage.o database.o strpool.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_UPDATER updater.cpp $^
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

test_oddvector:
	$(CXX) -DTEST_ODDVECTOR $(CXXFLAGS) oddvector.cpp
	./a.out

# TODO
test_playlist: colors.o theme.o config.o filesystem.o shellsplit.o
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

