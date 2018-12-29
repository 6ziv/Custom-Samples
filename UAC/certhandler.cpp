#include "certhandler.h"
#include "Windows.h"
#include "WinTrust.h"
#include "SoftPub.h"
#include "wincrypt.h"
bool VerifySign(wchar_t *filename,wchar_t **pSigner){

    GUID GenericActionId = WINTRUST_ACTION_GENERIC_VERIFY_V2;

    WINTRUST_DATA tData;
    memset(&tData,0,sizeof(WINTRUST_DATA));
    tData.cbStruct = sizeof(WINTRUST_DATA);
    tData.pPolicyCallbackData=NULL;
    tData.pSIPClientData=NULL;
    tData.dwUIChoice=WTD_UI_NONE;
    tData.fdwRevocationChecks=WTD_REVOKE_NONE;
    tData.dwUnionChoice=WTD_CHOICE_FILE;

    WINTRUST_FILE_INFO fInfo;
    fInfo.cbStruct=sizeof(WINTRUST_FILE_INFO_);
    fInfo.hFile=NULL;
    fInfo.pcwszFilePath=filename;
    fInfo.pgKnownSubject=NULL;

    tData.pFile = &fInfo;
    tData.dwStateAction=WTD_STATEACTION_VERIFY;
    tData.hWVTStateData = NULL;
    tData.pwszURLReference=NULL;
    tData.dwProvFlags=WTD_DISABLE_MD2_MD4|WTD_LIFETIME_SIGNING_FLAG|WTD_REVOCATION_CHECK_NONE;
    tData.dwUIContext=0;

    WINTRUST_SIGNATURE_SETTINGS SignSettings;
    SignSettings.cbStruct = sizeof(WINTRUST_SIGNATURE_SETTINGS);
    SignSettings.dwFlags = WSS_VERIFY_SPECIFIC;
    SignSettings.dwIndex = 0;
    SignSettings.dwVerifiedSigIndex=0;
    SignSettings.cSecondarySigs=NULL;

    tData.pSignatureSettings = NULL;
    bool Success;
    if(ERROR_SUCCESS==WinVerifyTrust(NULL, &GenericActionId, &tData))Success=true;else Success=false;
    tData.dwStateAction = WTD_STATEACTION_CLOSE;
    WinVerifyTrust(NULL, &GenericActionId, &tData);
    HCERTSTORE Store;
    HCRYPTMSG  CMsg;

    if(Success)
        Success=CryptQueryObject(CERT_QUERY_OBJECT_FILE,filename,CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,
                                    CERT_QUERY_FORMAT_FLAG_BINARY,0,NULL,NULL,NULL,&Store,&CMsg,NULL
                                 );
    DWORD Size=0;
    if(Success)
        CryptMsgGetParam(CMsg,CMSG_SIGNER_INFO_PARAM,0,NULL,&Size);
    CMSG_SIGNER_INFO *Signer=reinterpret_cast<CMSG_SIGNER_INFO*>(malloc(Size));
    if(Success)
        Success=CryptMsgGetParam(CMsg,CMSG_SIGNER_INFO_PARAM,0,Signer,&Size);
    CERT_INFO Cert;
    Cert.Issuer = Signer->Issuer;
    Cert.SerialNumber = Signer->SerialNumber;
    PCCERT_CONTEXT Context;
    if(Success){
        Context=CertFindCertificateInStore(Store,X509_ASN_ENCODING|PKCS_7_ASN_ENCODING,0,CERT_FIND_SUBJECT_CERT,
                                                            (PVOID)&Cert,NULL
                                                          );
        if(Context==NULL)Success=false;
    }
    DWORD nameSize;
    DWORD nameSize2;

    if(Success)
        nameSize=CertGetNameStringW(Context,CERT_NAME_FRIENDLY_DISPLAY_TYPE,0,NULL,NULL,0);
    *pSigner=reinterpret_cast<wchar_t*>(malloc(sizeof(wchar_t)*nameSize));
    if(Success){
        nameSize2=CertGetNameStringW(Context,CERT_NAME_SIMPLE_DISPLAY_TYPE,0,NULL,*pSigner,nameSize);
        if(nameSize2!=nameSize)Success=false;
    }

    return Success;
}
