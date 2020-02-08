XML    := $(shell xml2-config --cflags --libs)
CURSES := -lncursesw
CXXFLAGS   := -g -Wall
#CXXFLAGS   := -O2 -Wall
CPPFLAGS := $(shell xml2-config --cflags)
LDLIBS   := -lncursesw -lboost_system -lboost_filesystem -lpthread -lcurl $(shell xml2-config --libs)

clean:
	rm *.o

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $<

application: filesystem.o config.o shellsplit.o colors.o strpool.o database.o theme.o browsepage.o updater.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) application.cpp $^
	echo "Cannot test ncurses based stuff"

# ============================================================================
# Views
# ============================================================================
test_splash: colors.o theme.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_SPLASH views/splash.cpp $^
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

test_theme:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_THEME theme.cpp colors.cpp
	./a.out


test: test_filesystem test_ektoplayer test_config
	echo foo

test_oddvector:
	$(CXX) -DTEST_ODDVECTOR $(CXXFLAGS) oddvector.cpp
	./a.out

test_playlist:
	$(CXX) -DTEST_PLAYLIST $(CXXFLAGS) $(CURSES) $(XML) views/playlist.cpp colors.cpp theme.cpp config.cpp colorfader.cpp filesystem.cpp shellsplit.cpp
	echo "Widgets cannot be tested in Make"

test_listwidget:
	$(CXX) -DTEST_LISTWIDGET $(CXXFLAGS) $(CURSES) $(XML) widgets/listwidget.hpp colors.cpp theme.cpp config.cpp colorfader.cpp filesystem.cpp shellsplit.cpp
	echo "Widgets cannot be tested in Make"

test_playinginfo:
	$(CXX) -DTEST_PLAYINGINFO $(CXXFLAGS) $(CURSES) $(XML) views/playinginfo.cpp colors.cpp theme.cpp config.cpp filesystem.cpp shellsplit.cpp
	echo "Widgets cannot be tested in Make"

test_player:
	$(CXX) -DTEST_PLAYER $(BOOST) player.cpp
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

