NAME = keymaster
# DEBUG = -DDEBUG -DDEBUG_STDERR

WXFLAGS := $(shell wx-config --cxxflags)
WXLIBS := $(shell wx-config --libs)

CPP = $(shell wx-config --cxx)
CPPFLAGS += -std=c++14 -MD -MP -g $(DEBUG) $(WXFLAGS)

LD = $(shell wx-config --ld)
LIBS = -lsqlite3 -lportmidi

prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin

SRC = $(wildcard src/*.cpp) $(wildcard src/wx/*.cpp)
OBJS = $(SRC:%.cpp=%.o)
TEST_SRC = $(wildcard src/*.cpp) $(wildcard test/*.cpp)
TEST_OBJS = $(TEST_SRC:%.cpp=%.o)
TEST_LIBS = $(LIBS) -lCatch2Main -lCatch2

CATCH_FILE = /opt/homebrew/include/catch2/catch_all.hpp
CATCH_CATEGORY ?= ""

.PHONY: all test install uninstall tags clean distclean

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(LDFLAGS) $(LIBS) $(WXLIBS) -o $@ $^

-include $(SRC:%.cpp=%.d)
-include $(TEST_SRC:%.cpp=%.d)

storage.cpp:	src/schema.sql.h

# Turn db/schema.sql into a C++11 header file that defines a string
# containing the SQL.
src/schema.sql.h: db/schema.sql
	@echo "// THIS FILE IS GENERATED FROM $<" > $@ \
	&& echo 'static const char * const SCHEMA_SQL = R"(' >> $@ \
	&& cat $< >> $@ \
	&& echo ')";' >> $@

test: $(CATCH_FILE) $(NAME)_test
	./$(NAME)_test --colour-mode=none $(CATCH_CATEGORY)

$(CATCH_FILE):
	brew install catch2

$(NAME)_test:	$(TEST_OBJS)
	$(CXX) $(LDFLAGS) $(TEST_LIBS) -o $@ $(filter-out src/error.o,$^)

install:	$(NAME)
	install -s ./$(NAME) $(bindir)

uninstall:
	rm -f $(bindir)/$(NAME)

tags:	TAGS

TAGS:	$(SRC)
	etags $(SRC)

clean:
	rm -f $(NAME) $(NAME)_test src/*.o src/wx/*.o test/*.o

distclean: clean
	rm -f src/*.d src/wx/*.d test/*.d
