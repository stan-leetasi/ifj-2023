#!/bin/bash
make || exit 1
if [ $# -gt 0 ]; then
    for f in *.o
    do
        cp "${f}" "tests/$1"
    done
    cd "tests/$1" || exit 1
    bash "test.sh"
else
    echo "enter test name"
fi
