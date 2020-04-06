#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H
#include "pluginhelper.h"
#include <map>
#include <QObject>
#include <Windows.h>
#include <random>
#include "musicplayer.hpp"
#include <QTimer>
#include <thread>
#include <QThread>
#include <atomic>
#include "message.h"
#include <iostream>
Q_DECLARE_METATYPE(std::wstring)
Q_DECLARE_METATYPE(uint64_t)
class MessageTrans:public QObject
{
    Q_OBJECT
public:
    inline MessageTrans():QObject(nullptr){
        std::cout<<std::this_thread::get_id()<<std::endl;
        std::cout<<QThread::currentThreadId()<<std::endl;
    }
    inline ~MessageTrans(){};
public slots:
    inline uint32_t msg(std::wstring s1,std::wstring s2){
        return Message::addmsg(s1,s2.c_str());
    }
    inline void removemsg(uint32_t msg){
        Message::removemsg(msg);
    }
    inline QTimer* createtimer(){
        return new QTimer(nullptr);
    }
    inline void starttimer(QTimer* timer){
        timer->start();
    }
    inline void stoptimer(QTimer* timer){
        timer->stop();
    }
};
class PluginLoader
{
private:
    inline static std::atomic_bool dead=false;
    inline static std::map<uint32_t,std::tuple<HMODULE,PLUGIN_FUNCTIONS,std::wstring> >plugins;
    inline static std::map<uint32_t,std::pair<std::thread,std::atomic_bool> >workers;
    inline static std::vector<uint32_t> tokens;
    inline static std::default_random_engine rnd;
    inline static std::map<uint32_t,QTimer* >timers;
    inline static MessageTrans trans;
public:
    inline static std::mutex unload;
    static bool setmusic(const wchar_t* name);
    static uint32_t addmessage(uint32_t token,const wchar_t* tag,const wchar_t* msg);
    static void removemessage(uint32_t msgId);
    static void initNetwork(uint32_t token);
    static bool getPage(uint32_t token,const char*url,const char* postmsg,const char* referrer,char** ret);
    static uint32_t addTimer(uint32_t token,uint32_t interval,TIMEOUT callback);
    static void removeTimer(uint32_t timerId);
    static void ondropfile(wchar_t* filename);
    static void ondropfiles(uint64_t count,wchar_t **filenames);
    static void ondropdir(wchar_t* dirname);
    static void Init();
    static bool loadplugin(const wchar_t* libfile);
    static void unloadall();
    static void activate(const std::wstring dll,const std::wstring tag);
};
#endif // PLUGINLOADER_H
