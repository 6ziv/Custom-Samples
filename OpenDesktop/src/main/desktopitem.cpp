#include "desktopitem.h"
#include<QApplication>
#include<QCloseEvent>
#include<QScreen>
#include<Windows.h>
#include"explorer.h"
DesktopItem::DesktopItem(QScreen *parent)
    : QWidget(nullptr)
{
    this->setGeometry(parent->geometry());
    this->showFullScreen();
    webpage=new WebPage(this);
    webpage->show();
}
QUrl DesktopItem::GetUrl(){
    return webpage->url();
}
bool DesktopItem::GetCreate(){
    return webpage->GetCreateNew();
}
void DesktopItem::SetCreate(bool on){
    webpage->SetCreateNew(on);
}
void DesktopItem::Dispatch(QMouseEvent *e){
    webpage->SendToChild(reinterpret_cast<QEvent *>(e));
}
void DesktopItem::closeEvent(QCloseEvent *e){
    if(dying){
        this->deleteLater();
        e->accept();
    }
    else
        e->ignore();
}
void DesktopItem::Kill(){
    dying=true;
    this->close();
}
bool DesktopItem::nativeEvent(const QByteArray &eventType, void *message, long *){
    if(eventType!="windows_generic_MSG")return false;
    //Look at here, it seems to be a bug of Qt 5.11.1 and has been fixed in 5.11.2,
    //according to https://forum.qt.io/topic/93141/qtablewidget-itemselectionchanged/12
    #if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
            MSG *e = *reinterpret_cast<MSG**>(message);
    #else
            MSG *e = reinterpret_cast<MSG*>(message);
    #endif

    if(e->message==WM_MBUTTONDOWN){
        Explorer::ShowIcons();
        return true;
    }
    return false;
}

void DesktopItem::Goto(const QUrl url){
    webpage->Goto(url);
}
