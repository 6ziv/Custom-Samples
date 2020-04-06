#ifndef SETTINGS_H
#define SETTINGS_H
#include <QString>
#include <QFile>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include "pluginloader.hpp"
#include "hotkey.h"
class Settings{
public:
    inline static void InitSettings(HWND mainWindow,QString filename){
        HotKey::setHwnd(mainWindow);
        QFile file(filename);
        if(!file.open(QFile::Text|QFile::ReadOnly))return;
        QByteArray file_content=file.readAll();
        file.close();
        QJsonParseError err;
        QJsonDocument json=QJsonDocument::fromJson(file_content,&err);
        if(err.error!=QJsonParseError::NoError)return;
        if(!json.isObject())return;
        QJsonObject mainObj=json.object();
        if(mainObj.contains("plugins") && mainObj.value("plugins").isArray()){
            QJsonArray plugins=json.object().value("plugins").toArray();
            for(auto plugin:plugins){
                if(!plugin.isString())continue;
                PluginLoader::loadplugin(plugin.toString().toStdWString().c_str());
            }
        }
        if(mainObj.contains("shortcuts") && mainObj.value("shortcuts").isArray()){
            QJsonArray shortcuts=json.object().value("shortcuts").toArray();
            for(auto shortcut:shortcuts){
                if(!shortcut.isObject())continue;
                QJsonObject shortcut_obj=shortcut.toObject();
                if(!shortcut_obj.contains("VirtualKey"))continue;
                if(!shortcut_obj.contains("Modifier"))continue;
                if(!shortcut_obj.contains("Executable"))continue;

                bool ok;
                int vKey=shortcut_obj["VirtualKey"].toVariant().toInt(&ok);
                if(!ok)continue;
                int mod =shortcut_obj["Modifier"].toVariant().toInt(&ok);
                if(!ok)continue;
                if(!shortcut_obj["Executable"].isString())continue;
                QString exe=shortcut_obj["Executable"].toString();
                QString par=QString("");
                do{
                    if(!shortcut_obj.contains("Parameter"))break;
                    if(!shortcut_obj["Parameter"].isString())break;
                    par=shortcut_obj["Parameter"].toString();
                }while(0);
                bool admin=false;
                do{
                    if(!shortcut_obj.contains("RunAsAdmin"))break;
                    if(!shortcut_obj["RunAsAdmin"].isBool())break;
                    admin=shortcut_obj["RunAsAdmin"].toBool();
                }while(0);
                HotKey::addCommand(mod,vKey,exe,par,admin);
            }
        }
    }
};

#endif // SETTINGS_H
