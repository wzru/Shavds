#!/bin/bash
if [ $# -ge 2 ]
then
    optm=-O3
    while getopts "O:" opt
    do
        case $opt in
            O)
                optm=-O$OPTARG
                # echo "optm="$optm
                ;;
            ?)
                echo "there is some unrecognized parameters."
                exit 1
                ;;
        esac
    done
    #通过shift $(($OPTIND - 1))的处理，$*中就只保留了除去选项内容的参数，可以在后面的shell程序中进行处理
    shift $(($OPTIND - 1))
    lls=()
    for file in $@
    do
        echo "processing ${file}..."
        clang++ $file $optm -g -S -emit-llvm -o ${file%.*}.ll
        opt -load core/shavds.so -obfuscate < ${file%.*}.ll > ${file%.*}.bc
        llvm-dis ${file%.*}.bc -o ${file%.*}-obfuscate.ll
        lls+=(${file%.*}-obfuscate.ll)
    done
    # echo ${lls[*]}
    # echo "file" ${file%.*}
    merge="$(dirname $file)/merge.ll"
    llvm-link ${lls[*]} -S -o $merge
    echo "successfully generated ${merge} !"
else
    echo $#
fi