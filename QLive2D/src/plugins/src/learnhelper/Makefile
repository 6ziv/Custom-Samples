all: learnhelper.dll *.*

install_and_clean:install clean *.*

install: learnhelper.dll *.*
	copy /Y learnhelper.dll ..\..\plugins
	copy /Y learnhelper.config ..\..\conf

learnhelper.dll: learnhelper.obj learnhelper.def
	link /DLL learnhelper.obj shell32.lib /DEF:learnhelper.def
	
learnhelper.obj: learnhelper.cpp pluginhelper.h
	cl /c learnhelper.cpp /MT /Folearnhelper.obj
	
clean: *.*
	- del learnhelper.obj
	- del learnhelper.dll
	- del learnhelper.exp
	- del learnhelper.lib