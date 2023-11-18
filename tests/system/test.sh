#!/bin/bash

# rozdelenie samplov:
# 10+ ... citanie zo STDIN
# 20+ ... funkcie

make || exit 1

echo "Executing system tests"

#result_file="syntax_dbg.result"
#rm -f ${result_file}

for f in *.sample
do
    echo "----------------------------------------------------------"
    test_sample=${f%.*}
    echo "Compiling ${f}"
    ./main.out <"${test_sample}.sample" >"${test_sample}.prog"
    result=$?
    #echo -e "${test_sample} " "$(printf "%d" ${result})" >>${result_file}
    #echo -n "Code ${f} is: "
    if [ ${result} -eq 0 ]; then 
        #echo -e "\t\tOK"
        result=0
    else
        echo -e "Failed to compile ${f}\t\terr #${result}"
    fi
done

echo "----------------------------------------------------------"
#diff --color "${result_file%.result}.exp" "${result_file}"
#DIFF=$?
#if [ ${DIFF} -eq 0 ]; then 
#    echo "[PASS]"
#else
#    echo "[FAIL]"
#fi
