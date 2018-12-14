#include "compileheader.h"
#include "browserprocess.h"
#include <QApplication>
#include <QByteArray>
#include "renderwindow.h"
#include "include/cef_render_handler.h"
#include <QTimer>


int BrowserProcess::handler(void* user, const char* section, const char* name,const char* value){
    if(strcmp(section,"Settings")!=0)return 1;
    if(strcmp(name,"CaptureMouse")==0){
        if(strcmp(value,"true")==0){reinterpret_cast<configuration*>(user)->CaptureMouse=true;}
        if(strcmp(value,"false")==0){reinterpret_cast<configuration*>(user)->CaptureMouse=false;}
    }
    if(strcmp(name,"Alpha")==0){
        int t=atoi(value);
        if(t>0 && t<=100)reinterpret_cast<configuration*>(user)->Alpha=t;
    }
    if(strcmp(name,"Delay")==0){
        int t=atoi(value);
        if(t>5 && t<=20)reinterpret_cast<configuration*>(user)->Delay=t;
    }
    return 1;
}

void BrowserProcess::OnContextInitialized(){
    configuration config;
    config.Alpha=100;config.CaptureMouse=true;config.Delay=10;
    _zip=new ZipReader();
    ZipReader::InitMap();
    if(!_zip->LoadZip(_file)){CefQuitMessageLoop();CefShutdown();}

    if(_zip->LoadFile("config.ini")){
        char tmp[256];
        QByteArray arr;
        arr.clear();
        size_t size=_zip->GetSize();
        size_t read=0;
        std::istream *stream=_zip->GetStream();
        while(read<size){
            if(size-read>256){
                stream->read(tmp,256);
                read+=256;
                arr.append(tmp,256);
            }else{
                stream->read(tmp,size-read);
                arr.append(tmp,size-read);
                read=size;
            }
        }
        _zip->CurrentFile->CloseDecompressionStream();



        char *INI=reinterpret_cast<char*>(malloc(arr.size()+1));
        memcpy(INI,arr.data(),arr.size());
        INI[arr.size()]=0;
        if(ini_parse_string(INI,handler,&config)){//On error:Use default
            config.Alpha=100;config.CaptureMouse=true;config.Delay=10;
        }
        free(INI);
    }

    CefRegisterSchemeHandlerFactory("client", "myapp", new SchemeHandlerFactory(_zip));


    CefRefPtr<RenderWindow> renderHandler;
    renderHandler = new RenderWindow(&config);

    CefRefPtr<CefBrowser> browser;
    CefWindowInfo window_info;
    CefBrowserSettings browserSettings;
    browserSettings.javascript=STATE_ENABLED;

    window_info.SetAsWindowless(reinterpret_cast<HWND>(renderHandler->winId()));
    browser = CefBrowserHost::CreateBrowserSync(window_info, renderHandler.get(), "client://myapp/index.html", browserSettings, nullptr);
    renderHandler->setBrowser(browser);

    QTimer *timer=new QTimer(nullptr);
    timer->setInterval(config.Delay);
    this->connect(timer,&QTimer::timeout,this,[=]{renderHandler->doWork();});
    timer->start();

}
BrowserProcess::BrowserProcess(const char *file)
{
    memcpy(_file,file,1+(strlen(file)<(MAX_PATH-1)?strlen(file):(MAX_PATH-1)));
}
CefRefPtr<CefResourceHandler> SchemeHandlerFactory::Create(CefRefPtr<CefBrowser>browser,CefRefPtr<CefFrame>frame,const CefString &scheme_name,CefRefPtr<CefRequest> request){
    UNREFERENCED_PARAMETER(browser);
    UNREFERENCED_PARAMETER(frame);
    UNREFERENCED_PARAMETER(scheme_name);
    UNREFERENCED_PARAMETER(request);
    return new SchemeHandler(_zip);
}
void SchemeHandler::Cancel(){}
bool SchemeHandler::ProcessRequest( CefRefPtr< CefRequest > request, CefRefPtr< CefCallback > callback){
    if(!_zip->LoadFile(request->GetURL().ToString().replace(0,15,"").c_str())){
        callback->Cancel();
        return false;
    }
    FileData=_zip->GetStream();
    callback->Continue();
    return true;
}
bool SchemeHandler::ReadResponse(void* data_out,int bytes_to_read,int& bytes_read,CefRefPtr< CefCallback > callback){
    char c;
    FileData->read(reinterpret_cast<char*>(data_out),bytes_to_read);
    bytes_read=int(FileData->gcount());
    if(bytes_read==0)return false;
    if(FileData->fail())return false;
    FileData->get(c);
    if(!FileData->eof()){
        FileData->unget();
        callback->Continue();
    }else{
        _zip->CurrentFile->CloseDecompressionStream();
    }
    return true;
}
void SchemeHandler::GetResponseHeaders( CefRefPtr< CefResponse > response, int64& response_length, CefString& redirectUrl){
    response_length=int64(_zip->GetSize());
    response->SetStatus(200);
    char *MIME;
    _zip->getMIMEtype(&MIME);
    response->SetMimeType(MIME);
    free(MIME);
    redirectUrl.clear();
}
