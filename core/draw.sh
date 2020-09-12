#!/bin/bash
tp=$1
ll=$2
dir=$(dirname ${ll})
opt -dot-$1 $ll 1>/dev/null 2>/dev/null
for file in `ls -a | grep .dot`
do
    dot -Tpng -o ${dir}/${file%.*}.png ${file%.*}.dot
    rm ${file%.*}.dot
    echo "successfully generated picture ${dir}/${file%.*}.png !"
done