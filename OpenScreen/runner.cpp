#include "compileheader.h"

#include <string>
#include <vector>
#include "Windows.h"
#include <WtsApi32.h>
#include <stdio.h>
bool IsAdmin()
{
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    if (!AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                                  &AdministratorsGroup))
        {
            return false;
        }
    BOOL b = FALSE;
    if (!CheckTokenMembership(NULL, AdministratorsGroup, &b))
        {
            b = FALSE;
        }
    FreeSid(AdministratorsGroup);
    return b == TRUE;
}

void RunMeWithToken(HANDLE hToken)
{
    wchar_t t[MAX_PATH + 10];
    GetModuleFileNameW(nullptr, t, MAX_PATH);
    wsprintf(t, L"%s start", t);
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    LPVOID lpEnvironment = nullptr;
    HANDLE hProcessToken = nullptr;
    if (CreateProcessAsUserW(hToken, nullptr, t, nullptr, nullptr, FALSE,
                             CREATE_UNICODE_ENVIRONMENT, nullptr, nullptr, &si,
                             &pi) != TRUE)
        {
            return;
        }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return;
}

void RunMeDirectly()
{
    wchar_t t[MAX_PATH + 10];
    GetModuleFileNameW(nullptr, t, MAX_PATH);
    wsprintf(t, L"%s start", t);
    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));
    CreateProcessW(nullptr, t, nullptr, nullptr, FALSE,
                   CREATE_UNICODE_ENVIRONMENT, nullptr, nullptr, &si, &pi);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return;
}

bool GetWinlogon(DWORD sessionId, HANDLE &hWinlogon)
{
    PWTS_PROCESS_INFOW Start;
    DWORD Count;
    DWORD pid = 0;
    bool Found = false;
    if (WTSEnumerateProcessesW(WTS_CURRENT_SERVER_HANDLE, 0, 1, &Start, &Count) !=
            TRUE)
        {
            return false;
        }
    PWTS_PROCESS_INFOW End = Start + Count;
    for (PWTS_PROCESS_INFOW it = Start; it != End; it++)
        {
            if (it->SessionId == sessionId &&
                    _wcsicmp(L"winlogon.exe", it->pProcessName) == 0 &&
                    IsWellKnownSid(it->pUserSid, WinLocalSystemSid) == TRUE)
                {
                    pid = it->ProcessId;
                    Found = true;
                    break;
                }
        }
    WTSFreeMemory(Start);
    if (!Found)
        return false;
    HANDLE hExistingToken = INVALID_HANDLE_VALUE;
    HANDLE hProcess = ::OpenProcess(MAXIMUM_ALLOWED, FALSE, pid);
    if (hProcess == INVALID_HANDLE_VALUE)
        {
            return false;
        }
    if (OpenProcessToken(hProcess, MAXIMUM_ALLOWED, &hExistingToken) != TRUE)
        {
            CloseHandle(hProcess);
            return false;
        }
    if (DuplicateTokenEx(hExistingToken, MAXIMUM_ALLOWED, nullptr,
                         SecurityImpersonation, TokenImpersonation,
                         &hWinlogon) != TRUE)
        {
            CloseHandle(hProcess);
            CloseHandle(hExistingToken);
            return false;
        }
    CloseHandle(hProcess);
    CloseHandle(hExistingToken);
    return true;
}
void RunMe()
{
    if (!IsAdmin())
        {

            RunMeDirectly();
            return;
        }


    DWORD sessionId;
    HANDLE MyToken = INVALID_HANDLE_VALUE;
    constexpr DWORD flags = TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY;
    if (OpenProcessToken(GetCurrentProcess(), flags, &MyToken) != TRUE)
        {
            return;
        }
    DWORD Length = 0;
    if (GetTokenInformation(MyToken, TokenSessionId, &sessionId, sizeof(DWORD),
                            &Length) != TRUE)
        {
            CloseHandle(MyToken);
            return;
        }
    TOKEN_PRIVILEGES tp;
    LUID luid;
    if (!::LookupPrivilegeValueW(nullptr, SE_DEBUG_NAME, &luid))
        {
            CloseHandle(MyToken);
            return;
        }
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    if (::AdjustTokenPrivileges(MyToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES),
                                nullptr, nullptr) != TRUE)
        {
            CloseHandle(MyToken);
            return;
        }
    if (::GetLastError() == ERROR_NOT_ALL_ASSIGNED)
        {
            CloseHandle(MyToken);
            return;
        }
    CloseHandle(MyToken);
    std::vector<const wchar_t *> privis;
    privis.push_back(SE_TCB_NAME);
    privis.push_back(SE_ASSIGNPRIMARYTOKEN_NAME);
    privis.push_back(SE_INCREASE_QUOTA_NAME);
    privis.push_back(SE_CHANGE_NOTIFY_NAME);
    HANDLE hWinlogon{INVALID_HANDLE_VALUE};
    if (!GetWinlogon(sessionId, hWinlogon))
        {
            return;
        }
    for (const auto lpszPrivilege : privis)
        {
            TOKEN_PRIVILEGES tp;
            LUID luid;
            if (::LookupPrivilegeValueW(nullptr, lpszPrivilege, &luid) != TRUE)
                {
                    continue;
                }
            tp.PrivilegeCount = 1;
            tp.Privileges[0].Luid = luid;
            tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            // Enable the privilege or disable all privileges.
            if (AdjustTokenPrivileges(hWinlogon, FALSE, &tp, sizeof(TOKEN_PRIVILEGES),
                                      nullptr, nullptr) != TRUE)
                {
                    fwprintf(stderr, L"AdjustTokenPrivileges %s\n", lpszPrivilege);
                    continue;
                }
            if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
                {
                    CloseHandle(hWinlogon);
                    return;
                }
        }
    if (SetThreadToken(nullptr, hWinlogon) != TRUE)
        {
            CloseHandle(hWinlogon);
            return;
        }
    CloseHandle(hWinlogon);
    HANDLE hToken = INVALID_HANDLE_VALUE;
    HANDLE hTokenDup = INVALID_HANDLE_VALUE;
    if (WTSQueryUserToken(sessionId, &hToken) != TRUE)
        {
            return;
        }
    if (DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification,
                         TokenPrimary, &hTokenDup) != TRUE)
        {
            CloseHandle(hToken);
            return;
        }
    RunMeWithToken(hToken);

    CloseHandle(hToken);
    CloseHandle(hTokenDup);
}
