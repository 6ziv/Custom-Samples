#include "explorer.h"
#include "eventlistener.h"
#include<QApplication>
#include<QList>
#include<QMap>
#include<QMenu>
#include<QSystemTrayIcon>
#include<QSettings>
#include"desktopitem.h"
#include"system.h"
#include"settings.h"
namespace Explorer{
    namespace {
        QSettings *settings;

        HMODULE hDll;
        STARTHOOK StartHook;
        FINDWINDOWS FindWindows;
        STOPHOOK StopHook;
        HWND hWorkerW,hDesktop,hIcons,hWorkerX;
        QList<DesktopItem*> list;
        QList<QString> namelist;
        QMap<QScreen*,DesktopItem*>map;
        QSystemTrayIcon *icon;
        QList<QUrl>urls;
        bool inited=false;
        bool initDll(){
            list.clear();
            map.clear();
            hDll=LoadLibraryA("OpenDesktop.dll");
            if(hDll==nullptr)return false;
            FARPROC hFun1 = GetProcAddress(hDll, "StartHook");
            FARPROC hFun2 = GetProcAddress(hDll, "FindWindows");
            FARPROC hFun3 = GetProcAddress(hDll, "StopHook");
            StartHook=reinterpret_cast<STARTHOOK>(hFun1);
            FindWindows=reinterpret_cast<FINDWINDOWS>(hFun2);
            StopHook=reinterpret_cast<STOPHOOK>(hFun3);
            if((StartHook==nullptr)||(FindWindows==nullptr)||(StopHook==nullptr))return false;
            return true;
        }
    }

    bool init(){
        if(!inited){
            settings=new QSettings("6ziv","Wallpaper",nullptr);
            inited=true;
            if(!initDll()){
                MessageBoxA(nullptr,"Error when loading library.","Fatal Error!",MB_OK);
                return false;
            }
            icon=new QSystemTrayIcon();
            QMenu *menu=new QMenu(nullptr);
            QAction *Show=new QAction();
            Show->setText("Settings");
            Show->setCheckable(false);
            QObject::connect(Show,&QAction::triggered,menu,[=](){Settings::ptr_Settings=new Settings(nullptr);});
            menu->addAction(Show);
            QAction *Quit=new QAction();
            Quit->setText("Quit");
            Quit->setCheckable(false);
            QObject::connect(Quit,&QAction::triggered,menu,[=](){
                die();
            });
            menu->addAction(Quit);

            icon->setContextMenu(menu);
            QIcon ico=QIcon(":/icon/default.ico");
            icon->setIcon(ico);
            icon->show();
        }
        for(QList<DesktopItem*>::iterator iter=list.begin();iter!=list.end();iter++){
            try {
                delete *iter;
            } catch (...) {
                qt_noop();
            }
        }
        list.clear();
        namelist.clear();
        map.clear();
        BOOL t=FindWindows(&hDesktop,&hWorkerW,&hIcons);
        if(!t)
            return true;
        hWorkerX=FindWindowExA(nullptr,hWorkerW,"WorkerW",nullptr);
        if(hWorkerX==nullptr)
            error("Unexpected Window Structure. Can not understand.");

        StartHook(hIcons,hWorkerW,Listener);
        QList<QScreen*> lScreen=QApplication::screens();

        for(QList<QScreen*>::iterator iter=lScreen.begin();iter!=lScreen.end();iter++){    
            DesktopItem *item=new DesktopItem(*iter);
            list.append(item);
            namelist.append((*iter)->name());
            map.insert(*iter,item);
            SetParent(reinterpret_cast<HWND>(item->winId()),hWorkerX);
            item->Goto(Settings::GetUrl((*iter)->name()));
            item->SetCreate(Settings::GetCreate((*iter)->name()));
        }
        ShowWindow(hWorkerX,SW_SHOW);
        EnableWindow(hWorkerX,TRUE);
        return true;
    }
    void ShowIcons(){
        ShowWindow(hWorkerW,SW_SHOW);
    }
    void DispatchMouseMessage(const QPoint &pt,const WPARAM wParam){
        QMouseEvent *m;
        QScreen *Screen=QApplication::screenAt(pt);
        DesktopItem *item=*map.find(Screen);
        QPointF relative=pt-item->geometry().topLeft();
        switch(wParam){
            case WM_MOUSEMOVE:
                m=new QMouseEvent(QEvent::MouseMove,relative,Qt::NoButton,Qt::NoButton,Qt::NoModifier);
                break;
            case WM_LBUTTONDOWN:
                m=new QMouseEvent(QEvent::MouseButtonPress,relative,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
                break;
            case WM_LBUTTONUP:
                m=new QMouseEvent(QEvent::MouseButtonRelease,relative,Qt::LeftButton,Qt::NoButton,Qt::NoModifier);
                break;
            case WM_LBUTTONDBLCLK:
                m=new QMouseEvent(QEvent::MouseButtonDblClick,relative,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
                break;
            default:
                return;
        }
        item->Dispatch(m);
    }
    void Exit(){
        NOFAULTBEG
        for(QList<DesktopItem*>::iterator iter=list.begin();iter!=list.end();iter++){
            try {
                (*iter)->close();
            } catch (...) {
                qt_noop();
            }
        }
        list.clear();
        NOFAULT
        map.clear();
        NOFAULT
        StopHook();
        NOFAULT
        FreeLibrary(hDll);
        NOFAULT
        if(icon!=nullptr){
            icon->hide();
            delete icon;
        }
        NOFAULT
        ShowIcons();
        NOFAULT
        if(hWorkerX!=nullptr){
            ShowWindow(hWorkerX,SW_HIDE);
            EnableWindow(hWorkerX,FALSE);
        }
        NOFAULT
            ShowWindow(hWorkerW,SW_SHOW);
        NOFAULTFIN
    }

    int getSize(){
        return namelist.size();
    }
    QString getname(int id){
        return namelist.at(id);
    }
    QUrl geturl(int id){
        return list.at(id)->GetUrl();
    }
    void seturl(int id,QUrl url){
        list.at(id)->Goto(url);
    }
    bool getCreatable(int id){
        return list.at(id)->GetCreate();
    }
    void setCreatable(int id,bool on){
        list.at(id)->SetCreate(on);
    }

}

