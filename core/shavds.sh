#!/bin/bash
tp=$1
ll=$2
opt -load core/shavds.so -$tp < $ll > /dev/null