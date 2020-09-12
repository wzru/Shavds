# Shavds-Core

0. 编译

```shell
cd Shavds/core
make
cd ..
```

1. cmpfun

比较2个文件的所有函数相似性

```shell
cd Shavds/
./core/obfuscate-merge.sh -O3 ./samples/sum/*.cpp
./core/shavds.sh cmpfun ./samples/sum/merge.ll
```

2. draw

作call graph图

```shell
cd Shavds/
./core/obfuscate-merge.sh -O0 ./samples/func/*.cpp
./core/draw.sh callgraph ./samples/func/func1.ll
```

作cfg图

```shell
cd Shavds/
./core/obfuscate-merge.sh -O0 ./samples/func/*.cpp
./core/draw.sh cfg ./samples/func/func1.ll
```

