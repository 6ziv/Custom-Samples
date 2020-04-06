#ifndef NETWORKMGR_H
#define NETWORKMGR_H
#define CURL_STATICLIB
#include <curl/curl.h>
#include <map>
#include <random>
class NetworkMgr
{
public:
    inline static void Init(){
        curl_global_init(CURL_GLOBAL_ALL);
        //rnd.seed(GetTickCount());
    }
    inline static void initToken(uint32_t token){
        if(curl.find(token)!=curl.cend())
            curl.erase(token);
        curl[token]=curl_easy_init();
        curl_easy_setopt(curl[token],CURLOPT_COOKIEFILE,"");
        return;
    }
    static bool getPage(uint32_t token,const char*url,const char* postmsg,const char* referrer,char** ret);
    inline static void releaseToken(uint32_t token){
        if(curl.find(token)==curl.cend())return;
        curl_easy_cleanup(curl[token]);
    }
private:
    inline static std::map<uint32_t,CURL *>curl;
};

#endif // NETWORKMGR_H
