all: install_application install_plugins *.*

install_application:build_application release *.*
	COPY /Y src\application\build\release\QLive2D.exe release
	COPY /Y 3rdparty\bin\bass.dll release
	WINDEPLOYQT release

install_plugins:build_plugins release *.*
	XCOPY /E /C /I src\plugins\conf release\conf
	XCOPY /E /C /I src\plugins\plugins release\plugins
	
release:
	mkdir release

build_application: *.*
	cd src
	nmake build_application
	cd ..

build_plugins: *.*
	cd src
	nmake build_plugins
	cd ..

clean: *.*
	cd src
	nmake clean
	cd ..
	