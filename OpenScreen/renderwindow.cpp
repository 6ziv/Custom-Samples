#include "compileheader.h"

#include "renderwindow.h"
#include <QBitmap>
#include <QMouseEvent>
#include <QScreen>
#include <QUrl>
#include <QApplication>
#include "browserprocess.h"

void RenderWindow::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode){
    this->show();
    lab->show();
}

//Share this window, for being controlled
void RenderWindow::BroadCastMe(){
    HANDLE hfile = CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(HWND),TEXT("OpenScreen_59198"));
    if(nullptr==hfile)
        return;
    PVOID pview = nullptr;
    pview = MapViewOfFile(hfile,FILE_MAP_ALL_ACCESS,0,0,sizeof(HWND));
    if (nullptr==pview)
        return;
    HWND window=(HWND)this->winId();
    memcpy_s(pview,sizeof(HWND),&window,sizeof(HWND));
    return;
}

//Only Run Once!
bool RenderWindow::CheckRun(){
    if(nullptr==CreateMutexA(nullptr,true,"OpenScreen"))
        return false;
    if(ERROR_ALREADY_EXISTS==GetLastError())
        return false;
    return true;
}


RenderWindow::RenderWindow(configuration *pconfig) : QWidget(nullptr)
{
    if(!CheckRun())
        this->close();
    BroadCastMe();

    this->setWindowOpacity(((double)pconfig->Alpha)/100);
    QRect geo=QApplication::screens()[0]->geometry();
    this->setGeometry(geo);
    this->setWindowFlag(Qt::FramelessWindowHint,true);
    this->setWindowFlag(Qt::WindowStaysOnTopHint,true);
    this->setWindowFlag(Qt::Tool,true);
    lab=new QLabel(this);
    lab->setGeometry(geo);
    if(pconfig->CaptureMouse)
        this->setMouseTracking(true);
    else{
        this->setAttribute(Qt::WA_TransparentForMouseEvents,true);
        SetWindowLong((HWND)winId(), GWL_EXSTYLE, GetWindowLong((HWND)winId(), GWL_EXSTYLE)|WS_EX_TRANSPARENT | WS_EX_LAYERED);
        lab->setAttribute(Qt::WA_TransparentForMouseEvents,true);
        SetWindowLong((HWND)lab->winId(), GWL_EXSTYLE, GetWindowLong((HWND)lab->winId(), GWL_EXSTYLE)|WS_EX_TRANSPARENT | WS_EX_LAYERED);

    }
}

RenderWindow::~RenderWindow(){
    delete lab;
    CefQuitMessageLoop();
    CefShutdown();
}
cef_return_value_t RenderWindow::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame,CefRefPtr<CefRequest> request,CefRefPtr<CefRequestCallback> callback){
    QUrl url(QString::fromStdString(request->GetURL().ToString()));
    if(url.scheme()!="client" || url.host()!="myapp")
        return RV_CANCEL;
    return RV_CONTINUE;
}
HWND RenderWindow::GetHandle(){
    return reinterpret_cast<HWND>(this->winId());
}
bool RenderWindow::StartDragging(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDragData> drag_data,DragOperationsMask allowed_ops,int x,int y){
    UNREFERENCED_PARAMETER(browser);
    UNREFERENCED_PARAMETER(allowed_ops);
    CefMouseEvent mouse_event;
    mouse_event.x=x;
    mouse_event.y=y;
    mouse_event.modifiers=EVENTFLAG_LEFT_MOUSE_BUTTON;
    drag_data->ResetFileContents();
    _browser->GetHost()->DragTargetDragEnter(drag_data,mouse_event,DRAG_OPERATION_EVERY);
    IsDragging=true;
    return true;
}
void RenderWindow::OnLoadError(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame,ErrorCode errorCode,const CefString &errorText,const CefString &failedUrl){
    UNREFERENCED_PARAMETER(browser);
    UNREFERENCED_PARAMETER(frame);
    UNREFERENCED_PARAMETER(errorCode);
    UNREFERENCED_PARAMETER(errorText);
    UNREFERENCED_PARAMETER(failedUrl);

    CefQuitMessageLoop();
    CefShutdown();
}

void RenderWindow::mousePressEvent(QMouseEvent *e){
    if(!_browser)return;
    if(e->button()!=Qt::LeftButton)return;
    if(IsDragging)return;
    CefMouseEvent m;
    m.x=e->x();m.y=e->y();m.modifiers=EVENTFLAG_LEFT_MOUSE_BUTTON;
    SetCapture(reinterpret_cast<HWND>(this->winId()));
    _browser->GetHost()->SendMouseClickEvent(m,MBT_LEFT,false,1);

}
void RenderWindow::mouseReleaseEvent(QMouseEvent *e){
    if(!_browser)return;

    CefMouseEvent m;
    m.x=e->x();m.y=e->y();m.modifiers=EVENTFLAG_NONE;
    if(e->button()==Qt::LeftButton && IsDragging){
        IsDragging=false;
        _browser->GetHost()->DragTargetDrop(m);
        _browser->GetHost()->DragSourceSystemDragEnded();
        ReleaseCapture();
    }else{
        _browser->GetHost()->SendMouseClickEvent(m,MBT_LEFT,true,1);
    }
}

void RenderWindow::mouseMoveEvent(QMouseEvent *e){
    if(!_browser)return;
    if(!e->buttons().testFlag(Qt::LeftButton)){
        CefMouseEvent m;
        m.x=e->x();m.y=e->y();m.modifiers=EVENTFLAG_NONE;
        _browser->GetHost()->SendMouseMoveEvent(m,false);
    }
    CefMouseEvent m;
    m.x=e->x();m.y=e->y();m.modifiers=EVENTFLAG_LEFT_MOUSE_BUTTON;
    if(IsDragging){
        _browser->GetHost()->DragTargetDragOver(m,DRAG_OPERATION_EVERY);
    }else{
        _browser->GetHost()->SendMouseMoveEvent(m,false);
    }
}


bool RenderWindow::doWork()
{
    CefDoMessageLoopWork();
    return true;
}
bool RenderWindow::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect)
{
    UNREFERENCED_PARAMETER(browser);
    rect = CefRect(0, 0, lab->width(), lab->height());
    return true;
}
void RenderWindow::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, int width, int height)
{
    UNREFERENCED_PARAMETER(browser);
    UNREFERENCED_PARAMETER(dirtyRects);
    UNREFERENCED_PARAMETER(type);
    QImage img(reinterpret_cast<const uchar*>(buffer),width,height,QImage::Format_ARGB32);
    QBitmap mask=QPixmap::fromImage(img.createAlphaMask());
    QPixmap pix;
    pix=QPixmap::fromImage(img);

    lab->setPixmap(pix);
    lab->setMask(mask);
    this->setMask(mask);
//    this->window()->setAttribute(Qt::WA_TransparentForMouseEvents,true);
 //   SetWindowLong((HWND)winId(), GWL_EXSTYLE, GetWindowLong((HWND)winId(), GWL_EXSTYLE) | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOPMOST);
 //   SetWindowLong((HWND)lab->winId(), GWL_EXSTYLE, GetWindowLong((HWND)lab->winId(), GWL_EXSTYLE) | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOPMOST);

}
void RenderWindow::setBrowser(CefRefPtr<CefBrowser> browser){
    _browser=browser;
}
void RenderWindow::closeEvent(QCloseEvent *e){
    CefQuitMessageLoop();
    CefShutdown();
    e->accept();
    QApplication::exit(0);
}
