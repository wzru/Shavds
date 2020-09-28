# Shavds-Core

1. cmpfun

比较多个文件中所有函数的相似性

```shell
cd Shavds/
./core/gen.sh -O3 -g ./samples/sum/*.cpp
./core/shavds.sh cmpfun ./samples/sum/merge.ll
```

2. cmpcfg

比较多个文件中所有函数的CFG

```shell
cd Shavds/
./core/gen.sh -O3 -g ./samples/sum/*.cpp
./core/shavds.sh cmpcfg ./samples/sum/merge.ll
```

3. draw

画call graph

```shell
cd Shavds/
./core/gen.sh -O0 -g ./samples/func/*.cpp
./core/draw.sh callgraph ./samples/func/func1.ll
```

画CFG

```shell
cd Shavds/
./core/gen.sh -O0 -g ./samples/func/*.cpp
./core/draw.sh cfg ./samples/func/func1.ll
```

