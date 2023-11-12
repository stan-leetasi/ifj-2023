#!/bin/bash

make test.out  || exit 1
echo "Executing symtable unit tests"
./test.out
