#!/bin/bash

# rozdelenie samplov:
# 10+ ... citanie zo STDIN
# 20+ ... funkcie

make || exit 1

echo "Executing system tests"

#result_file="syntax_dbg.result"
#rm -f ${result_file}

compile_ok=true

echo "----------------------------------------------------------"
for f in *.sample
do
    test_sample=${f%.*}
    #echo "Compiling ${f}"
    ./main.out <"${test_sample}.sample" >"${test_sample}.prog"
    result=$?
    if [ ${result} -eq 0 ]; then 
        #echo -e "\t\tOK"
        result=0
    else
        rm "${test_sample}.prog"
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
