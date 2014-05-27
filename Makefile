#
# Generic compiles-it-all Makefile
#  Loni Nix <lornix@lornix.com>
#
# save any flags from environment
SAVE_CFLAGS:=$(CFLAGS)
# start empty again
CFLAGS=
#
# adhere to a higher standard!
CFLAGS+=-std=c99
#
# pretty much always want debugging symbols included
CFLAGS+=-g3
#
# optimize!
#CFLAGS+=-O
#
# or not!
CFLAGS+=-O0
#
# yell out all warnings and whatnot
CFLAGS+=-Wall -Wextra -Wunused -Wconversion
#
# Want to REALLY be picky? uncomment two lines below
#CFLAGS+=-Wreturn-local-addr -Wshadow -Wundef -Wwrite-strings -Wmissing-declarations -Wmissing-prototypes
#CFLAGS+=-Wdeprecated -Wdeprecated-declarations -Wpacked -Wpadded -Wunsuffixed-float-constants -Wdouble-promotion
#
# make all warnings into errors
#CFLAGS+=-Werror
#
# link time optimization! cool! smaller exec's!
#CFLAGS+=-flto
#LDFLAGS+=-flto
#
# enable for gmon performance statistics
#CFLAGS+=-pg
#
# preserve everything used to create binary, verbose assembly comments
#CFLAGS+=--save-temps -fverbose-asm
#
# das linker flags
# LDFLAGS+=
# LDFLAGS+=-lm
# LDFLAGS+=-lbfd
#
# add environment CFLAGS to end so they take precedence
CFLAGS+=$(SAVE_CFLAGS)
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
BIN=emma
#
CFLAGS+=-D'VERREV="$(VERREV)"'
#
.PHONY: all clean allclean testprogs testprogs-clean coverage cov

all: $(BIN)

OBJS=$(addsuffix .o,$(basename $(wildcard *.c)))
HDRS=$(wildcard *.h)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

%.o : %.c $(HDRS)
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o $@ $<

clean:
	rm -f $(BIN) $(OBJS)
	rm -f gmon.out *.gcda *.gcov *.gcno

coverage: clean testprogs
	@CFLAGS="--coverage" $(MAKE) all
	@# no file
	-./$(BIN)
	@# non-existent file
	@rm -f /tmp/non-existent-file-$(VERSION)
	-./$(BIN) /tmp/non-existent-file-$(VERSION)
	@# empty file
	@rm -f /tmp/empty-file-$(VERSION)
	@touch /tmp/empty-file-$(VERSION)
	-./$(BIN) /tmp/empty-file-$(VERSION)
	@rm -f /tmp/empty-file-$(VERSION)
	@# unreadable (empty) file
	@touch /tmp/unreadable-file-$(VERSION)
	@chmod 000 /tmp/unreadable-file-$(VERSION)
	-./$(BIN) /tmp/unreadable-file-$(VERSION)
	@rm -f /tmp/unreadable-file-$(VERSION)
	@# not an ELF file
	-./$(BIN) Makefile
	@# 64bit binary
	-./$(BIN) test/asm_hello
	@# 32bit binary
	-./$(BIN) test/asm_hello32
	@# a c++ binary
	-./$(BIN) test/hello_cpp
	@# itself
	-./$(BIN) $(BIN)
	@# whew - all done
	@echo
	@make --no-print-directory cov

testprogs:
	$(MAKE) -C test

testprogs-clean:
	$(MAKE) -C test clean

allclean: clean testprogs-clean

cov:
	@ls *.gcda >/dev/null 2>&1 && gcov -r -a *.c | awk ' \
		/^File/{ name=substr($$0, index($$0," ")); } \
		/^Lines/{ if (name=="") { print "===" }; print substr($$0,1,index($$0,":")) " " substr($$0,index($$0,":")+1) name; name=""; } \
		{ next }' || echo "Try 'make coverage' first"
