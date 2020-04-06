#include "pluginhelper.h"
#include <stdio.h>
#include <Windows.h>
static PLUGIN_FUNCTIONS functions;
uint32_t _msg=0;
uint32_t _token;
char login_credentials[256];
void Activated(const wchar_t* tag){
	ShellExecute(NULL,"open","https://learn.tsinghua.edu.cn", NULL, NULL, SW_SHOWNORMAL);
}
void Release(void){
	if(_msg!=0)
		functions.removemessage(_msg);
}
void timeout(){
	char* ret;
	if(!functions.getPage(_token,"https://learn.tsinghua.edu.cn/f/login",nullptr,nullptr,&ret))return;
	HeapFree(GetProcessHeap(),NULL,ret);
	
	if(!functions.getPage(_token,"https://id.tsinghua.edu.cn/do/off/ui/auth/login/post/bb5df85216504820be7bba2b0ae1535b/0?/login.do",login_credentials,nullptr,&ret))return;
	char* tmp1=strstr(ret,"window.location.replace(\"");
	if(tmp1==NULL)return;
	tmp1=tmp1+25;
	char* tmp2=strstr(tmp1,"\"");
	char* tmp3=reinterpret_cast<char*>(malloc(tmp2-tmp1+1));
	memcpy(tmp3,tmp1,tmp2-tmp1);
	tmp3[tmp2-tmp1]=0;
	HeapFree(GetProcessHeap(),NULL,ret);
	if(!functions.getPage(_token,tmp3,nullptr,"https://id.tsinghua.edu.cn/do/off/ui/auth/login/post/bb5df85216504820be7bba2b0ae1535b/0?/login.do",&ret))return;
	tmp1=strstr(ret,"window.location=\"");
	tmp1=tmp1+17;
	tmp2=strstr(tmp1,"\"");
	char *tmp4=reinterpret_cast<char*>(malloc(tmp2-tmp1+1+29));
	memcpy(tmp4,"https://learn.tsinghua.edu.cn",29);
	memcpy(tmp4+29,tmp1,tmp2-tmp1);
	tmp4[tmp2-tmp1+29]=0;
	HeapFree(GetProcessHeap(),NULL,ret);
	if(!functions.getPage(_token,tmp4,nullptr,tmp3,&ret))return;
	free(tmp3);free(tmp4);
	HeapFree(GetProcessHeap(),NULL,ret);
	if(!functions.getPage(_token,"https://learn.tsinghua.edu.cn/b/kc/zhjw_v_code_xnxq/getCurrentAndNextSemester",nullptr,nullptr,&ret))return;
	tmp1=strstr(ret,"\"xnxq\"");
	if(tmp1==NULL)return;
	tmp1=tmp1+6;
	tmp1=strstr(tmp1,"\"");
	if(tmp1==NULL)return;
	tmp1=tmp1+1;
	tmp2=strstr(tmp1,"\"");
	if(tmp2==NULL)return;
	tmp3=reinterpret_cast<char*>(malloc(tmp2-tmp1+1+96));
	memcpy(tmp3,"https://learn.tsinghua.edu.cn/b/wlxt/kc/v_wlkc_xs_xkb_kcb_extend/student/loadCourseBySemesterId/",96);
	memcpy(tmp3+96,tmp1,tmp2-tmp1);
	tmp3[tmp2-tmp1+96]='\0';
	HeapFree(GetProcessHeap(),NULL,ret);
	if(!functions.getPage(_token,tmp3,nullptr,nullptr,&ret))return;
	free(tmp3);
	//"wjzys"
	char* finder=ret;
	int sum=0;
	while(NULL!=(finder=strstr(finder,"\"wjzys\":"))){
		finder+=8;
		int i;
		if(0==sscanf(finder,"%d",&i)){
			continue;
		}
		sum=sum+i;
	}
	HeapFree(GetProcessHeap(),NULL,ret);
	wchar_t msg[128];
	swprintf(msg,L"You have %d assignments",sum);
	if(_msg!=0)
		functions.removemessage(_msg);
	_msg=functions.addmessage(_token,L"learn",msg);
	return;
}

bool Initialize(uint32_t token,PLUGIN_FUNCTIONS *f){
	_token=token;
	functions=*f;
	f->release=Release;
	f->ondropdir=nullptr;
	f->ondropfile=nullptr;
	f->ondropfiles=nullptr;
	f->initNetwork(token);
	FILE* reader=fopen("conf/learnhelper.config","r");
	if(reader==NULL)return false;
	fscanf(reader,"%s",login_credentials);
	fclose(reader);
	timeout();
	f->addTimer(token,300000,timeout);
	return true;
}
