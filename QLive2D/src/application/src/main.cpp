#include "app.hpp"
#pragma comment(lib,"Live2DCubismCore_MT.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"bass.lib")
#pragma comment(lib,"Propsys.lib")
#pragma comment(lib,"Shlwapi.lib")
#include <QApplication>
#include <QScreen>
#include "musicplayer.hpp"
#include "qdebug.h"
#include "pluginloader.hpp"
#include "message.h"
#include "setup.h"
#include <QDir>
#include "networkmgr.h"
#include "settings.h"
int main(int argc,char** argv)
{
    QApplication app(argc,argv);
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    for(int i=1;i<argc;i++){
        if(_stricmp(argv[i],"-ToastActivated")==0){
            Message::toast_done=[&app](){app.exit(0);};
            Message::Initialize();
            return app.exec();
        }
    }
    if(Setup::CreateShortcut()==-1)return 0;
    QSize scrsize=app.screens()[0]->size();
    if (!App::GetInstance()->Initialize(scrsize.width(),scrsize.height()-1))
		//-1 to avoid fullscreen, 
		//for QOpenGLWidget sometimes have trouble dealing with transparency when fullscreen
        return 0;
    Message::Initialize();
    NetworkMgr::Init();
    Settings::InitSettings(
                reinterpret_cast<HWND>(App::GetInstance()->GetWindow()->winId()),
                "settings.conf"
                );
    while(1){
        App::GetInstance()->dowork();
        app.processEvents();
        Sleep(50);
    }
    return 0;
}

