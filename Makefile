# Projekt IFJ2023
# Makefile
# Autor: Michal Krulich (xkruli03)
# Datum: 03.10.2023

.PHONY=all clean

CC=gcc
CFLAGS=-g -Wall -Wextra -std=c17 -O2

all: main.out

clean:
	rm -f *.out *.o

main.out: main.o dll.o parser.o scanner.o strR.o symtable.o
	${CC} ${CFLAGS} -o $@ $?

%.o: %.c
	${CC} ${CFLAGS} -c $?
