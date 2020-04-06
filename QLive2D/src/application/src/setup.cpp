#include "setup.h"
#include <Windows.h>
#include <ShObjIdl.h>
#include <wrl.h>
#include <propvarutil.h>
#include <propkey.h>
#include <stdio.h>
using namespace Microsoft::WRL;

bool Setup::InstallShortcut(const wchar_t *shortcutPath)
{
    wchar_t exePath[MAX_PATH];
    DWORD charWritten = GetModuleFileNameW(NULL, exePath, ARRAYSIZE(exePath));
    if(charWritten==0)return false;
    ComPtr<IShellLink> shellLink;
    bool ok = SUCCEEDED(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&shellLink)));

    if (!ok)return false;
    ok &= SUCCEEDED(shellLink->SetPath(exePath));
    ok &= SUCCEEDED(shellLink->SetArguments(L""));
    if(!ok)return false;
    ComPtr<IPropertyStore> propertyStore;
    ok &= SUCCEEDED(shellLink.As(&propertyStore));
    if(!ok)return false;
    PROPVARIANT PropVar;
    ok &= SUCCEEDED(InitPropVariantFromBoolean(true, &PropVar));
    ok &= SUCCEEDED(propertyStore->SetValue(PKEY_AppUserModel_PreventPinning, PropVar));
    if(!ok)return false;
    ok &= SUCCEEDED(InitPropVariantFromString(L"SixZiv.QtDesktopLive2d", &PropVar));
    ok &= SUCCEEDED(propertyStore->SetValue(PKEY_AppUserModel_ID, PropVar));
    if(!ok)return false;
    CLSID id;

    ok &= SUCCEEDED(CLSIDFromString(L"{3a3b3c1f-2c81-469a-baf7-09d6bbe5cd1b}",&id));
    ok &= SUCCEEDED(InitPropVariantFromCLSID(id,&PropVar));
    ok &= SUCCEEDED(propertyStore->SetValue(PKEY_AppUserModel_ToastActivatorCLSID, PropVar));
    if(!ok)return false;
    ok &= SUCCEEDED(propertyStore->Commit());
    if(!ok)return false;
    ComPtr<IPersistFile> persistFile;
    ok &= SUCCEEDED(shellLink.As(&persistFile));
    if(!ok)return false;
    ok &= SUCCEEDED(persistFile->Save(shortcutPath, TRUE));
    PropVariantClear(&PropVar);
    return ok;
}

int8_t Setup::CreateShortcut(){
    wchar_t shortcutPath[MAX_PATH];
    DWORD charWritten = GetEnvironmentVariable(L"APPDATA", shortcutPath, MAX_PATH);
    HRESULT hr = charWritten > 0 ? S_OK : E_INVALIDARG;

    if (!SUCCEEDED(hr))return -1;
    errno_t concatError = wcscat_s(shortcutPath, ARRAYSIZE(shortcutPath), L"\\Microsoft\\Windows\\Start Menu\\Programs\\QtDesktopLive2d.lnk");
    if(concatError!=0)return -1;

    DWORD attributes = GetFileAttributes(shortcutPath);
    bool fileExists = attributes < 0xFFFFFFF;
    if(fileExists)return 1;
    return InstallShortcut(shortcutPath)?0:-1;
}
