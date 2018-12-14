#ifndef RENDERWINDOW_H
#define RENDERWINDOW_H

#pragma warning(disable:4100)
#include "include/cef_app.h"
#include "include/cef_client.h"
#include "include/cef_render_handler.h"
#pragma warning(default:4100)
#include <QWidget>
#include <QLabel>
#include "browserprocess.h"

class RenderWindow :  public QWidget,public CefClient,public CefRenderHandler,public CefLoadHandler,public CefRequestHandler
{
    Q_OBJECT
public:

    bool CheckRun();
    void BroadCastMe();
    void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)override;
    void OnLoadError(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame,ErrorCode errorCode,const CefString &errorText,const CefString &failedUrl)override;
    explicit RenderWindow(configuration *pconfig);
    ~RenderWindow()override;
    HWND GetHandle();

    bool doWork();
    bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect)override;
    void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, int width, int height)override;
    void setBrowser(CefRefPtr<CefBrowser> browser);
    bool StartDragging(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDragData> drag_data,DragOperationsMask allowed_ops,int x,int y)override;
    bool IsDragging=false;

    ReturnValue OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser,CefRefPtr<CefFrame> frame,CefRefPtr<CefRequest> request,CefRefPtr<CefRequestCallback> callback)override;
    CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }
    CefRefPtr<CefRequestHandler> GetRequestHandler() override { return this; }
    CefRefPtr<CefRenderHandler> GetRenderHandler() override { return this; }
protected:
    void closeEvent(QCloseEvent *e)override;
    void mouseMoveEvent(QMouseEvent *e)override;
    void mousePressEvent(QMouseEvent *e)override;
    void mouseReleaseEvent(QMouseEvent *e)override;
private:
    CefRefPtr<CefResourceHandler> _resource=nullptr;
    CefRefPtr<CefBrowser> _browser=nullptr;
    QLabel *lab;
signals:
    void onMouseMove(int,int);
public:
    IMPLEMENT_REFCOUNTING(RenderWindow)
};

#endif // RENDERWINDOW_H
