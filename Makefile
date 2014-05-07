#
# Generic compiles-it-all Makefile
#  Loni Nix <lornix@lornix.com>
#
# adhere to a higher standard!
#CXXFLAGS+=-std=c99
#CXXFLAGS+=-std=c11
# pretty much always want debugging symbols included
CXXFLAGS+=-g3
#CXXFLAGS+=-gstabs3
# yell out all warnings and whatnot
CXXFLAGS+=-Wall -Wextra -Wunused
# make all warnings into errors
CXXFLAGS+=-Werror
# link time optimization! cool! smaller exec's!
#CXXFLAGS+=-flto
#LDFLAGS+=-flto
# optimize!
#CXXFLAGS+=-O3
# or not!
CXXFLAGS+=-O0
# enable for gmon performance statistics
#CXXFLAGS+=-pg
# preserve everything used to create binary, verbose assembly comments
#CXXFLAGS+=-masm=intel --save-temps -fverbose-asm
#
# das linker flags
# LDFLAGS+=
# LDFLAGS+=-lm
LDFLAGS+=-lbfd
#
CXX:=g++
#
.SUFFIXES:
.SUFFIXES: .cpp .h .o
#
SHELL=/bin/sh
#
# build VERREV macro
VERSION=$(shell cat VERSION)
REVISION=$(shell git rev-parse --short HEAD)
#
CXXFLAGS+=-D'VERREV="$(VERSION)-$(REVISION)"'
#
.PHONY: all clean

all: emma

OBJS=main.o parsefile.o utils.o

emma: $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

main.o: main.cpp emma.h

parsefile.o: parsefile.cpp parsefile.h emma.h

utils.o: utils.cpp utils.h emma.h

%.o : %.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -c -o $@ $<

clean:
	-rm emma $(OBJS)
