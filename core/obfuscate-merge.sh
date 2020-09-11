#!/bin/bash
if [ $# -ge 2 ]
then
    lls=()
    for file in $@
    do
        # echo "file" ${file%.*}
        clang++ $file -O3 -g -S -emit-llvm -o ${file%.*}.ll
        opt -load shavds.so -obfuscate < ${file%.*}.ll > ${file%.*}.bc
        llvm-dis ${file%.*}.bc -o ${file%.*}.ll
        lls+=(${file%.*}.ll)
    done
    # echo ${lls[*]}
    # echo "file" ${file%.*}
    llvm-link ${lls[*]} -S -o $(dirname $file)/merge.ll
else
    echo $#
fi