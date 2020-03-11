WARNINGS =  -Wall -Wextra -Wpedantic -pedantic -Wmissing-declarations -Wredundant-decls
WARNINGS += -Winit-self -Woverloaded-virtual -Wctor-dtor-privacy -Wstrict-null-sentinel
WARNINGS += -Wundef -Wuninitialized
WARNINGS += -Wlogical-op
WARNINGS += -Wold-style-cast -Wcast-align -Wcast-qual -Wnoexcept -Wno-unused
WARNINGS += -Wsign-promo -Wsign-compare -Wsign-conversion
WARNINGS += -Wmissing-include-dirs
#WARNINGS += -Werror # -Wfatal-errors
#WARNINGS += -Wdisabled-optimization -Wformat=2 -Wshadow -Wswitch-default -Wstrict-overflow=5 
#-D_GLIBCXX_ASSERTIONS

STD = c++11
CURSES_INC = "<ncurses.h>"

#CXXFLAGS := -std=$(STD) -fno-rtti -Og -g $(WARNINGS)
CXXFLAGS := -std=$(STD) -fno-rtti -O2 -DNDEBUG $(WARNINGS)
CPPFLAGS := $(shell xml2-config --cflags) -I/usr/include/readline -DCURSES_INC=$(CURSES_INC) 
LDLIBS   := -lreadline -lncursesw -lboost_system -lboost_filesystem -lpthread -lcurl $(shell xml2-config --libs)

CONFIG.deps   = shellsplit.o filesystem.o common.o xml.o
DATABASE.deps = stringpool.o packedvector.o common.o generic.hpp
THEME.deps    = colors.o
PLAYER.deps   = process.o
VIEWS         = $(addprefix views/, splash.o playinginfo.o progressbar.o tabbar.o mainwindow.o help.o info.o playlist.o)
VIEWS         += widgets/listwidget.hpp widgets/readline.o

application: config.o $(CONFIG.deps) database.o $(DATABASE.deps) theme.o $(THEME.deps) \
	browsepage.o updater.o $(VIEWS) ui/container.o \
	 player.o $(PLAYER.deps) actions.o bindings.o downloads.o ektoplayer.o trackloader.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDLIBS) application.cpp $^

clean:
	rm -f *.o
	rm -f views/*.o
	rm -f ui/*.o
	rm -f widgets/*.o

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

noexcept_objs = stringpool.o player.o process.o bindings.o ektoplayer.o filesystem.o \
								$(addprefix views/, splash.o )
$(noexcept_objs): %.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -fno-exceptions -c $< -o $@

# ============================================================================
# Views
# ============================================================================

test_readline:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_READLINE widgets/readline.cpp $^
	echo "cannot test here"

test_splash: theme.o $(THEME.deps)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_SPLASH views/splash.cpp $^
	echo "Cannot test ncurses based stuff"

test_help: theme.o $(THEME.deps) bindings.o actions.o player.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_HELP views/help.cpp $^
	echo "Cannot test ncurses based stuff"

test_progressbar: config.o $(CONFIG.deps) theme.o $(THEME.deps)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_PROGRESSBAR views/progressbar.cpp $^
	echo "Widgets cannot be tested in Make"

test_database: $(DATABASE.deps)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_DATABASE database.cpp $^
	./a.out

test_updater: database.o $(DATABASE.deps) browsepage.o downloads.o ektoplayer.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_UPDATER updater.cpp $^
	perf stat ./a.out

test_trackloader: database.o $(DATABASE.deps) downloads.o ektoplayer.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_TRACKLOADER trackloader.cpp $^
	perf stat ./a.out

test_tabbar: config.o $(CONFIG.deps) theme.o $(THEME.deps)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_TABBAR views/tabbar.cpp $^

test_stringpool:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_STRINGPOOL stringpool.cpp
	./a.out

test_common:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_COMMON common.cpp
	./a.out

test_filesystem:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_FILESYSTEM filesystem.cpp
	./a.out

test_config: $(CONFIG.deps) colors.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_CONFIG ektoplayer.cpp config.cpp $^
	./a.out

test_colors:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_COLORS colors.cpp
	./a.out

test_shellsplit:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_SHELLSPLIT shellsplit.cpp
	./a.out

test_theme: $(THEME.deps)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_THEME theme.cpp $^
	./a.out

test_playinginfo: theme.o $(THEME.deps) config.o $(CONFIG.deps) database.o $(DATABASE.deps)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_PLAYINGINFO views/playinginfo.cpp $^
	echo "Widgets cannot be tested in Make"

test_packedvector:
	$(CXX) -DTEST_PACKEDVECTOR $(CXXFLAGS) packedvector.cpp $^
	./a.out

test_playlist: theme.o $(THEME.deps) config.o $(CONFIG.deps) database.o $(DATABASE.deps)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_PLAYLIST views/playlist.cpp $^
	echo "Widgets cannot be tested in Make"

test_listwidget: theme.o $(THEME.deps) config.o $(CONFIG.deps)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_LISTWIDGET widgets/listwidget.cpp $^
	echo "Widgets cannot be tested in Make"

test_player:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) -DTEST_PLAYER player.cpp
	./a.out

test_ektoplayer:
	$(CXX) -DTEST_EKTOPLAYER ektoplayer.cpp filesystem.cpp
	./a.out

test_generic:
	$(CXX) -DTEST_GENERIC generic.cpp
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
