#include <QApplication>
#include <Windows.h>
#include <QFileDialog>
#include <QSettings>
#include <QVideoWidget>
#include <QMediaPlayer>
#include <QMediaPlaylist>
[[noreturn]] inline void Exit(){QApplication::exit(0);exit(0);}
class MyVideoWidget:public QVideoWidget{
public:
    MyVideoWidget():QVideoWidget(nullptr){
        setMouseTracking(true);
    }
protected:
    void mousePressEvent(QMouseEvent*){
        if(!this->isFullScreen())return;
        Exit();
    }
    void mouseMoveEvent(QMouseEvent*){
        if(!this->isFullScreen())return;
        Exit();
    }
    void keyPressEvent(QKeyEvent*){
        if(!this->isFullScreen())return;
        Exit();
    }
    bool nativeEvent(const QByteArray &,void *m,long *){
        if(!this->isFullScreen())return false;

#if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
        MSG *e = *reinterpret_cast<MSG**>(m);
#else
        MSG *e = reinterpret_cast<MSG*>(m);
#endif
        if(e->message==WM_SETCURSOR)
            SetCursor(NULL);
        if((e->message==WM_ACTIVATE)&&(LOWORD(e->wParam)==WA_INACTIVE))
            Exit();
        if((e->message==WM_ACTIVATEAPP)&&(e->wParam==FALSE))
            Exit();
        if((e->message==WM_NCACTIVATE)&&(e->wParam==FALSE))
            Exit();
        if(e->message==WM_DESTROY)
            Exit();
        return false;
    }
};
MyVideoWidget *GetNew(){


    QMediaPlayer *p=new QMediaPlayer();
    MyVideoWidget *v=new MyVideoWidget();
    v->setGeometry(0,0,152,112);
    v->setWindowFlag(Qt::FramelessWindowHint,true);
    p->setVideoOutput(v);
    QSettings settings("6ziv","ScreenSaver");
    QUrl url=settings.value("Film",QUrl()).toUrl();
    if(url.isEmpty())url=QUrl("./1.wmv");
    p->setMedia(url);
    p->play();
    QWidget::connect(p,&QMediaPlayer::mediaStatusChanged,p,[=](){if(p->mediaStatus()==QMediaPlayer::InvalidMedia)Exit();});
    return v;
}

void Setting(){
    QUrl url=QFileDialog::getOpenFileUrl(nullptr,"Select a film");
    QSettings settings("6ziv","ScreenSaver");
    settings.setValue("Film",url);
    Exit();
}

void FullScreen(){
    MyVideoWidget *n=GetNew();
    n->showFullScreen();
    SetCursor(NULL);
    return;
}
void Preview(HWND handle){
    MyVideoWidget *n=GetNew();
    n->show();
    n->winId();
    RECT rect;
    GetWindowRect(handle,&rect);
    SetParent(reinterpret_cast<HWND>(n->winId()),handle);
    SetWindowLongA((HWND)n->winId(),GWL_STYLE,WS_VISIBLE+WS_CHILDWINDOW);
    SetWindowPos(reinterpret_cast<HWND>(n->winId()),\
                 HWND_TOP,\
                 0,\
                 0,\
                 rect.right-rect.left,\
                 rect.bottom-rect.top,\
                 SWP_NOZORDER|SWP_FRAMECHANGED\
);
    return;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QStringList list=QApplication::arguments();

    if(list.size()<2){
        Setting();                                  //no parameter for configure
    }

    if(list.size()>3){
        Exit();                                     //too many parameters
    }

    if(list.size()==2){
        if(list.at(1)=="/c"){
            Setting();                              //"/c" for configure
        }else
        if(list.at(1)=="/s"){
            FullScreen();                           //"/s" for show
        }else
            Exit();//Invalid!
    }
    if(list.size()==3){
        if(list.at(1)!="/p")                        //"/p <handle>" for preview
            Exit();
        HWND par=reinterpret_cast<HWND>(list.at(2).toLongLong(0));
        if(par==nullptr)
            Exit();                                 //Invalid handle
        Preview(par);
    }
    a.setQuitOnLastWindowClosed(true);
    return a.exec();
}
