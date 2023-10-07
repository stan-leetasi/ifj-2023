#!/bin/bash

make test.out  || exit 1
echo "Executing scanner unit tests"
total=0
passed=0
for f in *.sample
do
    total=$((total + 1))
    test_sample=${f%.*}
    ./test.out <"${test_sample}.sample" >"${test_sample}.result"
    DIFF="$(diff "${test_sample}.exp" "${test_sample}.result")"
    if [ "${DIFF}" == "" ]; then 
        echo -n "[PASS]"
        passed=$((passed + 1))
    else
        echo -n "[FAIL]"
    fi
    echo " test #${test_sample}"
done
if [ "${total}" -eq "${passed}" ]; then
    echo "Everything OK"
else
    echo "Some tests failed ${passed}/${total}"
fi
make clean
