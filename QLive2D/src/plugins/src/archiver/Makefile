all: archiver.dll *.*

install_and_clean:install clean *.*

install: archiver.dll *.*
	copy /Y archiver.dll ..\..\plugins

archiver.dll: archiver.obj archiver.def
	link /DLL archiver.obj libzip/lib/zip.lib libzip/lib/libbz2.lib libzip/lib/liblzma.lib libzip/lib/zlibstatic.lib Shlwapi.lib Advapi32.lib /DEF:archiver.def
	
archiver.obj: archiver.cpp pluginhelper.h
	cl /c archiver.cpp /MT -I libzip/include /std:c++17 /EHsc /Foarchiver.obj
	
clean: *.*
	- del archiver.obj
	- del archiver.dll
	- del archiver.exp
	- del archiver.lib