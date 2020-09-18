SHELL := /bin/bash
all:
	make -C core
	make -C server

clean:
	-find ./ -regex ".*\.dot\|.*\.ll\|.*\.bc\|.*\.png" -exec rm -v {} \;