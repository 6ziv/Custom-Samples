#include "apputils.hpp"
#include <Windows.h>

bool AppUtils::readFileContent(const std::string filePath, std::vector<std::byte>& result)
{
    const char* path = filePath.c_str();
    HANDLE hFile=CreateFileA(path,FILE_READ_ATTRIBUTES|FILE_READ_DATA,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if(hFile==INVALID_HANDLE_VALUE){
        hFile=CreateFileA(path,FILE_READ_DATA,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    }
    if(hFile==INVALID_HANDLE_VALUE)
        return false;
    LARGE_INTEGER fSize;
    bool ok=true;
    if(FALSE==GetFileSizeEx(hFile,&fSize) || fSize.QuadPart<0){
        result.clear();
        std::byte tmp_buffer[256];
        DWORD read;
        do{
            if(FALSE==ReadFile(hFile,tmp_buffer,256,&read,NULL)){
                ok=false;
                break;
            }
            result.assign(tmp_buffer,tmp_buffer+read);
        }while(read!=0);
    }else{
        result.resize(fSize.QuadPart);
        LARGE_INTEGER readSize;
        readSize.QuadPart=0;
        while(readSize.QuadPart<fSize.QuadPart){
            DWORD bytesread;
            DWORD bytestoread;
            if(readSize.QuadPart+0xFFFFFFFF<fSize.QuadPart)//MAXDWORD
                bytestoread=0xFFFFFFFF;
            else
                bytestoread=fSize.QuadPart-readSize.QuadPart;
            if(FALSE==ReadFile(hFile,result.data()+readSize.QuadPart,bytestoread,&bytesread,NULL)){
                ok=false;
                break;
            }
            readSize.QuadPart+=bytesread;
        }
    }
    if(!ok)result.clear();
    return ok;
}
