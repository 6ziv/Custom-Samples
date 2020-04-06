all:plugins conf build_archiver build_learnhelper build_musicplayer *.*

plugins:
	mkdir plugins

conf:
	mkdir conf
	
build_archiver:plugins conf *.*
	cd src/archiver
	nmake install_and_clean
	cd ../..
	
build_learnhelper:plugins conf *.*
	cd src/learnhelper
	nmake install_and_clean
	cd ../..

build_musicplayer:plugins conf *.*
	cd src/musicplayer
	nmake install_and_clean
	cd ../..
	
clean: *.*
	- RMDIR /S /Q conf
	- RMDIR /S /Q plugins