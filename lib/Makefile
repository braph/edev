WARNINGS =  -Wall -Wextra -Wpedantic -pedantic -Wmissing-declarations -Wredundant-decls
WARNINGS += -Winit-self -Woverloaded-virtual -Wctor-dtor-privacy -Wstrict-null-sentinel
WARNINGS += -Wundef -Wuninitialized
WARNINGS += -Wlogical-op
WARNINGS += -Wold-style-cast -Wcast-align -Wcast-qual -Wnoexcept -Wno-unused
WARNINGS += -Wsign-promo -Wsign-compare -Wsign-conversion
WARNINGS += -Wmissing-include-dirs
WARNINGS += -Wpessimizing-move -Wredundant-move
#WARNINGS += -Werror # -Wfatal-errors
#WARNINGS += -Wdisabled-optimization -Wformat=2 -Wshadow -Wswitch-default -Wstrict-overflow=5
#-D_GLIBCXX_ASSERTIONS

STD = c++11

CXXFLAGS := -std=$(STD) -fno-rtti -Og -g $(WARNINGS)
#CXXFLAGS := -std=$(STD) -fno-rtti -O3 -s -DNDEBUG $(WARNINGS)
CPPFLAGS := $(shell xml2-config --cflags) -I..
LDLIBS   := -lboost_system -lboost_filesystem -lpthread -lcurl $(shell xml2-config --libs)

clean:
	rm -f {lib,.}/*.o
	rm -f {lib,.}/*.gch
	rm -f a.out

VALGRIND = valgrind --tool=memcheck --errors-for-leak-kinds=definite --leak-check=full --error-exitcode=1

# ============================================================================
# Tests....
# ============================================================================

test_filesystem:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) lib/filesystem.cpp $^
	$(VALGRIND) ./a.out

test_packedvector: packedvector.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) tests/packedvector.cpp $^
	$(VALGRIND) ./a.out

test_shellsplit:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) lib/shellsplit.cpp $^
	$(VALGRIND) ./a.out

test_spanview:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) lib/spanview.cpp $^
	$(VALGRIND) ./a.out

test_sscan:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) tests/sscan.cpp $^
	$(VALGRIND) ./a.out

test_string:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) tests/string.cpp $^
	$(VALGRIND) ./a.out

test_stringchunk:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) lib/stringchunk.cpp $^
	$(VALGRIND) ./a.out

test_xml:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDLIBS) lib/xml.cpp $^
	$(VALGRIND) ./a.out

