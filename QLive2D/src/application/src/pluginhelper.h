#ifndef PLUGINHELPER_H
#define PLUGINHELPER_H
#include <stdint.h>
typedef void(*ACTIVATED)(const wchar_t* tag);
typedef void(*TIMEOUT)(void);
typedef struct{
    //EXE
    bool(*setmusic)(const wchar_t* name);

    uint32_t(*addmessage)(uint32_t ticket,const wchar_t* tag,const wchar_t* msg);
    void(*removemessage)(uint32_t msgId);
    void(*initNetwork)(uint32_t ticket);
    bool(*getPage)(uint32_t ticket,const char*url,const char* postmsg,const char* referrer,char** ret);
    uint32_t(*addTimer)(uint32_t ticker,uint32_t interval,TIMEOUT callback);
    void(*removeTimer)(uint32_t timerId);
    //DLL
    void(*release)(void);
    bool(*ondropfile)(wchar_t* filename);
    bool(*ondropfiles)(uint64_t count,wchar_t** filenames);
    bool(*ondropdir)(wchar_t* dirname);
}PLUGIN_FUNCTIONS;
typedef bool(*INITIALIZE)(uint32_t ticket,PLUGIN_FUNCTIONS *);
#endif // PLUGINHELPER_H
