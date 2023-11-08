#!/bin/bash

# Ako spustiť testy:
# bash test.sh scanner
# bash test.sh strR
# bash test.sh symtable

# Niekedy treba extra spustiť make

make || exit 1
if [ $# -gt 0 ]; then
    if [ ! -d "tests/$1" ]; then 
        echo "Incorrect argument"
        exit 1
    fi
    for f in *.o
    do
        cp "${f}" "tests/$1"
        cp "main.out" "tests/$1"
    done
    cd "tests/$1" || exit 1
    bash "test.sh"
else
    echo "enter test name"
fi
