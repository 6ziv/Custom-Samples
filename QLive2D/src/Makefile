all: build_application build_plugins *.*

build_application:*.*
	cd application
	nmake
	cd ..

build_plugins:*.*
	cd plugins
	nmake
	cd ..
	
clean:clean_application clean_plugins *.*

clean_application: *.*
	cd application
	nmake clean
	cd ..

clean_plugins: *.*
	cd plugins
	nmake clean
	cd ..