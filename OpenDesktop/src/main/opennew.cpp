#include "opennew.h"
#include <Windows.h>

OpenNew::OpenNew()
    :QWebEngineView()
{

    //Warning: I am not sure of that but I strongly feel that by using ShellExecuteA serious.
    //Well, hope that I am wrong.
    connect(this,&QWebEngineView::urlChanged,this,[=](const QUrl &url){
        if((url.scheme()=="http")||(url.scheme()=="https")){
            ShellExecuteA(nullptr,"open",url.toEncoded(QUrl::ComponentFormattingOption::FullyEncoded),NULL,NULL,SW_SHOW);
            this->deleteLater();
        }
    });
}
