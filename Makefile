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
CFLAGS+=-std=gnu99
#
# pretty much always want debugging symbols included
CFLAGS+=-ggdb3
#
# but only include libs as needed
CFLAGS+=-Wl,--as-needed
#
# Optimize! or not!
CFLAGS+=-O0
#
# yell out all warnings and whatnot
CFLAGS+=-Wall -Wextra -Wunused -Wconversion
CFLAGS+=-Wsign-conversion
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
# sometimes we want to see it all
#CFLAGS+=--save-temps -fverbose-asm
#
# das linker flags
LDFLAGS+=
#
# needed libraries
#LIBS+=-lm
#LIBS+=-lbfd
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
VERSION:=$(shell cat VERSION)
REVISION:=$(shell git rev-parse --short HEAD)
VERREV:=$(VERSION)-$(REVISION)
#
BIN:=emma
#
CFLAGS+=-D'VERREV="$(VERREV)"'
#
.PHONY: all clean allclean testprogs testprogs-clean coverage cov

all: $(BIN)

OBJS:=$(addsuffix .o,$(basename $(wildcard *.c)))
HDRS:=$(wildcard *.h)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o : %.c $(HDRS)
	$(CC) $(CFLAGS) $(LDFLAGS) -c -o $@ $< $(LIBS)

clean:
	$(RM) $(BIN) $(OBJS)
	$(RM) gmon.out *.gcda *.gcov *.gcno

coverage: clean testprogs
	@CFLAGS="--coverage" $(MAKE) all
	@# no file
	-./$(BIN)
	@# non-existent file
	@$(RM) /tmp/non-existent-file-$(VERSION)
	-./$(BIN) /tmp/non-existent-file-$(VERSION)
	@# empty file
	@$(RM) /tmp/empty-file-$(VERSION)
	@touch /tmp/empty-file-$(VERSION)
	-./$(BIN) /tmp/empty-file-$(VERSION)
	@$(RM) /tmp/empty-file-$(VERSION)
	@# unreadable (empty) file
	@touch /tmp/unreadable-file-$(VERSION)
	@chmod 000 /tmp/unreadable-file-$(VERSION)
	-./$(BIN) /tmp/unreadable-file-$(VERSION)
	@$(RM) /tmp/unreadable-file-$(VERSION)
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
	@ls *.gcda >/dev/null 2>&1 && \
	gcov -r -a *.c | \
	awk ' \
	/^File/{ index_apost=index($$0,"\047")+1; \
		name=" " substr($$0, index_apost,length($$0)-index_apost); } \
	/^Lines/{ if (name=="") { print " ================" }; \
		index_colon=index($$0,":")+1; \
		percent=substr($$0,index_colon,index($$0,"%")-index_colon); \
		line_count=substr($$0,4+index($$0," of ")); \
		printf "%7.2f%% of %5d%s\n", \
		percent, line_count, name; \
		name=""; } \
	{ next }' \
	|| echo "Try 'make coverage' first"
