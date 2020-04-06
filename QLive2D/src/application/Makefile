all:build/release/QLive2D.exe *.*

build/release/QLive2D.exe: build/Makefile build/release
	cd build
	nmake
	cd ..

build/release:build
	mkdir build/release
	
build:
	mkdir build
	
build/Makefile: 
	qmake -makefile src -o build/Makefile
	
clean: *.*
	- RMDIR /S /Q build