#ifndef EXPLORER_H
#define EXPLORER_H
#include<Windows.h>
#include<QPoint>
#include<QScreen>
#include"desktopitem.h"
typedef BOOL (*STARTHOOK)(HWND,HWND,HWND);
typedef BOOL (*FINDWINDOWS)(HWND*,HWND*,HWND*);
typedef BOOL (*STOPHOOK)();

namespace Explorer{
    bool init();
    void ShowIcons();
    void DispatchMouseMessage(const QPoint &pt,const WPARAM wParam);
    void Exit();
    int getSize();
    QUrl geturl(int id);
    bool getCreatable(int id);
    QString getname(int id);
    void seturl(int id,QUrl url);
    void setCreatable(int id,bool on);
}
#endif // EXPLORER_H
