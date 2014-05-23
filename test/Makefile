#
CFLAGS=-g -O0
CFLAGS+=-Wall -Wextra -Wunused
#CFLAGS+=--save-temps
#
LDFLAGS=-lm
#
ASFLAGS=-g
#
AS=as
CC:=gcc
CXX:=g++
#
.SUFFIXES:
.SUFFIXES: .c .cpp .s .o
#
.phony: all clean
#
SRC=$(wildcard *.c) $(wildcard *.cpp) $(wildcard *.s)
EXECS=$(basename $(SRC))
#
all: $(EXECS)

%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@.o
	$(CC) $(CFLAGS) $(LDFLAGS) $@.o -o $@

%: %.cpp
	$(CXX) $(CFLAGS) $(LDFLAGS) -c $< -o $@.o
	$(CXX) $(CFLAGS) $(LDFLAGS) $@.o -o $@

% :%.s
	$(AS) $(ASFLAGS) $< -o $@.o
	$(LD) $@.o -o $@

%32 :%32.s
	$(AS) --32 $(ASFLAGS) $< -o $@.o
	$(LD) -melf_i386 $@.o -o $@

clean:
	rm -f *.o $(EXECS)
