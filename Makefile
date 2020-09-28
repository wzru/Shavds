SHELL := /bin/bash
all:
	make -C core
	make -C server

clean:
	-find ./samples -regex ".*\.dot\|.*\.ll\|.*\.bc\|.*\.png" -exec rm -v {} \;
	rm ./server/shavds-server 
	rm ./core/shavds.so