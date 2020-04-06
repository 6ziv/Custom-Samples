all: musicplayer.dll *.*

install_and_clean:install clean *.*

install: musicplayer.dll *.*
	copy /Y musicplayer.dll ..\..\plugins
	
musicplayer.dll: musicplayer.obj musicplayer.def
	link /DLL musicplayer.obj /DEF:musicplayer.def
	
musicplayer.obj: musicplayer.cpp pluginhelper.h
	cl /c musicplayer.cpp /MT /Fomusicplayer.obj
	
clean:  *.*
	- del musicplayer.obj
	- del musicplayer.dll
	- del musicplayer.exp
	- del musicplayer.lib