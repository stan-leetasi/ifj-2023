# Projekt IFJ2023
# Makefile
# Autor: Michal Krulich (xkruli03)
# Datum: 11.11.2023

.PHONY=all clean

CC=gcc
CFLAGS=-Wall -Wextra -std=c17

all: main.out

clean:
	rm -f *.out *.o

main.out: main.c dll.c parser.c scanner.c strR.c symtable.c logErr.c exp.c generator.c decode.c
	${CC} ${CFLAGS} -o $@ $^
