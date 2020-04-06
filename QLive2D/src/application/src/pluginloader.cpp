#include "pluginloader.hpp"
#include <Windows.h>
#include <algorithm>
#include "networkmgr.h"
#include "message.h"
void PluginLoader::Init(){
    rnd.seed(GetTickCount());
}
bool PluginLoader::loadplugin(const wchar_t* libfile){
    uint32_t token;
    do{
        token=rnd();
    }while(plugins.find(token)!=plugins.cend());

    HMODULE hDll=LoadLibraryW(libfile);
    if(hDll==NULL)return false;
    INITIALIZE init=reinterpret_cast<INITIALIZE>(GetProcAddress(hDll,"Initialize"));
    if(init==NULL)return false;
    PLUGIN_FUNCTIONS func;
    func.setmusic=setmusic;
    func.getPage=getPage;
    func.initNetwork=initNetwork;
    func.addmessage=addmessage;
    func.removemessage=removemessage;
    func.addTimer=addTimer;
    func.removeTimer=removeTimer;
    plugins[token]=std::make_tuple(hDll,func,libfile);
    workers[token]=std::make_pair(std::thread(),false);
    tokens.push_back(token);
    workers[token].second=true;
    workers[token].first=std::thread(
        [](HMODULE hDll,INITIALIZE init,uint32_t token,PLUGIN_FUNCTIONS func){
        bool ok=init(token,&func);
        if(!ok){
            plugins.erase(token);
            FreeLibrary(hDll);
            workers[token].first.detach();
            workers.erase(token);
            return;
        }
        plugins[token]=std::make_tuple(hDll,func,std::get<2>(plugins[token]));
        workers[token].second=false;
    },hDll,init,token,func);
    return true;
}
void PluginLoader::unloadall(){
    dead=true;
    qDebug()<<"UNLOAD1";
    ULONGLONG t1=GetTickCount64();
    while(GetTickCount64()<t1+5000){
        Sleep(100);
        if(std::none_of(workers.cbegin(),workers.cend(),
                       [](const decltype(workers)::value_type& t)->bool{
                       return static_cast<bool>(t.second.second);})){
            break;
        }
    }
    qDebug()<<"UNLOAD2";
    for(auto token:tokens){
        if(workers[token].second){
            workers[token].first.detach();
            workers[token].second=false;
        }else{
            if(workers[token].first.joinable())
                workers[token].first.join();
            if(std::get<1>(plugins[token]).release!=nullptr){
                workers[token].second=true;
                workers[token].first=std::thread(
                            [](const PLUGIN_FUNCTIONS& func,std::atomic_bool &busy){
                                func.release();
                                busy=false;
                            },std::cref(std::get<1>(plugins[token])),std::ref(workers[token].second)
                            );
            }
        }
    }
    qDebug()<<"UNLOAD3";
    t1=GetTickCount64();
    while(GetTickCount64()<t1+5000){
        Sleep(100);
        if(std::none_of(workers.cbegin(),workers.cend(),
                       [](const decltype(workers)::value_type& t)->bool{
                       return t.second.second;})){
            break;
        }
    }
    qDebug()<<"UNLOAD4";
    for(auto token:tokens){
        NetworkMgr::releaseToken(token);
        if(workers[token].second){
            workers[token].first.detach();
            workers[token].second=false;
        }else{
            if(workers[token].first.joinable())
                workers[token].first.join();
            FreeLibrary(std::get<0>(plugins[token]));
        }
    }
    qDebug()<<"UNLOAD5";
    tokens.clear();plugins.clear();workers.clear();
    qDebug()<<"UNLOAD6";
}

