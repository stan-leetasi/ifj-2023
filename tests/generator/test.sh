#!/bin/bash

make test.out  || exit 1
echo "Executing generator unit tests"
./test.out
