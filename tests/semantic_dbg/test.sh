#!/bin/bash

# v semantic_dbg.exp sú uvedené očakávané return kódy prekladača

# rozdelenie samplov:
# 0e*.sample ... sémanticky správne
# #e*.sample ... sémanticky nesprávne, kde # značí číslo chyby
# testy s číslami 20-49 sú programy, kde sú volané funkcie predtým ako sú definované
# testy s cislami 50+ su pre vyrazy
# testy s cislami 70+ su pre built-in funkcie

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

echo -e "\nValgrind leak-check running, this may take a while"
valgrind_ok=true
for f in 0e*.sample
do
    test_sample=${f%.*}
    valgrind --error-exitcode=1 --leak-check=yes --errors-for-leak-kinds=all --exit-on-first-error=yes ./main.out <"${test_sample}.sample" >"/dev/null" 2>"/dev/null"
    result=$?
    if [ ${result} -eq 1 ]; then 
        echo -e "VALGRIND found errors with \t\t${f}"
        valgrind_ok=false
    else 
        echo -n "."
    fi
done

for f in *.sample
do
    test_sample=${f%.*}
    valgrind --error-exitcode=1 --exit-on-first-error=yes ./main.out <"${test_sample}.sample" >"/dev/null" 2>"/dev/null"
    result=$?
    if [ ${result} -eq 1 ]; then 
        echo -e "VALGRIND found errors with \t\t${f}"
        valgrind_ok=false
    else 
        echo -n "."
    fi
done

if ${valgrind_ok} ; then
    echo "[VALGRIND PASS]"
fi