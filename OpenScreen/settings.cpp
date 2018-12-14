#include "compileheader.h"

#include "settings.h"
#include <QScreen>
#include <QStyle>
#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include "Windows.h"
#include "runner.h"
#include <QPushButton>
#include <QFileDialog>
#include <QTimer>
#include <QIcon>
#include "Shlwapi.h"
#include "TlHelp32.h"

Settings::Settings() :QWidget(nullptr)
{
    if(!IsAdmin()){
        wchar_t path[MAX_PATH+1];
        GetModuleFileNameW(nullptr,path,MAX_PATH);
        ShellExecuteW(nullptr,L"runas",path,L"",L"",SW_SHOWNORMAL);
        QApplication::exit(0);
        exit(0);
    }
    Reg=new QSettings(QSettings::Scope::SystemScope,QString("SixZiv"),QString("OpenScreen"),nullptr);
    this->setFixedWidth(QApplication::screens()[0]->size().width()/4);
    this->setFixedHeight(QApplication::screens()[0]->size().height()/2);
    this->setGeometry(QApplication::screens()[0]->size().width()*3/8,QApplication::screens()[0]->size().height()/4,QApplication::screens()[0]->size().width()/4,QApplication::screens()[0]->size().height()/2);
    this->show();
    btnRun=new QPushButton(this);
    btnRun->setGeometry(this->width()/16,this->height()*6/8,this->width()*3/8,this->height()/16);
    btnRun->setText("Start");
    btnRun->show();
    connect(btnRun,&QPushButton::clicked,this,&Settings::Start);

    btnStop=new QPushButton(this);
    btnStop->setGeometry(this->width()*9/16,this->height()*6/8,this->width()*3/8,this->height()/16);
    btnStop->setText("Stop");
    btnStop->show();
    connect(btnStop,&QPushButton::clicked,this,&Settings::Stop);

    btnKill=new QPushButton(this);
    btnKill->setGeometry(this->width()/16,this->height()*7/8,this->width()*7/8,this->height()/16);
    btnKill->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning));
    btnKill->setText("Force Terminate");
    btnKill->show();
    connect(btnKill,&QPushButton::clicked,this,[=](){

        wchar_t MyPath[MAX_PATH+1];
        if(0==GetModuleFileNameW(nullptr,MyPath,MAX_PATH+1))return;
        if(PathIsLFNFileSpecW(MyPath)){
            wchar_t nPath[MAX_PATH+1];
            if(0==GetShortPathNameW(MyPath,nPath,MAX_PATH+1))return;
            wcscpy_s(MyPath,MAX_PATH+1,nPath);
        }
        DWORD MyPid=GetCurrentProcessId();

        HANDLE hwnd=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
        if(hwnd==nullptr)return;
        PROCESSENTRY32 tmp;
        BOOL Success;
        tmp.dwSize=sizeof(tmp);
        Success=Process32First(hwnd,&tmp);

        for(;Success;Success=Process32Next(hwnd,&tmp)){
            //printf("%S",MyPath);fflush(stdout);
            HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_TERMINATE,false,tmp.th32ProcessID);
            if(hProcess==nullptr)continue;
            DWORD Size=MAX_PATH;
            wchar_t Path[MAX_PATH+1];

            if(!QueryFullProcessImageNameW(hProcess,0,Path,&Size))continue;
            if(PathIsLFNFileSpecW(Path)){
                wchar_t nPath[MAX_PATH+1];
                if(0==GetShortPathNameW(Path,nPath,MAX_PATH+1))continue;
                wcscpy_s(Path,MAX_PATH+1,nPath);
            }
            if(_wcsicmp(Path,MyPath)==0){
                if(MyPid!=tmp.th32ProcessID)TerminateProcess(hProcess,0);
            }
            tmp.dwSize=sizeof(tmp);

        }
    });

    QLabel *title1=new QLabel(this);
    title1->setGeometry(this->width()/16,this->height()/16,this->width()*3/16,this->height()/16);
    title1->setText("Theme");
    title1->show();

    FileSelector=new QTextEdit(this);
    FileSelector->setEnabled(false);
    FileSelector->setGeometry(this->width()/4,this->height()/16,this->width()/2,this->height()/16);
    //FileSelector->setText("MyTheme");
    FileSelector->show();

    QPushButton *Change=new QPushButton(this);
    Change->setGeometry(this->width()*13/16,this->height()/16,this->width()/8,this->height()/16);
    Change->setText("...");
    Change->show();
    connect(Change,&QPushButton::clicked,this,[=](){
        QString str=QFileDialog::getOpenFileName(this,"Select A File");
        if(str.isEmpty()||str.isNull())return;
        if(!QFile::exists(str))return;
        QFile file(str);
        if(!file.open(QIODevice::ReadOnly))return;
        DWORD head;
        if(4!=file.read(reinterpret_cast<char*>(&head),4)){
            file.close();
            return;
        }
        file.close();
        if(head!=0x04034b50)return;

        Reg->setValue("filename",str);
        Reg->sync();

    });

