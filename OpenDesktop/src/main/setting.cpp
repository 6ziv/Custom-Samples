#include "setting.h"
#include <QApplication>
#include<QDebug>
#include <QScreen>
#include <QSettings>
#include <QWebEngineSettings>
#include <QStandardPaths>
QMap<QString,Setting> Settings;
void LoadConfig(){
    //Load from registry configure information of known screens
    QSettings settings("SixZiv","TestWL",nullptr);
    QList<QScreen*> list=QApplication::screens();
    for(QList<QScreen*>::iterator iter=list.begin();iter!=list.end();iter++){
        QString name=(*iter)->name();
        Setting tmp;
        tmp.url=settings.value(name+QString("/url"),QUrl("chrome://version")).toUrl();
        tmp.AllowJS=settings.value(name+QString("/AllowJavascript"),true).toBool();
        tmp.AllowPlugin=settings.value(name+QString("/AllowPlugins"),false).toBool();
        tmp.AllowLocalStorage=settings.value(name+QString("/AllowLocalStorage"),false).toBool();
        tmp.AllowUnknownScheme=settings.value(name+QString("/AllowUnknownScheme"),false).toBool();
        tmp.CreateNew=settings.value(name+QString("/AllowCreatingNewWindow"),false).toBool();
        Settings.insert(name,tmp);
    }
}

void SaveConfig(){
    //Save to registry configure information of running screens
    QSettings settings("SixZiv","TestWL",nullptr);
    settings.clear();
    for(QMap<QString,Setting>::iterator iter=Settings.begin();iter!=Settings.end();iter++){
        settings.beginGroup(iter.key());
        settings.setValue("url",iter->url);
        settings.setValue("AllowJavascript",iter->AllowJS);
        settings.setValue("AllowPlugins",iter->AllowPlugin);
        settings.setValue("AllowLocalStorage",iter->AllowLocalStorage);
        settings.setValue("AllowUnknownScheme",iter->AllowUnknownScheme);
        settings.setValue("AllowCreatingNewWindow",iter->CreateNew);
        settings.endGroup();
    }
}

void ApplySetting(DesktopItem *item,Setting setting){
    if(item->webpage->url()!=setting.url)item->Goto(QUrl("www.baidu.com"));
    qDebug()<<setting.url;
    item->webpage->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled,setting.AllowJS);
    item->webpage->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled,setting.AllowLocalStorage);
    item->webpage->settings()->setAttribute(QWebEngineSettings::PluginsEnabled,setting.AllowPlugin);
    if(setting.AllowUnknownScheme)
        item->webpage->settings()->setUnknownUrlSchemePolicy(QWebEngineSettings::AllowUnknownUrlSchemesFromUserInteraction);
    else
        item->webpage->settings()->setUnknownUrlSchemePolicy(QWebEngineSettings::DisallowUnknownUrlSchemes);
}
