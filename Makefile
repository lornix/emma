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
CFLAGS+=-Wconversion
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
# LDFLAGS+=-lbfd
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
.PHONY: all clean all-clean kcov kcov-clean kcov-show testprogs testprogs-clean

all: emma

OBJS=$(addsuffix .o,$(basename $(wildcard *.c)))
HDRS=$(wildcard *.h)

emma: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

%.o : %.c $(HDRS)
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o $@ $<

clean:
	rm -f emma $(OBJS)

kcov: all testprogs
	@rm -f /tmp/empty-file-$(VERSION) /tmp/unreadable-file-$(VERSION) /tmp/non-existent-file-$(VERSION)
	@touch /tmp/empty-file-$(VERSION) /tmp/unreadable-file-$(VERSION)
	@chmod 000 /tmp/unreadable-file-$(VERSION)
	-kcov --skip-solibs kcov/ ./emma
	-kcov --skip-solibs kcov/ ./emma /tmp/empty-file-$(VERSION)
	-kcov --skip-solibs kcov/ ./emma /tmp/unreadable-file-$(VERSION)
	-kcov --skip-solibs kcov/ ./emma /tmp/non-existent-file-$(VERSION)
	-kcov --skip-solibs kcov/ ./emma emma
	-kcov --skip-solibs kcov/ ./emma test/hello_cpp
	-kcov --skip-solibs kcov/ ./emma test/asm_hello
	-kcov --skip-solibs kcov/ ./emma test/asm_hello32
	-kcov --skip-solibs kcov/ ./emma Makefile
	@rm -f /tmp/empty-file-$(VERSION) /tmp/unreadable-file-$(VERSION) /tmp/non-existent-file-$(VERSION)
	@kcov --report-only kcov/ ./emma

kcov-show:
	xdg-open kcov/index.html

kcov-clean:
	rm -rf kcov

testprogs:
	$(MAKE) -C test

testprogs-clean:
	$(MAKE) -C test clean

all-clean: clean kcov-clean testprogs-clean
