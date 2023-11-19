.PHONY=all clean

CC=gcc
CFLAGS=-g -Wall -Wextra -std=c17

all: test.out

clean:
	rm -f *.out *.o

test.out: test.c ../../parser.c ../../decode.c ../../generator.c ../../exp.c ../../strR.c ../../symtable.c ../../dll.c ../../logErr.c ../../scanner.c
	${CC} ${CFLAGS} -o $@ $^
