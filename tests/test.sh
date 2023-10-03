#!/bin/bash

if [ $# -gt 0 ]; then
    bash "$1/test.sh"
else
    echo "error"
fi
