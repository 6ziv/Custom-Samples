#include "glwidget.hpp"
#include "apputils.hpp"
#include "app.hpp"
#include "model.hpp"
#include "hotkey.h"
#include <Shlwapi.h>
#include <QTimer>
#include "qdebug.h"
#include <QMouseEvent>
#include <QBitmap>
#include <QLabel>
#include <QApplication>
#include <Windows.h>
#include <QMimeData>
#include "pluginloader.hpp"
#include "openglhelper.hpp"
#include "config.hpp"
GLWidget::GLWidget(int w,int h):
    QOpenGLWidget(nullptr)
{
    this->setWindowFlag(Qt::WindowType::MSWindowsOwnDC,false);
    this->setWindowFlag(Qt::FramelessWindowHint);
    this->setWindowFlag(Qt::Tool);
    this->setWindowFlag(Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setAttribute(Qt::WA_QuitOnClose);
    this->setAcceptDrops(true);
    this->show();
    ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
    //ChangeWindowMessageFilter(WM_COPYDATA, MSGFLT_ADD);
    //ChangeWindowMessageFilter(0x49, MSGFLT_ADD);/* WM_COPYGLOBALDATA */
    HWND hWnd=(HWND)(this->winId());
    SetWindowLong(hWnd,GWL_EXSTYLE,GetWindowLongW(hWnd,GWL_EXSTYLE)|WS_EX_TRANSPARENT|WS_EX_NOACTIVATE);
    this->setGeometry(0,0,w,h);
    this->makeCurrent();
    OpenGLHelper::get()->initializeOpenGLFunctions();
    OpenGLHelper::get()->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    OpenGLHelper::get()->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    OpenGLHelper::get()->glEnable(GL_BLEND);
    OpenGLHelper::get()->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    menu.connect(&menu,&Menu::selected,&menu,[this](int id){
        if(id==0){
            bool muted=MusicPlayer::GetInstance()->isMuted();
            MusicPlayer::GetInstance()->setmute(!muted);
            return;
        }
        if(id==1){
            DWORD retSize=0;
            AssocQueryStringW(NULL,ASSOCSTR_COMMAND,L".txt",L"open",NULL,&retSize);
            retSize+=1;
            wchar_t *ret=new wchar_t[retSize];
            memset(ret,0,retSize*sizeof(wchar_t));
            AssocQueryStringW(NULL,ASSOCSTR_COMMAND,L".txt",L"open",ret,&retSize);
            QString str=QString::fromStdWString(ret);
            str.replace("%1","settings.conf");
            std::wstring ws=str.toStdWString();
            STARTUPINFOW si;
            PROCESS_INFORMATION pi;
            memset(&si,0,sizeof(si));
            memset(&pi,0,sizeof(pi));
            si.cb=sizeof(si);
            CreateProcessW(NULL,ws.data(),NULL,NULL,FALSE,NULL,NULL,NULL,&si,&pi);
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        }
        if(id==2){
            std::thread thread([](){
                PluginLoader::unload.lock();
                PluginLoader::unloadall();
                ExitProcess(1223);
            });
            thread.detach();
        }
    });
}
void GLWidget::initializeGL(){
    //OpenGLHelper::get()->initializeOpenGLFunctions();
}
void GLWidget::resizeGL(int width, int height){
}
void GLWidget::testMouse(){
    HWND hWnd=(HWND)(this->winId());
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(hWnd,&pt);
    long double x,y;
    x=pt.x*2.0/width()-1.0;
    y=1.0-pt.y*2.0/height();
    bool res=App::GetInstance()->GetModel()->TestMouse(x,y);
    if(res)
        SetWindowLong(hWnd,GWL_EXSTYLE,GetWindowLongW(hWnd,GWL_EXSTYLE)&(~WS_EX_TRANSPARENT));
    else
        SetWindowLong(hWnd,GWL_EXSTYLE,GetWindowLongW(hWnd,GWL_EXSTYLE)|WS_EX_TRANSPARENT);
    DragAcceptFiles(hWnd,true);
}
void GLWidget::mousePressEvent(QMouseEvent*e){
    if(e->button()==Qt::RightButton){
        //if(menu.IsShown())return;
        auto pos=App::GetInstance()->GetModel()->getpos();
        int cx=(pos.first+1.0)*this->width()/2;
        int cy=(1.0-pos.second)*this->height()/2;
        if(cx>100){
            menu.moveto(QPoint(cx-150,cy-300));
            //move a little further to avoid being blocked by the model
        }
        else{
            menu.moveto(QPoint(cx+100,cy-300));
            //move to right if left is unavilable
        }
        if(!menu.IsShown())
            menu.AnimatedShow();
        return;
    }
    if(e->button()==Qt::LeftButton){
        if(menu.IsShown())
            menu.AnimatedHide();
        is_dragging=true;
        oldx=e->x();
        oldy=e->y();
    }
}
void GLWidget::mouseReleaseEvent(QMouseEvent*e){
    is_dragging=false;
}
void GLWidget::mouseMoveEvent(QMouseEvent*e){
    if(!e->buttons().testFlag(Qt::MouseButton::LeftButton)){
        mouseReleaseEvent(e);
        return;
    }
    if(!is_dragging)mousePressEvent(e);
    double x,y;
    x=(e->x()-oldx)*2.0/width();
    y=-(e->y()-oldy)*2.0/height();
    oldx=e->x();oldy=e->y();
    App::GetInstance()->GetModel()->move(x,y);
}
void GLWidget::paintGL(){
    App::GetInstance()->Run();
}
void GLWidget::clear(){
    OpenGLHelper::get()->glViewport(0, 0, width(), height());
    OpenGLHelper::get()->glClearColor(0,0,0,0.0);
    OpenGLHelper::get()->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    OpenGLHelper::get()->glClearDepthf(1.0);
}
bool GLWidget::nativeEvent(const QByteArray &b,void*m,long*r){
    MSG* msg=reinterpret_cast<MSG*>(m);
    if(msg->message==WM_MOUSEACTIVATE){
        *r=MA_NOACTIVATE;
        return true;
    }
    if(msg->message==WM_QUERYENDSESSION){
        std::atomic_bool lock=true;
        std::thread thread([&lock](){
            PluginLoader::unload.lock();
            PluginLoader::unloadall();
            lock=false;
        });
        while(lock)QApplication::instance()->processEvents();
        *r=TRUE;
        return true;
    }
    if(msg->message==WM_ACTIVATE){
        if(LOWORD(msg->wParam)){
            this->clearFocus();
            *r=1;
            return true;
        }
    }
    if(msg->message==WM_HOTKEY){
        int id=static_cast<int>(msg->wParam);
        HotKey::Run(id);
    }

    if(msg->message==WM_DROPFILES){


        UINT count=DragQueryFileW(reinterpret_cast<HDROP>(msg->wParam),0xFFFFFFFF,NULL,0);
        if(count==0)return true;
        if(count>1){
            wchar_t **files=reinterpret_cast<wchar_t**>(malloc(sizeof(wchar_t*)*count));
            for(UINT i=0;i<count;i++){
                UINT size=DragQueryFileW(reinterpret_cast<HDROP>(msg->wParam),i,NULL,0);
                files[i]=reinterpret_cast<wchar_t*>(malloc(sizeof(wchar_t)*(size+1)));
                DragQueryFileW(reinterpret_cast<HDROP>(msg->wParam),i,files[i],size+1);
                files[i][size]=L'\0';
            }
            PluginLoader::ondropfiles(count,files);
            return true;
        }
        UINT size=DragQueryFileW(reinterpret_cast<HDROP>(msg->wParam),0,NULL,0);
        wchar_t* filename=reinterpret_cast<wchar_t*>(malloc(sizeof(wchar_t)*(size+1)));
        DragQueryFileW(reinterpret_cast<HDROP>(msg->wParam),0,filename,size+1);
        filename[size]=L'\0';

        if(GetFileAttributesW(filename)&FILE_ATTRIBUTE_DIRECTORY){
            PluginLoader::ondropdir(filename);
        }else{
            PluginLoader::ondropfile(filename);
        }
        return true;
    }
    return QOpenGLWidget::nativeEvent(b,m,r);
}
