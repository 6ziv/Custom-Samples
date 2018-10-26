
#define UMDF_USING_NTSTATUS 
#include <Windows.h>
#include <BluetoothAPIs.h>  
#include <wchar.h>  
#include <ntsecapi.h>
#include <ntstatus.h>
#include <subauth.h>



using namespace std;
 
long long ReadAddr(){
	LSA_OBJECT_ATTRIBUTES Attr;
	ZeroMemory(&Attr, sizeof(LSA_OBJECT_ATTRIBUTES));
	LSA_HANDLE hPolicy;
	NTSTATUS rtn = LsaOpenPolicy(NULL,&Attr,POLICY_GET_PRIVATE_INFORMATION,&hPolicy);
	if (rtn != STATUS_SUCCESS)return 0;
	LSA_UNICODE_STRING SubAuth_Name;
	SubAuth_Name.Length=25*sizeof(wchar_t);
	SubAuth_Name.MaximumLength=64*sizeof(wchar_t);
	SubAuth_Name.Buffer=reinterpret_cast<PWSTR>(malloc(64*sizeof(wchar_t)));
	wcscpy(SubAuth_Name.Buffer,L"SubAuth_Bluetooth_Address");
	PLSA_UNICODE_STRING SubAuth_Data;
	rtn=LsaRetrievePrivateData(hPolicy,&SubAuth_Name,&SubAuth_Data);
	LsaClose(hPolicy);
	if (rtn != STATUS_SUCCESS)return 0;
	if((*SubAuth_Data).Length>19*sizeof(wchar_t))return 0;
	int len=(*SubAuth_Data).Length/sizeof(wchar_t);
	char addr[20];
	for(int i=0;i<len;i++)
		addr[i]=wctob((*SubAuth_Data).Buffer[i]);

	if(addr[0]!='0')return 0;
	if(addr[1]!='x')return 0;
	if(addr[len-1]!='#')return 0;
	long long ans;
	ans=0;
	for(int i=2;i<len-1;i++){
		if((addr[i]<'0' || addr[i]>'9')&&(addr[i]<'A' || addr[i]>'F'))return 0;
		if(addr[i]>='0' && addr[i]<='9')
			ans=ans*16+addr[i]-'0';
		else
			ans=ans*16+addr[i]-'A'+10;
	}
	return ans;
}	
void getRadio(HANDLE *pRadio,HBLUETOOTH_RADIO_FIND *pRadioFind) {
	BLUETOOTH_FIND_RADIO_PARAMS parFinder;
	parFinder.dwSize=sizeof(BLUETOOTH_FIND_RADIO_PARAMS);

	*pRadioFind=BluetoothFindFirstRadio(&parFinder,pRadio);

	BLUETOOTH_RADIO_INFO bri;
	bri.dwSize=sizeof(BLUETOOTH_RADIO_INFO);
	if (BluetoothGetRadioInfo(*pRadio, &bri)!=ERROR_SUCCESS) {
		CloseHandle(*pRadio);
		BluetoothFindRadioClose(*pRadioFind);
		*pRadio=NULL;
		*pRadioFind=NULL;
	}
	return;
}

bool bluetooth()
{  
	unsigned long long addr=ReadAddr();
	if(!addr)return true;
	HANDLE Radio;
	HBLUETOOTH_RADIO_FIND RadioFinder;
	getRadio(&Radio,&RadioFinder);
	if(NULL==Radio)return true;
	
	BLUETOOTH_DEVICE_SEARCH_PARAMS par;
	par.dwSize=sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS);
	par.hRadio = Radio;
	par.fReturnAuthenticated = TRUE;
	par.fReturnConnected = TRUE;
	par.fReturnRemembered = TRUE;
	par.fReturnUnknown = TRUE;
	par.fIssueInquiry=TRUE;
	par.cTimeoutMultiplier = 3;

	BLUETOOTH_DEVICE_INFO info;
	info.dwSize=sizeof(BLUETOOTH_DEVICE_INFO);
	bool found=false;
	HBLUETOOTH_DEVICE_FIND Finder=BluetoothFindFirstDevice(&par, &info);
	bool cont;
	if(NULL==Finder)cont=false;
	else cont=true;
	while (cont)
	{  
		if(info.Address.ullLong==addr){
			found=true;
			break;
		}
		cont=BluetoothFindNextDevice(Finder, &info);
	}
	BluetoothFindDeviceClose(Finder);
	CloseHandle(Radio);
	BluetoothFindRadioClose(RadioFinder);
	return found;
}

NTSTATUS
NTAPI
Msv1_0SubAuthenticationRoutine (
	IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
	IN PVOID LogonInformation,
	IN ULONG Flags,
	IN PUSER_ALL_INFORMATION UserAll,
	OUT PULONG WhichFields,
	OUT PULONG UserFlags,
	OUT PBOOLEAN Authoritative,
	OUT PLARGE_INTEGER LogoffTime,
	OUT PLARGE_INTEGER KickoffTime
)

{
    *Authoritative = TRUE;
    *UserFlags = 0;
    *WhichFields = 0;
	LogoffTime->HighPart = 0x7FFFFFFF;
	LogoffTime->LowPart = 0xFFFFFFFF;
	KickoffTime->HighPart = 0x7FFFFFFF;
	KickoffTime->LowPart = 0xFFFFFFFF;
	if(bluetooth())
		return STATUS_SUCCESS;
	else
		return STATUS_WRONG_PASSWORD;
}


NTSTATUS
NTAPI
Msv1_0SubAuthenticationFilter (
	IN NETLOGON_LOGON_INFO_CLASS LogonLevel,
	IN PVOID LogonInformation,
	IN ULONG Flags,
	IN PUSER_ALL_INFORMATION UserAll,
	OUT PULONG WhichFields,
	OUT PULONG UserFlags,
	OUT PBOOLEAN Authoritative,
	OUT PLARGE_INTEGER LogoffTime,
	OUT PLARGE_INTEGER KickoffTime
)
{
	return Msv1_0SubAuthenticationRoutine(LogonLevel,LogonInformation,Flags,UserAll,WhichFields,UserFlags,Authoritative,LogoffTime,KickoffTime);
}


