SRC=

CPPFLAGS=-I../common
CXXFLAGS=-std=c++23 -O3 -flto=auto -Wall -Wextra -Wpedantic -Wconversion -Wshadow=local  -g3 -ggdb3

# tbb: enable more parallel execution for stdlib
# fmt: provides ::format and ::print in NS fmt, not yet in std
LDLIBS=-ltbb -lfmt

OBJ=$(SRC:.cc=.o)
LINK.o=$(LINK.cc)
TARGET=$(SRC:.cc=)

all: $(TARGET)

clean:
	rm -f $(OBJ) $(TARGET)

Makefile.deps: $(SRC) Makefile
	$(CXX) $(CPPFLAGS) -MM $(SRC) >$@

include Makefile.deps