void PluginLoader::ondropfile(wchar_t* filename){
    if(dead)return;
    for(auto token:tokens){
        if(std::get<1>(plugins[token]).ondropfile==nullptr)
            continue;
        if(workers[token].second)
            continue;
        if(workers[token].first.joinable())
            workers[token].first.join();
        workers[token].second=true;
        workers[token].first=std::thread(
                    [](wchar_t* fn,const PLUGIN_FUNCTIONS& func,std::atomic_bool &busy){
                        func.ondropfile(fn);
                        busy=false;
                        free(fn);
                    },filename,std::cref(std::get<1>(plugins[token])),std::ref(workers[token].second)
                    );
    }
}
void PluginLoader::ondropfiles(uint64_t count,wchar_t** filenames){
    if(dead)return;
    for(auto token:tokens){
        if(std::get<1>(plugins[token]).ondropfiles==nullptr)
            continue;
        if(workers[token].second)
            continue;
        if(workers[token].first.joinable())
            workers[token].first.join();
        workers[token].second=true;
        workers[token].first=std::thread(
                    [](uint64_t c,wchar_t** fn,const PLUGIN_FUNCTIONS& func,std::atomic_bool &busy){
                        func.ondropfiles(c,fn);
                        busy=false;
                        for(uint64_t i=0;i<c;i++)
                            free(fn[i]);
                        free(fn);
                    },count,filenames,
                        std::cref(std::get<1>(plugins[token])),
                        std::ref(workers[token].second)
                    );
    }
}
void PluginLoader::ondropdir(wchar_t* dirname){
    if(dead)return;
    qDebug()<<std::wstring(dirname);
    for(auto token:tokens){
        if(std::get<1>(plugins[token]).ondropdir==nullptr)
            continue;
        if(workers[token].second)
            continue;
        if(workers[token].first.joinable())
            workers[token].first.join();
        workers[token].second=true;
        workers[token].first=std::thread(
                    [](wchar_t* fn,const PLUGIN_FUNCTIONS& func,std::atomic_bool &busy){
                        func.ondropdir(fn);
                        free(fn);
                        busy=false;
                    },dirname,std::cref(std::get<1>(plugins[token])),std::ref(workers[token].second)
                    );
    }
}

bool PluginLoader::setmusic(const wchar_t* filename){
    return MusicPlayer::GetInstance()->setMusic(filename);
}
uint32_t PluginLoader::addmessage(uint32_t token,const wchar_t* tag,const wchar_t* msg){
    std::wstring xtag=std::get<2>(plugins[token])+L"?"+tag;
    uint32_t ret;
    QMetaObject::invokeMethod(&trans,"msg", Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(uint32_t,std::ref(ret)),
                              Q_ARG( std::wstring, xtag ),
                              Q_ARG( std::wstring, std::wstring(msg) )
                              );
    return ret;
}
void PluginLoader::removemessage(uint32_t msgId){
    QMetaObject::invokeMethod(&trans,"removemsg", Qt::BlockingQueuedConnection,
                              Q_ARG( uint32_t, msgId )
                              );
}
void PluginLoader::initNetwork(uint32_t token){
    if(plugins.find(token)==plugins.cend())
        return;
    NetworkMgr::initToken(token);
    return;
}
bool PluginLoader::getPage(uint32_t token,const char*url,const char* postmsg,const char* referrer,char** ret){
    *ret=nullptr;
    if(plugins.find(token)==plugins.cend())
        return false;
    return NetworkMgr::getPage(token,url,postmsg,referrer,ret);
}
uint32_t PluginLoader::addTimer(uint32_t token,uint32_t interval,TIMEOUT callback){
    if(plugins.find(token)==plugins.cend())
        return 0;
    uint32_t timerId;
    do{
        timerId=rnd();
    }while((timerId==0) || (timers.find(timerId)!=timers.cend()));
    QMetaObject::invokeMethod(&trans,"createtimer", Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG( QTimer*, timers[timerId] )
                              );
    timers[timerId]->setInterval(interval);
    timers[timerId]->setSingleShot(false);
    timers[timerId]->connect(
                timers[timerId],&QTimer::timeout,timers[timerId],
                             [token,callback]()
    {
        if(PluginLoader::workers[token].second)return;
        PluginLoader::workers[token].second=true;
        if(PluginLoader::workers[token].first.joinable())
            PluginLoader::workers[token].first.join();
        PluginLoader::workers[token].first=std::thread(
            [](TIMEOUT timeout,std::atomic_bool &busy)
        {
            std::invoke(timeout);
            busy=false;
        },callback,std::ref(PluginLoader::workers[token].second));
    });
    QMetaObject::invokeMethod(&trans,"starttimer", Qt::BlockingQueuedConnection,
                              Q_ARG( QTimer*, timers[timerId] )
                              );

    return timerId;
}
void PluginLoader::removeTimer(uint32_t timerId){
    QMetaObject::invokeMethod(&trans,"stoptimer", Qt::BlockingQueuedConnection,
                              Q_ARG( QTimer*, timers[timerId] )
                              );
    timers[timerId]->deleteLater();
    timers.erase(timerId);
}
void PluginLoader::activate(const std::wstring dll,const std::wstring tag){
    HMODULE hDll=LoadLibraryW(dll.c_str());
    if(hDll==NULL)return;
    ACTIVATED activated=reinterpret_cast<ACTIVATED>(GetProcAddress(hDll,"Activated"));
    if(activated==NULL)return;
    if(Message::toast_done){
        activated(tag.c_str());
        FreeLibrary(hDll);
        return;
    }else{
        std::thread(
            [](const std::wstring ws,ACTIVATED activate,HMODULE module){
                activate(ws.c_str());
                FreeLibrary(module);
            }
            ,tag,activated,hDll).detach();
    }
}
