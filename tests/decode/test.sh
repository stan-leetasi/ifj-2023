#!/bin/bash

make test.out  || exit 1
echo "Executing strEncode unit tests"
./test.out <input.sample >output.result
diff "output.exp" "output.result"
result=$?
if [ ${result} -eq 0 ]; then
    echo "[PASS]"
else
     echo "[FAIL]"
fi
