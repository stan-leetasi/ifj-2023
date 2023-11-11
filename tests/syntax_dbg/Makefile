.PHONY=all clean

CC=gcc
CFLAGS=-Wall -Wextra -std=c17 -g

all: main.out

clean:
	rm -f *.out *.o

main.out: ../../main.c ../../dll.c ../../parser.c ../../scanner.c ../../strR.c ../../symtable.c ../../logErr.c ../../exp.c ../../generator.c
	${CC} ${CFLAGS} -o $@ $^
