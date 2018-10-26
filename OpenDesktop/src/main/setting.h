#ifndef SETTING_H
#define SETTING_H
#include "desktopitem.h"
#include <QMap>
#include <QScreen>
#include <QUrl>

typedef enum ATTRIBUTE{
    JS,
    PLUGIN,
    LOCALSTORAGE,
    UNKNOWNSCHEME,
    NEWWINDOW
}ATTRIBUTE;

class Setting
{
public:
    QUrl url=QUrl("chrome://version");
    bool AllowJS=true;
    bool AllowPlugin=false;
    bool AllowLocalStorage=false;
    bool AllowUnknownScheme=false;
    bool CreateNew=false;
};

extern QMap<QString,Setting> Settings;
void LoadConfig();
void SaveConfig();
void ApplySetting(DesktopItem *item,Setting setting);

#endif // SETTING_H
