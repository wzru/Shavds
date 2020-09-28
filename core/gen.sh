#!/bin/bash
RED='\e[1;41m'  # 红
GREEN='\e[1;42m'  # 绿
YELLOW='\e[43;37m'  # 黄
BLUE='\e[1;44m'  # 蓝
PINK='\e[1;45m'  # 粉红
RES='\e[0m'  # 清除颜色

if [ $# -ge 2 ]
then
    optm=-O3
    dbg=
    while getopts "O:g" opt
    do
        case $opt in
            O)
                optm=-O$OPTARG
                ;;
            g)
                dbg=-g
                ;;
            ?)
                echo -e "there is some ${RED}unrecognized${RES} parameters"
                exit 1
                ;;
        esac
    done
    #通过shift $(($OPTIND - 1))的处理，$*中就只保留了除去选项内容的参数，可以在后面的shell程序中进行处理
    shift $(($OPTIND - 1))
    lls=()
    for file in $@
    do
        echo "processing '${file}'..."
        clang++ $file $optm $dbg -S -emit-llvm -o ${file%.*}.ll
        if [ $? -ne 0 ]
        then
            echo -e "${RED}failed${RES} to process '${file}'"
            exit 1
        fi
        opt -load core/shavds.so -obfuscate < ${file%.*}.ll > ${file%.*}.bc
        llvm-dis ${file%.*}.bc -o ${file%.*}-obfuscate.ll
        rm ${file%.*}.bc
        if [ $? -eq 0 ]
        then
            echo -e "successfully generated '${file%.*}.ll'"
        fi
        lls+=(${file%.*}-obfuscate.ll)
    done
    if [ ${#lls[*]} -le 1 ]
    then
        echo -e "no need to merge."
        exit 1
    fi
    merge="$(dirname $file)/merge.ll"
    llvm-link ${lls[*]} -S -o $merge
    if [ $? -eq 0 ]
    then
        echo -e "${GREEN}successfully${RES} generated '${BLUE}${merge}${RES}'"
    fi
    # 删除obfuscate文件
    for file in $@
    do
        rm ${file%.*}-obfuscate.ll
    done

else
    echo -e "${PINK}usage: gen.sh -O[0 | 1 | 2 | 3] [-g] [*.c | *.cpp]...${RES}"
fi