#!/bin/bash

# rozdelenie samplov:
# 10+ ... citanie zo STDIN
# 20+ ... funkcie

make || exit 1

echo "Executing system tests"

#result_file="syntax_dbg.result"
#rm -f ${result_file}

compile_ok=true

for f in *.sample
do
    echo "----------------------------------------------------------"
    test_sample=${f%.*}
    #echo "Compiling ${f}"
    ./main.out <"${test_sample}.sample" >"${test_sample}.prog"
    result=$?
    #echo -e "${test_sample} " "$(printf "%d" ${result})" >>${result_file}
    #echo -n "Code ${f} is: "
    if [ ${result} -eq 0 ]; then 
        #echo -e "\t\tOK"
        result=0
    else
        compile_ok=false
        echo -e "Failed to compile ${f}\t\terr #${result}"
    fi
done

echo "----------------------------------------------------------"

if ${compile_ok} ; then 
    echo "[PASS] - Everything compiled successfully"
    echo "         Now try system_run tests"
else
    echo "[FAIL]"
fi
