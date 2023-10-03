# Projekt IFJ2023
# Makefile
# Autor: Michal Krulich (xkruli03)
# Datum: 03.10.2023

.PHONY=all clean

CC=gcc
CFLAGS=-g -Wall -Wextra -std=c17 -O2

all: main.out

clean:
	rm -f *.out

main.out: main.c
	${CC} ${CFLAGS} -o $@ $<
