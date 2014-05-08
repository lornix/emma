#
CFLAGS=-g -O0
CFLAGS+=-Wall -Wextra -Wunused
#
LDFLAGS=-lm
#
NFLAGS=-g -felf64
#
CC:=gcc
CXX:=g++
NASM:=nasm
#
.SUFFIXES:
.SUFFIXES: .c .cpp .asm .s .o
#
.phony: all clean

OBJS=countinstr environ locale_commas rscalc sqrt tracer1 pokefunction rdtsc tst32

all: $(OBJS)

% :%.s
	as -g $< -o $@.o
	ld $@.o -o $@

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

%: %.cpp
	$(CXX) $(CFLAGS) $(LDFLAGS) $< -o $@

%: %.asm
	$(NASM) $(NFLAGS) $< -o $@.o
	$(LD) -o $@ $@.o -lc || $(CC) -o $@ $@.o -lc

clean:
	rm -f *.o $(OBJS)
