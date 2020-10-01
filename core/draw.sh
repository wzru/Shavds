#!/bin/bash
RED='\e[1;41m'  # 红
GREEN='\e[1;42m'  # 绿
YELLOW='\e[43;37m'  # 黄
BLUE='\e[1;44m'  # 蓝
PINK='\e[1;45m'  # 粉红
RES='\e[0m'  # 清除颜色

if [ $# -ge 2 ]
then
    tp=callgraph
    while getopts "T:" opt
    do
        case $opt in
            T)
                tp=$OPTARG
                ;;
            ?)
                echo -e "there is some ${RED}unrecognized${RES} parameters"
                exit 1
                ;;
        esac
    done
    #通过shift $(($OPTIND - 1))的处理，$*中就只保留了除去选项内容的参数，可以在后面的shell程序中进行处理
    shift $(($OPTIND - 1))
    echo -e "draw type is ${PINK}${tp}${RES}..."
    for file in $@
    do
        echo "processing ${file} ..."
        dir=$(dirname ${file})
        opt -dot-$tp $file 1>/dev/null 2>/dev/null
        for dot in `ls -a | grep .dot`
        do
            base=$(basename $file)
            if [ $tp == "cfg" ]
            then
                # echo dot=${dot%.*}
                dot_base=$(echo ${dot%.*} | sed 's/\.//g')-
                # echo dot_base=$dot_base
            else
                dot_base=""
            fi
            pic=${dir}/${dot_base}${base%.*}-${tp}.png
            dot -Tpng -o $pic ${dot%.*}.dot
            rm ${dot%.*}.dot
            if [ $? -eq 0 ]
            then
                echo -e "${YELLEW}removed${RES} '${dot%.*}.dot'"
            fi
            if [ $? -eq 0 ]
            then
                echo -e "successfully generated '${pic}'!"
            fi
        done
    done

else
    echo -e "${PINK}usage: draw.sh -T <callgraph | cfg> [*.ll | *.bc]...${RES}"
fi