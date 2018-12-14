#include "compileheader.h"
#pragma warning(disable:4100)
#include "include/cef_app.h"
#include "include/cef_client.h"
#include "include/cef_render_handler.h"
#pragma warning(default:4100)
#include "renderwindow.h"
#include <QMainWindow>
#include <QApplication>
#include <QLabel>
#include <QBitmap>
#include <QTimer>
#include <QSettings>
#include <QFile>
#include <string>
#include "browserprocess.h"
#include "stdlib.h"
#include "zipreader.h"
#include "settings.h"

#include "include/cef_app.h"
#include "include/cef_browser.h"

int main(int argc, char *argv[])
{

    QApplication a(argc,argv);
    HINSTANCE hInstance=GetModuleHandle(nullptr);

    if(argc<2)
    {
    Settings settings;
    return a.exec();
    }

    CefMainArgs args(hInstance);
    QString LoadPath;
    {
    QSettings Reg(QSettings::SystemScope,QString("SixZiv"),QString("OpenScreen"),nullptr);
    LoadPath=Reg.value("filename",QString()).toString();
    if(LoadPath.isEmpty() || LoadPath.isNull())
        return 0;
    }

    CefRefPtr<App>app=new App;
    CefRefPtr<BrowserProcess> bp=new BrowserProcess(LoadPath.toStdString().c_str());
    app->SetBrowserProcessHandler(bp);

    int r = CefExecuteProcess(args, app, nullptr);

    if (r >= 0) return r;

    CefSettings settings;
    settings.background_color=0;
    settings.windowless_rendering_enabled=true;
    bool b = CefInitialize(args, settings, app, nullptr);
    if (!b) return -1;

    return a.exec();
}
