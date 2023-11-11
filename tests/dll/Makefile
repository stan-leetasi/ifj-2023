.PHONY=all clean

CC=gcc
CFLAGS=-Wall -Wextra -std=c17 -g

all: test.out

clean:
	rm -f *.out *.o

test.out: test.c ../../strR.c ../../dll.c
	${CC} ${CFLAGS} -o $@ $^

