#
# Generic compiles-it-all Makefile
#  Loni Nix <lornix@lornix.com>
#
# adhere to a higher standard!
CFLAGS+=-std=c99
#CFLAGS+=-std=c11
# pretty much always want debugging symbols included
CFLAGS+=-g3
#CFLAGS+=-gstabs3
# yell out all warnings and whatnot
CFLAGS+=-Wall -Wextra -Wunused
# make all warnings into errors
#CFLAGS+=-Werror
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
# LDFLAGS+=-lm
LDFLAGS+=-lbfd
#
CC:=gcc
#
.SUFFIXES:
.SUFFIXES: .c .h .o
#
SHELL=/bin/sh
#
# build VERREV macro
VERSION=$(shell cat VERSION)
REVISION=$(shell git rev-parse --short HEAD)
VERREV=$(VERSION)-$(REVISION)
#
CFLAGS+=-D'VERREV="$(VERREV)"'
#
.PHONY: all clean kcov kcov-clean kcov-show

all: emma

OBJS=main.o parsefile.o

emma: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

main.o: main.c emma.h

parsefile.o: parsefile.c parsefile.h emma.h

%.o : %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o $@ $<

clean:
	rm -f emma $(OBJS)

kcov: all
	@rm -f /tmp/empty-file-$(VERSION) /tmp/unreadable-file-$(VERSION) /tmp/non-existent-file-$(VERSION)
	@touch /tmp/empty-file-$(VERSION) /tmp/unreadable-file-$(VERSION)
	@chmod 000 /tmp/unreadable-file-$(VERSION)
	-kcov --skip-solibs kcov/ ./emma emma $(OBJS) /tmp/empty-file-$(VERSION) /tmp/unreadable-file-$(VERSION) /tmp/non-existent-file-$(VERSION)
	@rm -f /tmp/empty-file-$(VERSION) /tmp/unreadable-file-$(VERSION) /tmp/non-existent-file-$(VERSION)
	@kcov --report-only kcov/ ./emma

kcov-show:
	xdg-open kcov/index.html

kcov-clean:
	rm -rf kcov
