#!/bin/bash

# Ako spustiť testy:
# bash test.sh dll
# bash test.sh scanner
# bash test.sh strR
# bash test.sh symtable
# bash test.sh syntax_dbg
# bash test.sh semantic_dbg

if [ $# -gt 0 ]; then
    if [ ! -d "tests/$1" ]; then 
        echo "Incorrect argument"
        exit 1
    fi
    cd "tests/$1" || exit 1
    bash "test.sh"
else
    echo "enter test name:"
    echo "bash test.sh dll"
    echo "bash test.sh scanner"
    echo "bash test.sh strR"
    echo "bash test.sh symtable"
    echo "bash test.sh syntax_dbg"
    echo "bash test.sh semantic_dbg"
fi
