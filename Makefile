SHELL := bash

all:
	make -C core
	make -C server
	# cd client && npm install
	
clean:
	-find ./samples -regex ".*\.dot\|.*\.ll\|.*\.bc\|.*\.png" -exec rm -v {} \;
	rm ./server/shavds-server 
	rm ./core/shavds.so