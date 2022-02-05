NAME = keymaster
# DEBUG = -DDEBUG -DDEBUG_STDERR

WXFLAGS := $(shell wx-config --cxxflags)
WXLIBS := $(shell wx-config --libs)

CPP = $(shell wx-config --cxx)
CPPFLAGS += -std=c++11 -MD -MP -g $(DEBUG) $(WXFLAGS)

LD = $(shell wx-config --ld)
LIBS = -lc -lc++ -lsqlite3 -lportmidi

prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin

SRC = $(wildcard src/*.cpp) $(wildcard src/wx/*.cpp)
OBJS = $(SRC:%.cpp=%.o)
TEST_SRC = $(wildcard src/*.cpp) $(wildcard test/*.cpp)
TEST_OBJS = $(TEST_SRC:%.cpp=%.o)

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

src/curves.cpp: src/generated_curves.h

src/generated_curves.h:	bin/generate_curves.py
	bin/generate_curves.py > $@

test: $(NAME)_test
	./$(NAME)_test --use-colour no $(CATCH_CATEGORY)

$(NAME)_test:	$(TEST_OBJS)
	$(CXX) $(LDFLAGS) $(LIBS) -o $@ $(filter-out src/error.o,$^)

install:	$(bindir)/$(NAME)
	install ./$(NAME) $(bindir)

uninstall:
	rm -f $(bindir)/$(NAME)

tags:	TAGS

TAGS:	$(SRC)
	etags $(SRC)

clean:
	rm -f $(NAME) $(NAME)_test src/*.o src/wx/*.o test/*.o

distclean: clean
	rm -f src/*.d src/wx/*.d test/*.d
