NAME = keymaster
# DEBUG = -DDEBUG -DDEBUG_STDERR

CPPFLAGS += -std=c++11 -MD -MP -g $(DEBUG)
LIBS = -lc -lc++ -lsqlite3 -lportmidi -lncurses

prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin

SRC = $(wildcard src/*.cpp) $(wildcard src/curses/*.cpp)
OBJS = $(SRC:%.cpp=%.o)
TEST_SRC = $(SRC) $(wildcard test/*.cpp)
TEST_OBJS = $(TEST_SRC:%.cpp=%.o)

CATCH_CATEGORY ?= ""

.PHONY: all test install uninstall tags clean distclean

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(LDFLAGS) $(LIBS) -o $@ $^

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
	$(CXX) $(LDFLAGS) $(LIBS) -o $@ $(filter-out src/error.o src/main.o,$^)

install:	$(bindir)/$(NAME)

$(bindir)/$(NAME):	$(NAME)
	cp ./$(NAME) $(bindir)
	chmod 755 $(bindir)/$(NAME)

uninstall:
	rm -f $(bindir)/$(name)

tags:	TAGS

TAGS:	$(SRC)
	etags $(SRC)

clean:
	rm -f $(NAME) $(NAME)_test src/*.o src/curses/*.o test/*.o

distclean: clean
	rm -f src/*.d src/curses/*.d test/*.d
