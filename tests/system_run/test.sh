#!/bin/bash

echo "Executing SYSTEM RUN tests"
total=0
passed=0

for f in *.prog
do
    total=$((total + 1))
    test_sample=${f%.*}
    ./ic23int "$f" <"${test_sample}.stdin" >"${test_sample}.result"
    result=$?
    if [ ${result} -eq 0 ]; then 
        diff "${test_sample}.exp" "${test_sample}.result"
        result=$?
        if [ ${result} -eq 0 ]; then
            passed=$((passed + 1))
        else
            echo "${f}: different output"
        fi
    else
        echo -e "${f}: Interpreter returned \t\terr #${result}"
    fi
done

if [ "${total}" -eq "${passed}" ]; then
    echo "[PASS]"
else
    echo "[FAIL] Some tests failed ${passed}/${total}"
fi
