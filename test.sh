#!/bin/bash

# Ako spustiť test:
# bash test.sh scanner

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
    done
    cd "tests/$1" || exit 1
    rm -f "test.result"
    bash "test.sh" 2> "test.result"
else
    echo "enter test name"
fi
