all: shavds.cpp
	clang++ `llvm-config --cxxflags` -Wl,-znodelete -fno-rtti -fPIC -shared -std=c++17 -O2 shavds.cpp -o shavds.so `llvm-config --ldflags` 

install: all
	cp ./shavds.so /opt/lib