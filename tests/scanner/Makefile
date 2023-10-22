.PHONY=all clean

CC=gcc
CFLAGS=-g -Wall -Wextra -std=c17 -O2

all: test.out

clean:
	rm -f *.out *.o

test.out: test.o scanner.o strR.o
	${CC} ${CFLAGS} -o $@ $^

%.o: %.c
	${CC} ${CFLAGS} -c $?
