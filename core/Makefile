all: shavds.cpp
	clang++ `llvm-config --cxxflags` -Wl,-znodelete -fno-rtti -fPIC -shared -std=c++17 shavds.cpp -o shavds.so `llvm-config --ldflags` 