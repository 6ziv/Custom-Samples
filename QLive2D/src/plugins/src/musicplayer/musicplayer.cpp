#include "pluginhelper.h"
#include <iostream>
bool(*exesetmusic)(const wchar_t* name);
bool ondropfile(wchar_t* filepath){
	std::cout<<"DROP"<<std::endl;
	std::wcout<<filepath<<std::endl;
	return exesetmusic(filepath);
}
static PLUGIN_FUNCTIONS* functions;
bool Initialize(uint32_t token,PLUGIN_FUNCTIONS *f){
	std::cout<<"INIT"<<std::endl;
	functions=f;
	std::cout<<"INIT1"<<std::endl;
	f->release=nullptr;
	std::cout<<"INIT1.1"<<std::endl;
	f->ondropdir=nullptr;
	std::cout<<"INIT1.5"<<std::endl;
	f->ondropfile=ondropfile;
	std::cout<<"INIT1.75"<<std::endl;
	f->ondropfiles=nullptr;
	std::cout<<"INIT2"<<std::endl;
	exesetmusic=f->setmusic;
	std::cout<<"INIT3"<<std::endl;
	return true;
}
