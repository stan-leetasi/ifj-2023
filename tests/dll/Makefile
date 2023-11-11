.PHONY=all clean

CC=gcc
CFLAGS=-g -Wall -Wextra -std=c17 -O2

all: test.out

clean:
	rm -f *.out *.o

test.out: test.c ../../strR.c ../../dll.c
	${CC} ${CFLAGS} -o $@ $^

