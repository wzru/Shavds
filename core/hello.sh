opt -load ./shavds.so -hello < 11.ll > /dev/null
echo "---2.ll---"
opt -load ./shavds.so -hello < 2.ll > /dev/null