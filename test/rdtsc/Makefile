#
.phony: all

all: rdtsc

rdtsc: rdtsc.s
	as -g rdtsc.s -o rdtsc.o
	ld rdtsc.o -o rdtsc
#	rm rdtsc.o

clean:
	rm -f rdtsc.o rdtsc

