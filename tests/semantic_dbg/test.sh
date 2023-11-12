#!/bin/bash

# v semantic_dbg.exp sú uvedené očakávané return kódy prekladača

# rozdelenie samplov:
# (bez prefixu) *.sample ... sémanticky správne
# #e*.sample ... sémanticky nesprávne, kde # značí číslo chyby
# testy s cislami 50+ su pre vyrazy

make || exit 1

echo "Executing semantic-check test"

result_file="semantic_dbg.result"
rm -f ${result_file}

for f in *.sample
do
    echo "----------------------------------------------------------"
    test_sample=${f%.*}
    echo "Parsing ${f}"
    ./main.out <"${test_sample}.sample" >"/dev/null"
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
