#
# Generic compiles-it-all Makefile
#  Loni Nix <lornix@lornix.com>
#
# adhere to a higher standard!
#CFLAGS+=-std=c99
#CFLAGS+=-std=c11
# pretty much always want debugging symbols included
CFLAGS+=-g3
#CFLAGS+=-gstabs3
# yell out all warnings and whatnot
CFLAGS+=-Wall -Wextra -Wunused
# make all warnings into errors
CFLAGS+=-Werror
# link time optimization! cool! smaller exec's!
#CFLAGS+=-flto
#LDFLAGS+=-flto
# optimize!
#CFLAGS+=-O3
# or not!
CFLAGS+=-O0
# enable for gmon performance statistics
#CFLAGS+=-pg
# preserve everything used to create binary, verbose assembly comments
#CFLAGS+=-masm=intel --save-temps -fverbose-asm
#
# das linker flags
# LDFLAGS+=
LDFLAGS+=-lm
# LDFLAGS+=-lbfd
#
CC:=gcc
#
.SUFFIXES:
.SUFFIXES: .c .cpp .o
#
SHELL=/bin/sh
#
.PHONY: all clean

all: emma

OBJS=main.o parsefile.o utils.o

emma: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

main.o: main.c emma.h

parsefile.o: parsefile.c parsefile.h emma.h

utils.o: utils.c utils.h emma.h

%.o : %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o $@ $<

% : %.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

clean:
	-rm emma *.o

