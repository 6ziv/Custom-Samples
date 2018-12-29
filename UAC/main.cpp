#include "mainwindow.h"
#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QLabel>
#include <QPixmap>
#include <Windows.h>
#include"WinUser.h"
#include "screenshot.h"
#include <QTimer>
#pragma data_seg("consent")
wchar_t CopyWrong[49] = L"Microsoft Windows (c) 2009 Microsoft Corporation";
#pragma data_seg()
#pragma comment(linker,"/SECTION:consent,RW")

Q_GUI_EXPORT QPixmap qt_pixmapFromWinHBITMAP(HBITMAP bitmap, int hbitmapFormat = 0);
int main(int argc, char *argv[])
{
    bool Secure=false;
    HKEY Key;
    DWORD RegType;
    DWORD RegLen;
    DWORD RegVal;
    if(ERROR_SUCCESS!=RegOpenKeyExW(HKEY_LOCAL_MACHINE,L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",NULL,KEY_QUERY_VALUE,&Key))
        Secure=true;
    if(!Secure){
        RegLen=1;
        if(ERROR_SUCCESS!=RegQueryValueExW(Key,L"PromptOnSecureDesktop",NULL,&RegType,NULL,&RegLen)){
            Secure=true;
            RegCloseKey(Key);
        }
    }
    if(RegLen!=sizeof(DWORD))Secure=true;
    if(RegType!=REG_DWORD)Secure=true;
    if(!Secure){
        if(ERROR_SUCCESS!=RegQueryValueExW(Key,L"PromptOnSecureDesktop",NULL,&RegType,(PBYTE)&RegVal,&RegLen)){
            Secure=true;
        }
    }
    RegCloseKey(Key);
    if(!Secure)
        if(RegVal!=0)
            Secure=true;


    HDESK WL,Old;
    HBITMAP hBmp;

    if(Secure){
        HWND Desk=GetDesktopWindow();
        RECT Rect;
        GetClientRect(GetDesktopWindow(),&Rect);
        HDC hDC=GetDC(Desk);
        HDC newDC=CreateCompatibleDC(hDC);
        hBmp=CreateCompatibleBitmap(hDC,Rect.right,Rect.bottom);
        SelectObject(newDC,hBmp);
        BitBlt(newDC,0,0,Rect.right,Rect.bottom,hDC,0,0,SRCCOPY);
    }
    if(Secure){
        Old=GetThreadDesktop(GetCurrentThreadId());
        if(Old==NULL){
            Secure=false;
        }
    }
    if(Secure){
        WL=OpenDesktopA("Winlogon",0,FALSE,GENERIC_ALL);
        if(WL==NULL){
            Secure=false;
        }
    }
    QPixmap *pImage;

    if(Secure){
        if(0==SetThreadDesktop(WL))
            Secure=false;
    }

    if(argc!=4)return 0;
    {
        int i=0;
        while(argv[1][i]!=0){
            if((argv[1][i]>'9')||(argv[1][i]<'0'))return 0;
            i++;
        }
    }
    {
        int i=0;
        while(argv[2][i]!=0){
            if((argv[2][i]>'9')||(argv[2][i]<'0'))return 0;
            i++;
        }
    }
    {
        int i=0;
           while(argv[3][i]!=0){
            if(((argv[3][i]>'9')||(argv[3][i]<'0'))&&((argv[3][i]>'f')||(argv[3][i]<'a'))&&((argv[3][i]>'F')||(argv[3][i]<'A')))
                return 0;
            i++;
        }
    }
    int pid,len;
    void *addr;
    sscanf(argv[1],"%d",&pid);
    sscanf(argv[2],"%d",&len);
    sscanf(argv[3],"%p",&addr);
    QApplication a(argc, argv);


    if(Secure){
        QPixmap pixmap=qt_pixmapFromWinHBITMAP(hBmp);

        QLabel *FullScreenBackGround=new QLabel(nullptr);
        FullScreenBackGround->showFullScreen();
        FullScreenBackGround->setPixmap(pixmap);
        FullScreenBackGround->setAttribute(Qt::WA_TransparentForMouseEvents);
        FullScreenBackGround->setWindowOpacity(0.5);
    }
    MainWindow w(pid,len,addr,Secure,Old);
    w.show();
    if(Secure){
        QTimer *timer=new QTimer(nullptr);
        timer->setInterval(5000);
        timer->connect(timer,&QTimer::timeout,timer,[=]{SwitchDesktop(WL);});
        SwitchDesktop(WL);
        timer->start();
        CloseDesktop(WL);
    }

    return a.exec();
}
