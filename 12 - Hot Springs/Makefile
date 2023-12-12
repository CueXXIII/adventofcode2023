SRC=fsc.cc fsc5.cc fsc6.cc fsc7.cc

CPPFLAGS=-I../common
CXXFLAGS=-std=c++23 -O3 -march=native -flto=auto -ffat-lto-objects -Wall -Wextra -Wpedantic -Wconversion -Wshadow=local  -g3 -ggdb3
CXXFLAGS+=-fsanitize=undefined  # remove for final crunch

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
