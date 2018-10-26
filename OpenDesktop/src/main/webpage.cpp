#include "webpage.h"
#include <QApplication>
#include <QEvent>
#include <QWebEngineSettings>
#include <Windows.h>
#include "opennew.h"
bool WebPage::GetCreateNew(){
    return Create;
}
void WebPage::SendToChild(QEvent *e){
    QApplication::sendEvent(childObj,e);
}
bool WebPage::event(QEvent *e){
    //Events to a WebEngineView should be handled by a RenderWidgetHostViewQtDelegateWidget
    if (e->type() == QEvent::ChildPolished)
    {
        QChildEvent *child_ev = static_cast<QChildEvent*>(e);
        childObj = child_ev->child();
    }
    return QWebEngineView::event(e);
}
void WebPage::SetCreateNew(bool NewWindow){
    Create=NewWindow;
}
WebPage::WebPage(QWidget* parent)
    : QWebEngineView (parent)
{

    //By default WA_DeleteOnClose is set to false.
    //Leave this line only for personal preferrence.
    this->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose,false);
    this->setGeometry(0,0,parent->width(),parent->height());
    this->show();

    //Load default settings on startup
    this->setContextMenuPolicy(Qt::ContextMenuPolicy::PreventContextMenu);
    this->settings()->setAttribute(QWebEngineSettings::ShowScrollBars,false);

    Goto(QUrl("qrc:/html/default.html"));

}

void WebPage::Goto(QByteArray address){
    QUrl url=QUrl(address,QUrl::ParsingMode::TolerantMode);
    this->load(url);
}
void WebPage::Goto(QUrl url){
    this->load(url);
}
QWebEngineView* WebPage::createWindow(QWebEnginePage::WebWindowType){
    if(Create){
        //Open in a new window (with default web browser)
        QWebEngineView *view=new OpenNew();
        return view;
    }
    else{
        //Do not open new window
        return nullptr;
    }

    //And it is said that "return this" will work, but it actually works fine only in throwing exceptions.
    //And why?
}
