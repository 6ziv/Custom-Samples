#ifndef BROWSERPROCESS_H
#define BROWSERPROCESS_H
#include "zipreader.h"
#include <set>
#include <QObject>
#include "ini.h"
#pragma warning(disable:4100)
#include "include/cef_browser_process_handler.h"
#include "include/cef_scheme.h"
#include "include/cef_resource_handler.h"
#include "include/cef_app.h"
#include "include/cef_zip_reader.h"
#pragma warning(default:4100)
typedef struct
{
    int Alpha;
    bool CaptureMouse;
    int Delay;
} configuration;

class BrowserProcess:public CefBrowserProcessHandler,public QObject
{
private:
    static int handler(void* user, const char* section, const char* name,const char* value);
public:
    BrowserProcess(const char *file);
    ZipReader *_zip=nullptr;
    char _file[MAX_PATH];
    void OnContextInitialized()override;
    IMPLEMENT_REFCOUNTING(BrowserProcess)
};
class SchemeHandlerFactory:public CefSchemeHandlerFactory{
public:
    SchemeHandlerFactory(ZipReader *zip){_zip=zip;}
    ZipReader *_zip=nullptr;
    CefRefPtr<CefResourceHandler> Create(CefRefPtr<CefBrowser>browser,CefRefPtr<CefFrame>frame,const CefString &scheme_name,CefRefPtr<CefRequest> request)override;
    IMPLEMENT_REFCOUNTING(CefSchemeHandlerFactory)
};
class SchemeHandler:public CefResourceHandler{
public:
    bool ProcessRequest( CefRefPtr< CefRequest > request, CefRefPtr< CefCallback > callback)override;
    bool ReadResponse( void* data_out, int bytes_to_read, int& bytes_read, CefRefPtr< CefCallback > callback)override;
    void Cancel()override;
    void GetResponseHeaders( CefRefPtr< CefResponse > response, int64& response_length, CefString& redirectUrl)override;
    int r;
    ZipReader *_zip=nullptr;
    std::istream *FileData;
    SchemeHandler(ZipReader *zip){_zip=zip;}
    IMPLEMENT_REFCOUNTING(SchemeHandler)
};
class App:public CefApp{
public:
    CefRefPtr<CefBrowserProcessHandler> _BrowserProcessHandler;
    void SetBrowserProcessHandler(const CefRefPtr<CefBrowserProcessHandler> BrowserProcessHandler){_BrowserProcessHandler=BrowserProcessHandler;}
    CefRefPtr<CefBrowserProcessHandler>GetBrowserProcessHandler()override{return _BrowserProcessHandler;}
    IMPLEMENT_REFCOUNTING(App)
};
#endif // BROWSERPROCESS_H
