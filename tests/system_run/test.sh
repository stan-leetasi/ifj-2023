#!/bin/bash

echo "Executing SYSTEM RUN tests"
total=0
passed=0

for f in *.prog
do
    total=$((total + 1))
    test_sample=${f%.*}
    echo -n "${f}: "
    ./ic23int "${f}" <"${test_sample}.stdin" >"${test_sample}.result"
    result=$?
    if [ ${result} -eq 0 ]; then 
        diff "${test_sample}.stdout" "${test_sample}.result"
        result=$?
        if [ ${result} -eq 0 ]; then
            echo -e "\t\tOK"
            passed=$((passed + 1))
        else
            echo -e "Different output"
        fi
    else
        echo -e "Interpreter returned \t\terr #${result}"
    fi
done

if [ "${total}" -eq "${passed}" ]; then
    echo "[PASS]"
else
    echo "[FAIL] Some tests failed ${passed}/${total}"
fi
