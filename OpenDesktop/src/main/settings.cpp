#include "settings.h"
#include"explorer.h"
#include<Windows.h>
#include <QApplication>
#include <QPushButton>
#include<QComboBox>
#include<QScreen>
#include<QSettings>
#include<QMessageBox>
#include<QStyle>
#include<QLabel>
#include<QList>
#include<QVariant>
#include<QFileDialog>
Settings* Settings::ptr_Settings=nullptr;
URLLineEdit::URLLineEdit(QWidget *parent):QLineEdit (parent)
{
    par=reinterpret_cast<Settings *>(parent);
    this->setText(par->qurl.fileName());
}
void URLLineEdit::mouseReleaseEvent(QMouseEvent *){
    QUrl url=QFileDialog::getOpenFileUrl();
    if(url.isLocalFile()){
        this->setText(url.fileName());
        par->qurl=url;
        par->Changed=true;
        par->OK->setEnabled(true);
        par->Apply->setEnabled(true);
    }
}

void Settings::SetStartUp(bool on){
    if(on){
        wchar_t mypath[2048];
        DWORD size=GetModuleFileNameW(nullptr,mypath,2048);
        HKEY hKey;
        int ret=RegOpenKeyExA(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Run",NULL,KEY_SET_VALUE,&hKey);
        if(ret!=ERROR_SUCCESS)
            return;
        RegSetValueExW(hKey,L"OpenDesktop",0,REG_SZ,reinterpret_cast<BYTE *>(mypath),size*sizeof(wchar_t));
        RegCloseKey(hKey);
    }else{
        HKEY hKey;
        int ret=RegOpenKeyExA(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Run",NULL,KEY_ALL_ACCESS,&hKey);
        if(ret!=ERROR_SUCCESS)
            return;
        RegDeleteValueA(hKey,"OpenDesktop");
        RegCloseKey(hKey);
    }
    SetLastError(ERROR_SUCCESS);
}

bool Settings::GetStartUp(){
    HKEY hKey;
    RegOpenKeyExA(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Run",NULL,KEY_QUERY_VALUE,&hKey);
    LSTATUS ret=RegGetValueA(hKey,NULL,"OpenDesktop",RRF_RT_REG_SZ,nullptr,nullptr,nullptr);
    RegCloseKey(hKey);
    if(ret==ERROR_SUCCESS)
        return true;
    else
        return false;
}

QUrl Settings::GetUrl(QString key){
    QSettings settings("6ziv","OpenDesktop");
    return settings.value(key.append("/Url"),QUrl("qrc:/html/default.html")).toUrl();
}
bool Settings::GetCreate(QString key){
    QSettings settings("6ziv","OpenDesktop");
    return settings.value(key.append("/AllowCreatingNewWindows"),false).toBool();
}
void Settings::Set(QString key,QUrl url,bool create){
    QSettings settings("6ziv","OpenDesktop");
    settings.beginGroup(key);
    settings.setValue("/Url",url);
    settings.setValue("/AllowCreatingNewWindows",create);
}
Settings::Settings(QWidget *parent) : QWidget(parent)
{
    if(Settings::ptr_Settings){
        Settings::ptr_Settings->show();
        this->deleteLater();
        return;
    }else
        ptr_Settings=this;
    //It is said that desktop() does not work properly when handling more than one screens according to doc.qt.io.
    //Have not tried that out for I am too poor to get myself a second screen
    int width=QApplication::screens().at(0)->size().width();
    int height=QApplication::screens().at(0)->size().height();

    QLabel *local=new QLabel(this);
    local->setGeometry(0,height/120,width/10,height/60);
    local->setText("Local Settings");
    local->show();

    //Select a screen to configure
    QLabel *screen=new QLabel(this);
    screen->setGeometry(width/50,height/30,width/25,height/40);
    screen->setText("Screen:");
    screen->show();
    SelectAScreen=new QComboBox(this);
    SelectAScreen->setGeometry(width*3/50,height/30,width*6/50,height/40);
    SelectAScreen->show();
    SelectAScreen->setCurrentIndex(0);
    oldIndex=0;

    //Configure the url shown
    QLabel *URLLabel=new QLabel(this);
    URLLabel->setGeometry(width/50,height/15,width/25,height/40);
    URLLabel->setText("URL:");
    URLLabel->show();
    url=new URLLineEdit(this);
    url->setGeometry(width*3/50,height/15,width*3/40,height/40);
    url->setText("");
    url->setReadOnly(true);
    url->show();
    QPushButton *urlDefault=new QPushButton(this);
    urlDefault->setGeometry(width*28/200,height/15,width/25,height/40);
    urlDefault->setText("Default");
    urlDefault->show();

    //Allow creating new windows
    QLabel *CreateLabel=new QLabel(this);
    CreateLabel->setGeometry(width/50,height/10,width/8,height/40);
    CreateLabel->setText("Allow Creating New Windows:");
    CreateLabel->show();
    CreateNew=new QCheckBox(this);
    CreateNew->setGeometry(width*3/20,height/10,width/50,height/50);
    CreateNew->show();

    QLabel *global=new QLabel(this);
    global->setGeometry(0,height*3/20,width/10,height/60);
    global->setText("Global Settings");
    global->show();

    QLabel *StartLabel=new QLabel(this);
    StartLabel->setGeometry(width/50,height*7/40,width/8,height/40);
    StartLabel->setText("Startup With Windows:");
    StartLabel->show();
    StartWithWindows=new QCheckBox(this);
    StartWithWindows->setGeometry(width*6/40,height*7/40,width/50,height/50);
    StartWithWindows->setChecked(GetStartUp());
    StartWithWindows->show();

    //OK button
    OK=new QPushButton(this);
    OK->setText("Okay");
    OK->setGeometry(width/120,height/4,width/20,height/30);
    OK->setEnabled(false);
    OK->show();

    //Apply button
    Apply=new QPushButton(this);
    Apply->setText("Apply");
    Apply->setGeometry(width*9/120,height/4,width/20,height/30);
    Apply->setEnabled(false);
    Apply->show();

    //Cancel button
    Cancel=new QPushButton(this);
    Cancel->setText("Cancel");
    Cancel->setGeometry(width*17/120,height/4,width/20,height/30);
    Cancel->show();

    //About
    QPushButton *Me=new QPushButton(this);
    Me->setFlat(true);
    Me->setGeometry(width/100,height*3/10,width*17/300,height/30);
    Me->setText("OpenDesktop");
    Me->setStyleSheet("color:blue");
    Me->show();

    QLabel *label=new QLabel(this);
    label->setText(",proudly based on ");
    label->setGeometry(width/15,height*3/10,width*2/25,height/30);
    label->show();
    QPushButton *Qt=new QPushButton(this);
    Qt->setFlat(true);
    Qt->setText(QString("Qt ")+QString(QT_VERSION_STR));
    Qt->setGeometry(width*11/75,height*3/10,width*4/75,height/30);
    Qt->setStyleSheet("color:blue");
    Qt->show();

    //Loading screens
    int size=Explorer::getSize();
    for(int i=0;i<size;i++){
        SelectAScreen->addItem(Explorer::getname(i),i);
    }
    if(size==1)
        SelectAScreen->setEnabled(false);
    this->setWindowIcon(QIcon(":/icon/default.ico"));
    this->setGeometry(width*2/5,height/3,width/5,height/3);
    this->setMaximumSize(this->size());
    this->setMinimumSize(this->size());
    this->setWindowFlags(Qt::Dialog|Qt::WindowCloseButtonHint);

    connect(SelectAScreen,SIGNAL(currentIndexChanged(int)),this,SLOT(FuncChanged(int)));
    connect(urlDefault,&QPushButton::pressed,this,[=](){
        Changed=true;
        OK->setEnabled(true);
        Apply->setEnabled(true);
        qurl=QUrl("qrc:/html/default.html");
        url->setText(qurl.fileName());
    });
    connect(CreateNew,&QCheckBox::stateChanged,this,[=](){
        Changed=true;
        OK->setEnabled(true);
        Apply->setEnabled(true);
    });
    connect(StartWithWindows,&QCheckBox::stateChanged,this,[=](){
        OK->setEnabled(true);
        Apply->setEnabled(true);
    });
    connect(Me,&QPushButton::clicked,this,[=](){
        QMessageBox::about(this,
                           "About OpenDesktop",
                           "Simply an opensource project for putting HTML pages onto the desktop<br/>\
                           Using QWebEngine(Chromium) as core.<br/><br/>\
                           Partly based on work of <a href='https://github.com/ThomasHuai/'>ThomasHuai</a>.<br/>\
                           You can follow <a href='https://github.com/ThomasHuai/Wallpaper'>this link</a> to see the original source.<br/><br/><br/>\
                           project website: <a href='https://github.com/6ziv/OpenDesktop'>https://github.com/6ziv/OpenDesktop</a><br/>\
                           For reporting bugs, please <a href='mailto:root@6ziv.com'>email to me</a> or <a href='https://github.com/6ziv/OpenDesktop/issues/new'>come up with an issue</a>.<br/><br/>\
                           As it is based on Qt Open Source, please use under <a href='https://www.gnu.org/licenses/gpl-3.0.html'>GPL License</a>.<br/><br/>\
                           Have Fun!"
                );
    });
    connect(OK,&QPushButton::clicked,this,[=](){
        Explorer::seturl(oldIndex,qurl);
        Explorer::setCreatable(oldIndex,CreateNew->isChecked());
        Set(Explorer::getname(oldIndex),qurl,CreateNew->isChecked());
        if(GetStartUp()!=StartWithWindows->isChecked()){
            SetStartUp(StartWithWindows->isChecked());
        }
        this->deleteLater();
    });
    connect(Apply,&QPushButton::clicked,this,[=](){
        Apply->setEnabled(false);
        OK->setEnabled(false);
        if(Changed){
            Explorer::seturl(oldIndex,qurl);
            Explorer::setCreatable(oldIndex,CreateNew->isChecked());
            Set(Explorer::getname(oldIndex),qurl,CreateNew->isChecked());
        }
        Changed=false;
        if(GetStartUp()!=StartWithWindows->isChecked()){
            SetStartUp(StartWithWindows->isChecked());
        }
    });
    connect(Cancel,&QPushButton::clicked,this,[=](){
        this->deleteLater();
    });
    connect(Qt,&QPushButton::clicked,this,[=](){
        QApplication::aboutQt();
    });

    FuncChanged(0);
    this->show();
}
Settings::~Settings(){
    Settings::ptr_Settings=nullptr;
}
void Settings::FuncChanged(int NewId){
    if(Changed){
        MessageBoxA(nullptr,"Settings Changed, please save first!","Message",MB_OK);
        SelectAScreen->setCurrentIndex(oldIndex);
    }else{
        qurl=Explorer::geturl(NewId);
        url->setText(qurl.fileName());
        CreateNew->setChecked(Explorer::getCreatable(NewId));
        oldIndex=NewId;
    }
}

