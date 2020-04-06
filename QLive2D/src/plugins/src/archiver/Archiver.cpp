#include "pluginhelper.h"
#include <zip.h>
#include <string>
#include <iostream>
#include <filesystem>
#include <Windows.h>
#include <Shlwapi.h>
void name_translate(std::wstring ws,char **ret){
	std::wcout<<L"Translate:"<<ws<<std::endl;
	char tmp;
	int rSize=WideCharToMultiByte(CP_UTF8,NULL,ws.c_str(),-1,&tmp,0,NULL,NULL);
	char* dname=reinterpret_cast<char*>(malloc(rSize+1));
	rSize=WideCharToMultiByte(CP_UTF8,NULL,ws.c_str(),-1,dname,rSize,NULL,NULL);
	dname[rSize]=0;
	std::wcout<<L"Translate:"<<rSize<<std::endl;
	*ret=dname;
}
typedef struct{
	wchar_t* name;
	FILE* file;
	bool done;
}USERDATA;
zip_int64_t callback(void *userdata, void *data, zip_uint64_t len, zip_source_cmd_t cmd){
	auto usr=reinterpret_cast<USERDATA*>(userdata);
	if(cmd==ZIP_SOURCE_OPEN){
		std::wcout<<L"OPEN:"<<usr->name<<std::endl;
		return 0;
	}
	if(cmd==ZIP_SOURCE_STAT){
		zip_stat_init(reinterpret_cast<zip_stat_t*>(data));
		return sizeof(zip_stat_t);
	}
	if(cmd==ZIP_SOURCE_CLOSE){
		std::wcout<<L"CLOSE:"<<usr->name<<std::endl;
		if(usr->file!=nullptr){
			fclose(usr->file);
			usr->file=nullptr;
		}
		if(usr->name!=nullptr){
			delete usr->name;
			usr->name=nullptr;
		}
		delete usr;
		return 0;
	}
	if(cmd==ZIP_SOURCE_ERROR){
		return 2 * sizeof(int);
	}
	if(cmd==ZIP_SOURCE_READ){
		if(usr->done)return 0;
		if(usr->file==nullptr){
			usr->file=_wfopen(usr->name,L"rb");
		}
		size_t ret=fread(data,1,len,usr->file);
		if(ret<len){
			std::wcout<<L"RDDONE:"<<usr->name<<std::endl;
			usr->done=true;
			fclose(usr->file);
			usr->file=nullptr;
		}
		return ret;
	}
	return -1;
}
void addFile(zip_t* zip,std::wstring path,std::wstring inner_path){
	USERDATA *d=new USERDATA;
	d->name=new wchar_t[path.length()+1];
	wcscpy(d->name,path.c_str());
	d->file=nullptr;
	d->done=false;
	zip_source_t* src=zip_source_function(zip,callback,d);
	
	if(NULL==src){
		delete d->name;
		delete d;
		std::cout<<"Fail:Create source"<<std::endl;
		return;
	}
	char* fname;
	name_translate(inner_path,&fname);
	zip_file_add(zip,fname,src,ZIP_FL_ENC_UTF_8);
	free(fname);
	
}
void addDir(zip_t *zip,std::wstring path,std::wstring inner_path){
	
	char* dir;
	name_translate(inner_path,&dir);
	zip_dir_add(zip,dir,ZIP_FL_ENC_UTF_8);
	free(dir);
	WIN32_FIND_DATAW FindFileData;
	HANDLE hFind=FindFirstFileW((path+L"/*").c_str(), &FindFileData);
	if(hFind==INVALID_HANDLE_VALUE)return;
	do{
		std::wcout<<FindFileData.cFileName<<std::endl;
		if(wcscmp(FindFileData.cFileName,L".")==0)continue;
		if(wcscmp(FindFileData.cFileName,L"..")==0)continue;
		if(FILE_ATTRIBUTE_DIRECTORY&FindFileData.dwFileAttributes){
			addDir(zip,path+L"/"+FindFileData.cFileName,inner_path+L"/"+FindFileData.cFileName);
		}else{
			addFile(zip,path+L"/"+FindFileData.cFileName,inner_path+L"/"+FindFileData.cFileName);
		}
	}while(FindNextFileW(hFind,&FindFileData));
	FindClose(hFind);
}
bool onfiles(uint64_t count,wchar_t** filenames){
	if(count==0)return true;
	SYSTEMTIME time;
	GetLocalTime(&time);
	
	int err=0;
	zip_t* zip=zip_open("temp.zip",ZIP_TRUNCATE|ZIP_CREATE,&err);
	if(zip==nullptr){
		std::cout<<"Err="<<err<<std::endl;
		return true;
	}
	
	for(uint64_t i=0;i<count;i++){
		std::filesystem::path path(filenames[i]);
		DWORD attributes=GetFileAttributesW(filenames[i]);
		if(attributes==INVALID_FILE_ATTRIBUTES)continue;
		if(FILE_ATTRIBUTE_DIRECTORY&attributes){
			addDir(zip,filenames[i],path.filename().wstring());
		}else{
			addFile(zip,filenames[i],path.filename().wstring());
		}
	}
	zip_close(zip);
	
	std::filesystem::path filepath1(filenames[0]);
	std::filesystem::path filedir=filepath1.parent_path();
	std::filesystem::path p_prefix=filedir;
	p_prefix.append(filedir.filename().wstring());
	std::wstring prefix=p_prefix.wstring();
	prefix+=std::wstring(L"-")+std::to_wstring(time.wYear)+L"-"+std::to_wstring(time.wMonth)+L"-"+std::to_wstring(time.wDay);
	std::wstring postfix=L".zip";
	
	std::wstring zipname=prefix+postfix;
	std::wcout<<zipname<<std::endl;
	if(PathFileExistsW(zipname.c_str())){
		int tmp=0;
		do{
			tmp++;
			zipname=prefix+L"-"+std::to_wstring(tmp)+postfix;
		}
		while(PathFileExistsW(zipname.c_str()));
	}
	MoveFileW(L"temp.zip",zipname.c_str());
	
	return true;
}
bool ondir(wchar_t* dirname){
	std::wstring prefix=dirname;
	std::wstring postfix=L".zip";
	std::wstring zipname=prefix+postfix;
	if(PathFileExistsW(zipname.c_str())){
		int tmp=0;
		do{
			tmp++;
			zipname=prefix+L"-"+std::to_wstring(tmp)+postfix;
		}
		while(PathFileExistsW(zipname.c_str()));
	}
	
	int err=0;
	zip_t* zip=zip_open("temp.zip",ZIP_TRUNCATE|ZIP_CREATE,&err);
	if(zip==nullptr){
		std::cout<<"Err="<<err<<std::endl;
		return true;
	}
	
	std::filesystem::path dir_path(dirname);
	addDir(zip,dir_path,dir_path.filename().wstring());
	zip_close(zip);
	MoveFileW(L"temp.zip",zipname.c_str());
	return true;
}
bool Initialize(uint32_t token,PLUGIN_FUNCTIONS *f){
	f->release=nullptr;
	f->ondropdir=ondir;
	f->ondropfile=nullptr;
	f->ondropfiles=onfiles;
	f->initNetwork(token);
	return true;
}
