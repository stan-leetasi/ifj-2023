#!/bin/bash
make test.out  || exit 1
echo "Executing scanner unit tests"
for f in ./*.sample
do
    echo "Processing $f file..."
    # take action on each file. $f store current file name
    test_sample=${f%.*}
    ./test.out <"${test_sample}.sample" >"${test_sample}.result"
    if "$(diff "${test_sample}.exp" "${test_sample}.result")"; then 
        echo -n "[PASS]"
    else
        echo -n "[FAIL]"
    fi
    echo " test #${test_sample}"
done
make clean
