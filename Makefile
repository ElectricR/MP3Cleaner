MP3Cleaner: src/main.cpp
	cmake -B.build -DCMAKE_EXPORT_COMPILE_COMMANDS=1; cd .build; make

run: MP3Cleaner
	./MP3Cleaner

debug: MP3Cleaner
	gdb MP3Cleaner

install:
	sudo pacman -S taglib1
