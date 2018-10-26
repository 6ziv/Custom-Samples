#include "system.h"
#include <Windows.h>
#include <QApplication>
#include "explorer.h"
[[noreturn]] inline void die(){
    Explorer::Exit();
    QApplication::exit(0);  //inside event loop
    exit(0);                //outside event loop
}
[[noreturn]] void error(const char *message){
    MessageBoxA(nullptr,message,"Error",MB_OK);
    die();
}
