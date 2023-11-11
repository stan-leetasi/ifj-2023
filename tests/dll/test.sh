#!/bin/bash

make test.out  || exit 1
echo "Executing dll unit tests"
./test.out
make clean
