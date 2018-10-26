#define UMDF_USING_NTSTATUS
#include<stdio.h>
#include<conio.h>
#include<Windows.h>
#include<BluetoothAPIs.h>
#include<ntsecapi.h>
#include<ntstatus.h>

BOOL IsUserAdmin(VOID) {
	BOOL b;
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	PSID AdministratorsGroup;
	b = AllocateAndInitializeSid(&NtAuthority,2,SECURITY_BUILTIN_DOMAIN_RID,DOMAIN_ALIAS_RID_ADMINS,0,0,0,0,0,0,&AdministratorsGroup);
	if(b) {
		if (!CheckTokenMembership( NULL, AdministratorsGroup, &b)) {
			b = FALSE;
		}
		FreeSid(AdministratorsGroup);
	} 
	return(b);
}

void SetSelected(bool b) {
	HANDLE Output = GetStdHandle(STD_OUTPUT_HANDLE);
	if(b)
		SetConsoleTextAttribute(Output,BACKGROUND_BLUE|BACKGROUND_GREEN|BACKGROUND_RED);
	else
		SetConsoleTextAttribute(Output,FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED);
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
void GetAll(const HANDLE Radio,unsigned long long *Address,wchar_t **Label,int *Size,const int MaxSize) {
	*Size=0;


	BLUETOOTH_DEVICE_SEARCH_PARAMS par;
	par.dwSize=sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS);
	par.hRadio = Radio;
	par.fReturnAuthenticated = TRUE;
	par.fReturnConnected = FALSE;
	par.fReturnRemembered = TRUE;
	par.fReturnUnknown = TRUE;
	par.fIssueInquiry=TRUE;
	par.cTimeoutMultiplier = 3;

	BLUETOOTH_DEVICE_INFO info;
	info.dwSize=sizeof(BLUETOOTH_DEVICE_INFO);
	HBLUETOOTH_DEVICE_FIND Finder=BluetoothFindFirstDevice(&par, &info);
	bool found;
	if(NULL==Finder)found=false;
	else found=true;
	while(found) {
		Address[*Size]=info.Address.ullLong;
		Label[*Size]=reinterpret_cast<wchar_t*>(malloc((wcslen(info.szName)+1)*sizeof(wchar_t)));
		memset(Label[*Size],0,(wcslen(info.szName)+1)*sizeof(wchar_t));
		wcscpy(Label[*Size],info.szName);
		*Size=*Size+1;
		if(*Size>=MaxSize)return;
		found=BluetoothFindNextDevice(Finder, &info);
	}
	BluetoothFindDeviceClose(Finder);
}
void Clear() {
	system("cls");
}
bool MoveCursor(const int oldCur,const int newCur) {
	if(oldCur/8!=newCur/8) {
		return false;
	}
	HANDLE Output = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(Output,&info);

	COORD oldCOO;
	oldCOO.X=0;
	oldCOO.Y=oldCur%8+2;
	DWORD tmp;
	FillConsoleOutputAttribute(Output,FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_RED,info.dwSize.X,oldCOO,&tmp);

	COORD newCOO;
	newCOO.X=0;
	newCOO.Y=newCur%8+2;
	FillConsoleOutputAttribute(Output,BACKGROUND_GREEN|BACKGROUND_BLUE|BACKGROUND_RED,info.dwSize.X,newCOO,&tmp);
	return true;
}
void ShowPage(const unsigned long long *Address,wchar_t **Label,const int Size,const int Page,const int Selected) {
	Clear();
	int h=8*Page;
	int t=8*Page+8;
	if(h>=Size)return;
	if(t>Size)t=Size;
	printf("Id\tAddress%16s\tLabel\n\n","");
	for(int i=h; i<t; i++) {
		if(i==Selected)SetSelected(true);
		printf("%d\t%016I64x\t%S\n",i+1,Address[i],Label[i]);
		if(i==Selected)SetSelected(false);
	}
	MoveCursor(Selected%8,Selected%8);
}
void GetDevices(unsigned long long *Address, wchar_t **Label,int &size) {
	HBLUETOOTH_RADIO_FIND Finder = NULL;
	HANDLE Radio = NULL;
	getRadio(&Radio,&Finder);

	GetAll(Radio,Address,Label,&size,256);
	CloseHandle(Radio);
	BluetoothFindRadioClose(Finder);
}
int Select(unsigned long long *Address, wchar_t **Label,int &size) {
	int Control=0;
	int Current=0;
	ShowPage(Address,Label,size,0,0);
	while(1) {

		printf("%32s\rItem %d/%d,Page %d/%d\r","",Current+1,size,Current/8+1,(size-1)/8+1);
		int t=getch();

		if(t==224) {
			Control=1;
			continue;
		}
		if(t==0) {
			Control=2;
			continue;
		}
		if(Control==0) {
			if(t==13)
				return Current;
			if(t==27)
				return -1;
		}
		if(Control==1) {
			Control=0;
			if(t==72 && Current!=0) {
				Current--;
				if(7==Current%8)ShowPage(Address,Label,size,Current/8,Current);
				else MoveCursor(Current%8+1,Current%8);
			}
			if(t==80 && Current!=size-1) {
				Current++;
				if(0==Current%8)ShowPage(Address,Label,size,Current/8,Current);
				else MoveCursor(Current%8-1,Current%8);
			}
			if(t==73) {
				if(Current>7)
					Current-=8;
				else
					Current=0;
				ShowPage(Address,Label,size,Current/8,Current);
			}
			if(t==81) {
				if(Current+8<size)
					Current+=8;
				else
					Current=size-1;
				ShowPage(Address,Label,size,Current/8,Current);
			}
		}
		if(Control==2) {
			Control=0;
			if(t==63) {
				printf("%32s\rSearching, please wait...\r","",Current+1,size,Current/8+1,(size-1)/8+1);
				GetDevices(Address,Label,size);
				ShowPage(Address,Label,size,0,0);
				Current=0;
			}
		}
	}
}


