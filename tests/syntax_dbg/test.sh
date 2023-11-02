#!/bin/bash

echo "Executing syntax-check test, check results yourself for now"

for f in *.sample
do
    echo "----------------------------------------------------------"
    test_sample=${f%.*}
    echo "Parsing ${f}"
    ./main.out <"${test_sample}.sample" >"/dev/null"
    result=$?
    echo -n "Code ${f} is: "
    if [ ${result} -eq 0 ]; then 
        echo -e "\t\tOK"
    else
        echo -e "\t\terr #${result}"
    fi
done
