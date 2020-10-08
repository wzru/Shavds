#!/bin/bash
RED='\e[1;41m'  # 红
GREEN='\e[1;42m'  # 绿
YELLOW='\e[43;37m'  # 黄
BLUE='\e[1;44m'  # 蓝
PINK='\e[1;45m'  # 粉红
RES='\e[0m'  # 清除颜色

ran=$(cat /proc/sys/kernel/random/uuid | md5sum | cut -c 1-17)

if [ $# -ge 2 ]
then
    optm=-O3
    dbg=
    f=
    while getopts "O:gf:" opt
    do
        case $opt in
            O)
                optm=-O$OPTARG
                ;;
            g)
                dbg=-g
                ;;
            f)
                f=-f$OPTARG
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
    bases=()
    for file in $@
    do
        echo "processing '${file}'..."
        dir=$(dirname ${file})/$ran
        base=$(basename ${file})
        base=${base%.*}
        # echo base=$base
        mkdir -p $dir
        clang++ $file $optm $dbg $f -S -emit-llvm -o ${dir}/${base}.ll
        # echo "clang++ $file $optm $dbg $f -S -emit-llvm -o ${file%.*}.ll"
        if [ $? -ne 0 ]
        then
            echo -e "${RED}failed${RES} to process '${file}'"
            exit 1
        fi
        opt -load core/shavds.so -obfuscate < ${dir}/${base}.ll > ${dir}/${base}.bc
        llvm-dis ${dir}/${base}.bc -o ${dir}/${base}-obfuscate.ll
        rm ${dir}/${base}.bc
        if [ $? -eq 0 ]
        then
            echo -e "successfully generated '${dir}/${base}.ll'"
        fi
        bases+=${base}
        lls+=(${dir}/${base}-obfuscate.ll)
    done
    # echo "bases=${bases[*]}"
    if [ ${#lls[*]} -ge 2 ]
    then
        merge="${dir}/${bases[*]}.ll"
        llvm-link ${lls[*]} -S -o $merge
        if [ $? -eq 0 ]
        then
            echo -e "${GREEN}successfully${RES} generated '${BLUE}${merge}${RES}'"
        fi
    fi
    # 删除obfuscate文件
    for file in $@
    do
        base=$(basename $file)
        rm ${dir}/${base%.*}-obfuscate.ll
    done

else
    echo -e "${PINK}usage: gen.sh -O[0 | 1 | 2 | 3] [-g] [*.c | *.cpp]...${RES}"
fi