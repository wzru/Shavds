clang++ 1.cpp -O3 -S -emit-llvm -o 1.ll
echo "---1.ll---"
opt -load ./shavds.so -obfuscate < 1.ll > 11.bc
llvm-dis 11.bc -o 11.ll

clang++ 2.cpp -O3 -S -emit-llvm -o 2.ll
echo "---2.ll---"
opt -load ./shavds.so -obfuscate < 2.ll > 22.bc
llvm-dis 22.bc -o 22.ll

echo "---merge.ll---"
# cat 11.ll 22.ll >> merge.ll
llvm-link 11.ll 22.ll -S -o merge.ll
opt -load ./shavds.so -hello < merge.ll > /dev/null