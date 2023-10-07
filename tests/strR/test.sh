#!/bin/bash

make test.out  || exit 1
echo "Executing strR unit tests"
./test.out
make clean
