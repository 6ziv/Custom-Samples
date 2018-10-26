#include <QApplication>
#include"webpage.h"
#include"desktopitem.h"
#include"explorer.h"
#include"eventlistener.h"
#include"system.h"
#include<Windows.h>
#include<QScreen>
#include "settings.h"
#include<QSettings>
#include"io.h"
int main(int argc, char *argv[])
{
    if(_access("OpenDesktop.dll",4)!=0){
        error("dll missing!");
    };
    SetLastError(0);
    CreateMutexA(nullptr,FALSE,"OpenDesktop");
    if(GetLastError()){
        MessageBoxA(nullptr,"An instance has already been running!","Error",MB_OK);
        die();
    }

    QApplication a(argc, argv);
    StartListener();
    if(!Explorer::init())
        error("Error Initializing.");
    a.setQuitOnLastWindowClosed(false);
    return a.exec();
}
