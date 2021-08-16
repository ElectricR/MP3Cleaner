MP3Cleaner: src/main.cpp
	cmake -Bbuild -DCMAKE_EXPORT_COMPILE_COMMANDS=1; cd build; make

run: MP3Cleaner
	./MP3Cleaner

install:
	git clone https://github.com/taglib/taglib
	cd taglib; \
		mkdir build; \
		cmake -DCMAKE_BUILD_TYPE=Release -Bbuild -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=$(shell pwd)/lib/tag; \
		cd build; \
		make; \
		make install;
