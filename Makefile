XML    := $(shell xml2-config --cflags --libs)
CURSES := -lncursesw
BOOST  := -lboost_system -lboost_filesystem -lpthread
CURL   := -lcurl
OPTS   := -g -Wall
#OPTS   := -O2 -Wall

test: test_filesystem test_ektoplayer test_config
	echo foo

test_oddvector:
	g++ -DTEST_ODDVECTOR $(OPTS) oddvector.cpp
	./a.out

test_strpool:
	g++ -DTEST_STRPOOL $(OPTS) strpool.cpp
	./a.out

test_playlist:
	g++ -DTEST_PLAYLIST $(OPTS) $(CURSES) $(XML) views/playlist.cpp colors.cpp theme.cpp config.cpp colorfader.cpp filesystem.cpp shellsplit.cpp
	echo "Widgets cannot be tested in Make"

test_listwidget:
	g++ -DTEST_LISTWIDGET $(OPTS) $(CURSES) $(XML) widgets/listwidget.hpp colors.cpp theme.cpp config.cpp colorfader.cpp filesystem.cpp shellsplit.cpp
	echo "Widgets cannot be tested in Make"

test_progressbar:
	g++ -DTEST_PROGRESSBAR $(OPTS) $(CURSES) $(XML) views/progressbar.cpp colors.cpp theme.cpp config.cpp colorfader.cpp filesystem.cpp shellsplit.cpp
	echo "Widgets cannot be tested in Make"

test_playinginfo:
	g++ -DTEST_PLAYINGINFO $(OPTS) $(CURSES) $(XML) views/playinginfo.cpp colors.cpp theme.cpp config.cpp filesystem.cpp shellsplit.cpp
	echo "Widgets cannot be tested in Make"

test_common:
	g++ -DTEST_COMMON common.cpp
	./a.out

test_updater:
	g++ -DTEST_UPDATER $(OPTS) $(BOOST) $(CURL) $(XML) updater.cpp browsepage.cpp database.cpp strpool.cpp
	perf stat ./a.out

test_player:
	g++ -DTEST_PLAYER $(BOOST) player.cpp
	./a.out

test_filesystem:
	g++ -DTEST_FILESYSTEM filesystem.cpp
	./a.out

test_database:
	g++ -DTEST_DATABASE database.cpp
	./a.out

test_ektoplayer:
	g++ -DTEST_EKTOPLAYER ektoplayer.cpp filesystem.cpp
	./a.out

test_config:
	g++ -DTEST_CONFIG $(OPTS) $(XML) $(CURSES) ektoplayer.cpp filesystem.cpp config.cpp xml.cpp colors.cpp shellsplit.cpp
	./a.out

test_colors:
	g++ -DTEST_COLORS colors.cpp -lncursesw
	./a.out

test_shellsplit:
	g++ -DTEST_SHELLSPLIT shellsplit.cpp
	./a.out

test_colorfader:
	g++ -DTEST_COLORFADER colorfader.cpp
	./a.out

test_theme:
	g++ -DTEST_THEME theme.cpp colors.cpp -lncursesw
	./a.out

test_splash:
	g++ -DTEST_SPLASH views/splash.cpp colorfader.cpp colors.cpp theme.cpp -lncursesw
	echo "Cannot test ncurses based stuff"

test_ui:
	g++ -DTEST_UI ui.cpp
	./a.out

test_application:
	g++ $(OPTS) $(XML) $(CURSES) $(CURL) $(BOOST) \
		filesystem.cpp application.cpp config.cpp shellsplit.cpp \
		colors.cpp theme.cpp database.cpp updater.cpp \
		strpool.cpp browsepage.cpp
	./a.out

#test_options:
#	g++ -DTEST_OPTIONS `xml2-config --cflags --libs` -lncursesw options.cpp colors.cpp
#	./a.out

test_xml:
	g++ -DTEST_XML $(XML) $(CURSES) $(BOOST) xml.cpp
	./a.out

test_browsepage:
	g++ -DTEST_BROWSEPAGE $(XML) $(BOOST) $(CURL) $(OPTS) browsepage.cpp
	valgrind ./a.out

