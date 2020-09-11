#!/bin/bash
ll=$1
opt -dot-callgraph $ll 1>/dev/null 2>/dev/null
mv callgraph.dot ${ll%.*}.dot
dot -Tpng -o ${ll%.*}.png ${ll%.*}.dot
echo "successfully generated picture ${ll%.*}.png !"
# rm callgraph.dot