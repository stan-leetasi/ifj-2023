#!/bin/bash

# rozdelenie samplov:
# 10+ ... citanie zo STDIN
# 20+ ... funkcie

make || exit 1

echo "Executing system tests"

#result_file="syntax_dbg.result"
#rm -f ${result_file}

compile_ok=true

for f in *.swift
do
    test_sample=${f%.*}
    #echo "Compiling ${f}"
    ./main.out <"${test_sample}.swift" >"${test_sample}.prog"
    result=$?
    if [ ${result} -eq 0 ]; then 
        #echo -e "\t\tOK"
        result=0
    else
        compile_ok=false
        echo -e "Failed to compile ${f}\t\terr #${result}"
        echo "----------------------------------------------------------"
    fi
done


if ${compile_ok} ; then 
    echo "[PASS] - Everything compiled successfully"
    echo "         Now try system_run tests"
else
    echo "[FAIL]"
fi

if [ ! -d "../system_run" ] ; then
    echo "directory system_run not found"
    exit 0
fi
for f in *.prog
do
    mv "$f" "../system_run"
done

echo -e "\nValgrind leak-check running, this may take a while (checks only programs that compile successfully)"
valgrind_ok=true
for f in *.swift
do
    test_sample=${f%.*}
    ./main.out <"${test_sample}.swift" >"/dev/null" 2>"/dev/null"
    result=$?
    if [ ${result} -eq 0 ]; then 
        valgrind --error-exitcode=1 --leak-check=yes --errors-for-leak-kinds=all --exit-on-first-error=yes ./main.out <"${test_sample}.swift" >"/dev/null" 2>"/dev/null"
        result=$?
        if [ ${result} -eq 1 ]; then 
            echo -e "\nVALGRIND found errors with \t\t${f}"
            valgrind_ok=false
        else 
            echo -n "."
        fi
    fi
done

if ${valgrind_ok} ; then
    echo "[VALGRIND PASS]"
fi