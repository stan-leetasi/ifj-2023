#!/bin/bash

# v syntax_exp.exp sú uvedené očakávané return kódy prekladača

# na prvom riadku sample-u je očakávaný typ výsledku výrazu

# rozdelenie samplov:
# 0e*.sample ... sémanticky správne
# #e*.sample ... sémanticky nesprávne, kde # značí číslo chyby

make || exit 1

echo "Executing semantic expression test"

result_file="semantic_exp.result"
rm -f ${result_file}

for f in *.sample
do
    echo "----------------------------------------------------------"
    test_sample=${f%.*}
    echo "Parsing ${f}"
    ./test.out <"${test_sample}.sample" >>"EXPOUT.txt"
    result=$?
    echo -e "${test_sample}" "$(printf "%d" ${result})" >>${result_file}
    echo -n "Code ${f} is: "
    if [ ${result} -eq 0 ]; then 
        echo -e "\t\tOK"
    else
        echo -e "\t\terr #${result}"
    fi
done

echo "----------------------------------------------------------"
diff --color "${result_file%.result}.exp" "${result_file}"
DIFF=$?
if [ ${DIFF} -eq 0 ]; then 
    echo "[PASS]"
else
    echo "[FAIL]"
fi
