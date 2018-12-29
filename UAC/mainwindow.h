#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QFont>
#include "Windows.h"
class PushButton:public QPushButton
{
    Q_OBJECT
public:
    PushButton(QWidget * parent=0);
    ~PushButton();
private:
    QFont myFont;
    QFont myFontBld;
protected:
    //virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void enterEvent(QEvent *e);
    virtual void leaveEvent(QEvent *e);
signals:
    void MouseEnter();
    void MouseLeave();
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(int pid,int len, void* addr,bool IsSec,HDESK OldDesk);
    ~MainWindow();
private:
    HANDLE hProcess;
    void *Address;
    HANDLE OldForeign;
protected:
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
private:
    int OffsetX;
    int OffsetY;
    bool isDragging;
    bool SecureMode;
    HDESK OldDesktop;
    void EXIT(DWORD retCode);
};

#endif // MAINWINDOW_H
