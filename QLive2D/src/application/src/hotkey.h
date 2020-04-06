#ifndef HOTKEY_H
#define HOTKEY_H
#include <QStringList>
#include <Windows.h>
#include <QSettings>
#include <thread>
class HotKey
{
public:
    inline static void addCommand(UINT mod,UINT vk,QString executable,QString parameter,bool UAC){
        if(FALSE==RegisterHotKey(window,commands.size(),mod,vk))return;
        std::wstring exec=executable.toStdWString();
        std::wstring tmpexe=L"\0";
        DWORD ret=ExpandEnvironmentStringsW(exec.c_str(),tmpexe.data(),0);
        tmpexe.resize(ret+1);
        ExpandEnvironmentStringsW(exec.c_str(),tmpexe.data(),ret+1);
        std::wstring exe=std::wstring(tmpexe.data(),ret);
        commands.push_back(std::make_tuple(exe,parameter.toStdWString(),UAC));
    }
    inline static void setHwnd(HWND win){
        window=win;
    }
    inline static void Run(int id){
        if(commands.size()<=id || id<0)return;
        std::wstring exe=std::get<0>(commands[id]);
        std::wstring parameter=std::get<1>(commands[id]);
        std::wstring op=std::get<2>(commands[id])?L"runas":L"open";
        std::thread th(
        [](std::wstring e,std::wstring p,std::wstring o)
        {
            ShellExecuteW(NULL,o.c_str(),e.c_str(),p.c_str(),NULL,SW_SHOWDEFAULT);
        },exe,parameter,op);
        th.detach();
    }
private:
    inline static HWND window;
    inline static std::vector<std::tuple<std::wstring,std::wstring,bool> >commands;
};

#endif // HOTKEY_H
