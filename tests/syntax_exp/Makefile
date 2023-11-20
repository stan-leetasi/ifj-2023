.PHONY=all clean

CC=gcc
CFLAGS=-Wall -Wextra -std=c17 -g

all: test.out

clean:
	rm -f *.out *.o

test.out: test.c ../../dll.c ../../parser.c ../../scanner.c ../../strR.c ../../symtable.c ../../logErr.c ../../exp.c ../../generator.c ../../decode.c
	${CC} ${CFLAGS} -o $@ $^