void Save(const long long data) {
	wchar_t str[32];
	swprintf(str,L"0x%016I64X#",data);
	printf("Data to save:%S\n",str);
	fflush(stdout);
	LSA_OBJECT_ATTRIBUTES Attr;
	ZeroMemory(&Attr, sizeof(LSA_OBJECT_ATTRIBUTES));
	LSA_HANDLE hPolicy;
	NTSTATUS rtn = LsaOpenPolicy(NULL,&Attr,POLICY_CREATE_SECRET,&hPolicy);
	if (rtn != STATUS_SUCCESS) {
		printf("Save error:OpenPolicy returned %lu\n",LsaNtStatusToWinError(rtn));
		return;
	}
	LSA_UNICODE_STRING SubAuth_Name;
	SubAuth_Name.Length=25*sizeof(wchar_t);
	SubAuth_Name.MaximumLength=64*sizeof(wchar_t);
	SubAuth_Name.Buffer=reinterpret_cast<PWSTR>(malloc(64*sizeof(wchar_t)));
	wcscpy(SubAuth_Name.Buffer,L"SubAuth_Bluetooth_Address");

	LSA_UNICODE_STRING SubAuth_Data;
	SubAuth_Data.Length=wcslen(str)*sizeof(wchar_t);
	SubAuth_Data.MaximumLength=64*sizeof(wchar_t);
	SubAuth_Data.Buffer=reinterpret_cast<PWSTR>(malloc(64*sizeof(wchar_t)));
	wcscpy(SubAuth_Data.Buffer,str);

	rtn=LsaStorePrivateData(hPolicy,&SubAuth_Name,&SubAuth_Data);
	if (rtn != STATUS_SUCCESS) {
		printf("Save error:LsaStorePrivateData returned %lu\n",LsaNtStatusToWinError(rtn));
		return;
	}
	LsaClose(hPolicy);

	printf("All Done!");
	return;

}
int main(int argc,char **argv) {
	if(!IsUserAdmin()) {
		SHELLEXECUTEINFO shExInfo = {0};
		shExInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		shExInfo.fMask = 0;
		shExInfo.hwnd = 0;
		shExInfo.lpVerb = "runas";
		shExInfo.lpFile = argv[0];
		shExInfo.lpParameters = "";
		shExInfo.lpDirectory = NULL;
		shExInfo.nShow = SW_SHOW;
		shExInfo.hInstApp = 0;
		
		ShellExecuteExA(&shExInfo);

		return 0;
	}
	unsigned long long Address[256];
	wchar_t *Label[256];
	int size;
	GetDevices(Address,Label,size);
	int rtn=Select(Address,Label,size);
	if(rtn==-1) {
		printf("\nQuitting...");
		return 0;
	}
	if(rtn>=size || rtn<0) {
		printf("\nSomething unexpected happened...");
		return 0;
	}
	printf("\nYou selected:\n#%d\t%I64x\t%S\nNow Saving...\n",rtn+1,Address[rtn],Label[rtn]);
	Save(Address[rtn]);
	return 0;
}
