SHELL := /bin/bash
all: core
	make -C core
clean:
	-find ./ -regex ".*\.dot\|.*\.ll\|.*\.bc\|.*\.png" -exec rm -v {} \;