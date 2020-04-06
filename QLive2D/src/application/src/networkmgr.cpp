#include "networkmgr.h"
#include <random>
#include "qdebug.h"
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"zlib.lib")
#pragma comment(lib,"crypt32.lib")
#pragma comment(lib,"libcurl.lib")
#pragma comment(lib,"wldap32.lib")
size_t wf(char*ptr,size_t size,size_t nmemb,void* userdata){
   std::string &writer=*reinterpret_cast<std::string*>(userdata);
   writer.append(ptr,nmemb*size);
   return size*nmemb;
}
bool NetworkMgr::getPage(uint32_t token, const char *url,const char* postmsg,const char *referrer, char **ret)
{
    if(curl.find(token)==curl.cend())return false;
    std::string http;
    http.clear();
    CURL* &session=curl[token];
    curl_easy_setopt(session,CURLOPT_URL,url);
    //curl_easy_setopt(session, CURLOPT_PROXY, "127.0.0.1:8888");
    curl_easy_setopt(session,CURLOPT_USERAGENT,"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.149 Safari/537.36");
    curl_easy_setopt(session,CURLOPT_FOLLOWLOCATION,1);
    if(postmsg==nullptr){
        curl_easy_setopt(session,CURLOPT_POST,0);
        curl_easy_setopt(session,CURLOPT_HTTPGET,0);
    }else{
        curl_easy_setopt(session,CURLOPT_POST,1);
        curl_easy_setopt(session,CURLOPT_POSTFIELDS,postmsg);
    }
    curl_easy_setopt(session,CURLOPT_AUTOREFERER,1);
    curl_easy_setopt(session,CURLOPT_TIMEOUT,5);
    curl_easy_setopt(session,CURLOPT_WRITEDATA,&http);
    curl_easy_setopt(session,CURLOPT_WRITEFUNCTION,wf);
    if(referrer!=nullptr)
        curl_easy_setopt(session,CURLOPT_REFERER,referrer);
    curl_easy_setopt(session,CURLOPT_SSL_VERIFYPEER,0L);
    curl_easy_setopt(session,CURLOPT_SSL_VERIFYHOST,0L);
    curl_easy_perform(session);
    long retcode;
    auto code = curl_easy_getinfo(session, CURLINFO_RESPONSE_CODE , &retcode);
    if(code != CURLE_OK || retcode != 200 )return false;
    *ret=reinterpret_cast<char*>(HeapAlloc(GetProcessHeap(),NULL,http.length()+1));
    if(ret==NULL)return false;
    memcpy(*ret,http.c_str(),http.length()+1);
    return true;
}