//    HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Run
    QLabel *title2=new QLabel(this);
    title2->setGeometry(this->width()/16,this->height()*3/16,this->width()*3/8,this->height()/16);
    title2->setText("Load On Startup");
    title2->show();

    RegStart=new QSettings("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",QSettings::NativeFormat,this);
    StartWithSystem=new QCheckBox(this);
    StartWithSystem->setGeometry(this->width()/2,this->height()*3/16,this->width()/8,this->height()/16);
    StartWithSystem->show();
    connect(StartWithSystem,&QCheckBox::toggled,this,[=](bool isChecked){
        if(isChecked){
            RegStart->setValue("OpenScreen",QCoreApplication::applicationFilePath());
        }else{
            RegStart->remove("OpenScreen");
        }
        RegStart->sync();
    });

    QLabel *thanks=new QLabel(this);
    thanks->setGeometry(this->width()/16,this->height()*5/16,this->width()*7/8,this->height()/16);
    thanks->setText("This program includes codes/binaries from:");
    thanks->show();

    QByteArray ar=QByteArray::fromBase64("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");
    QImage img;
    img=QImage::fromData(ar,"JPG");
    QPixmap QtIco=QPixmap::fromImage(img);
    QtIco=QtIco.scaledToHeight(this->height()/16);

    Label *Qt=new Label(this);
    QtIco=QtIco.scaledToHeight(this->height()/16);
    Qt->setPixmap(QtIco);
    Qt->setGeometry(this->width()/16,this->height()*7/16,QtIco.width(),QtIco.height());
    Qt->show();
    Qt->setToolTip("LGPL v3 License");
    connect(Qt,&Label::pressed,this,[=](){QApplication::aboutQt();});

    QByteArray arr=QByteArray::fromBase64("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");
    QImage image;
    image=QImage::fromData(arr,"JPG");
    QPixmap p=QPixmap::fromImage(image);
    p=p.scaledToHeight(this->height()/16);
    Label *Cef=new Label(this);
    Cef->setPixmap(p);
    Cef->setGeometry(this->width()*5/16,this->height()*7/16,p.width(),p.height());
    Cef->setToolTip("BSD License");
    Cef->show();
    connect(Cef,&Label::pressed,this,
        [=](){
            QMessageBox(QMessageBox::Icon::Information,"About CEF",
                    QString::fromWCharArray(

L"CEF is a BSD-licensed open source project founded by Marshall Greenblatt in 2008 and based on the Google Chromium project. Unlike the Chromium project itself, which focuses mainly on Google Chrome application development, CEF focuses on facilitating embedded browser use cases in third-party applications. CEF insulates the user from the underlying Chromium and Blink code complexity by offering production-quality stable APIs, release branches tracking specific Chromium releases, and binary distributions. Most features in CEF have default implementations that provide rich functionality while requiring little or no integration work from the user. There are currently over 100 million installed instances of CEF around the world embedded in products from a wide range of companies and industries. A partial list of companies and products using CEF is available on the CEF Wikipedia page. Some use cases for CEF include:\n\
\n\
Embedding an HTML5-compliant Web browser control in an existing native application.\n\
Creating a light-weight native \"shell\" application that hosts a user interface developed primarily using Web technologies.\n\
Rendering Web content \"off-screen\" in applications that have their own custom drawing frameworks.\n\
Acting as a host for automated testing of existing Web properties and applications.\n\
CEF supports a wide range of programming languages and operating systems and can be easily integrated into both new and existing applications. It was designed from the ground up with both performance and ease of use in mind. The base framework includes C and C++ programming interfaces exposed via native libraries that insulate the host application from Chromium and Blink implementation details. It provides close integration between the browser and the host application including support for custom plugins, protocols, JavaScript objects and JavaScript extensions. The host application can optionally control resource loading, navigation, context menus, printing and more, while taking advantage of the same performance and HTML5 technologies available in the Google Chrome Web browser.\n"),

                        QMessageBox::StandardButton::Ok,this).exec();
        }
    );

    Label *Inih=new Label(this);
    Inih->setText("inih");
    Inih->setGeometry(this->width()*9/16,this->height()*7/16,this->width()/8,this->height()/16);
    Inih->show();
    QFont f=Inih->font();
    f.setPixelSize(this->height()/24);
    Inih->setFont(f);
    Inih->setToolTip("New BSD License");
    connect(Inih,&Label::pressed,this,
        [=](){
            QMessageBox(QMessageBox::Icon::Information,"About Inih",
                    QString::fromWCharArray(

L"inih is a simple .INI file parser written in C, released under the New BSD license (see LICENSE). The original project home page is localted at:\n\
\n\
http://code.google.com/p/inih/"),
                        QMessageBox::StandardButton::Ok,this).exec();
        }
    );


    Label *Zip=new Label(this);
    Zip->setText("ZipLib");
    Zip->setGeometry(this->width()*12/16,this->height()*7/16,this->width()*3/16,this->height()/16);
    Zip->show();
    QFont ff=Zip->font();
    ff.setPixelSize(this->height()/24);
    Zip->setFont(ff);
    Zip->setToolTip("Custom License By W.Benny");
    connect(Zip,&Label::pressed,this,
        [=](){
            QMessageBox(QMessageBox::Icon::Information,"About ZipLib",
                    QString::fromWCharArray(

L"ZipLib is a lightweight C++11 library for working with ZIP archives with ease. The very useful feature of ZipLib is that it is built around STL streams, no additional dependencies (boost) are required.\n\
\n\
Features\n\
    Compression/decompresion using the DEFLATE algorithm\n\
    Storing/fetching files without compression\n\
    Adding, editing and removing files and directories in the archive\n\
    Support of the data descriptor\n\
    Data flow is driven via STL streams, therefore there is no need to load huge amount of data into the memory\n\
    Support of PKWare encryption\n\
    Support of entry comments and archive comments\n\
    Contains very usable stream classes in ZipLib/streams like memory stream, substream or teestream\n\
    Contains functions for easy (de)serialization (ZipLib/streams/serialization.h)\n\
    Project is using only C++11 standard libraries, therefore there IS NO dependency on 3rd party libraries (except of included zlib, which uses standard libraries of C afterall :))\n\
    Built around STL smart pointers\n\
    Support of Windows and Linux\n\
"),
                        QMessageBox::StandardButton::Ok,this).exec();
        }
    );

    QLabel *label=new QLabel(this);
    label->setText("Also with reference to (but rewrite):");
    label->setGeometry(this->width()/16,this->height()*9/16,this->width()*7/8,this->height()/16);
    label->show();
    Label *PrivExec=new Label(this);
    QFont fff=Zip->font();
    fff.setPixelSize(this->height()/24);
    PrivExec->setFont(fff);

    PrivExec->setText("Privexec");
    PrivExec->setGeometry(this->width()/16,this->height()*10/16,this->width()*7/8,this->height()/16);
    PrivExec->setToolTip("MIT License");
    PrivExec->show();
    connect(PrivExec,&Label::pressed,this,
            [=](){
                QMessageBox(QMessageBox::Icon::Information,"About Privexec",
                        QString::fromWCharArray(
L"Privexec\n\
\n\
    Run the program with the specified permission level"),
                            QMessageBox::StandardButton::Ok,this).exec();
            }
            );

    QTimer *timer=new QTimer();
    timer->setInterval(1000);
    connect(timer,&QTimer::timeout,this,&Settings::Refresh);
    timer->start();
}
void Settings::Refresh(){
    filename=this->GetFile();
    wchar_t tmp[MAX_PATH+1];
    memset(tmp,0,sizeof(tmp));
    wcscpy_s(tmp,MAX_PATH+1,filename.toStdWString().c_str());
    PathStripPathW(tmp);
    FileSelector->setText(QString::fromWCharArray(tmp,wcslen(tmp)));
    IsRunning=this->CheckRunning();
    btnRun->setDisabled(IsRunning);
    btnStop->setEnabled(IsRunning);

}
QString Settings::GetFile(){
    QString LoadPath;
    {
    Reg->sync();
    LoadPath=Reg->value("filename",QString()).toString();
    if(LoadPath.isEmpty() || LoadPath.isNull())
        return QString();
    }
    return LoadPath;
}
bool Settings::CheckRunning(){
    HANDLE h=OpenMutexA(MUTEX_ALL_ACCESS,false,"OpenScreen");
    if(nullptr==h){
        return false;
    }
    CloseHandle(h);
    return true;
}
void Settings::Stop(){
    HANDLE hfile = CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(HWND),TEXT("OpenScreen_59198"));
    if(nullptr==hfile)
        return;
    PVOID pview = nullptr;
    pview = MapViewOfFile(hfile,FILE_MAP_ALL_ACCESS,0,0,sizeof(HWND));
    if (nullptr==pview)
        return;
    HWND window;
    memcpy_s(&window,sizeof(HWND),pview,sizeof(HWND));
    PostMessageA(window,WM_CLOSE,0,0);
    return;
}
void Settings::Start(){
    RunMe();
}
