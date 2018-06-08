#include "inipara.h"
#include "winapis.h"
#include "share_lock.h"
#include <stdio.h>
#include <tlhelp32.h>
#include <shlobj.h>

#ifdef DISABLE_SAFE
static _NtCreateUserProcess     sNtCreateUserProcess, 
                                pNtCreateUserProcess;
static _CreateProcessInternalW  sCreateProcessInternalW, 
                                pCreateProcessInternalW;
#else
extern _NtCreateUserProcess     sNtCreateUserProcess;
extern _NtCreateUserProcess     pNtCreateUserProcess;
extern _CreateProcessInternalW  sCreateProcessInternalW;
extern _CreateProcessInternalW  pCreateProcessInternalW;
#endif

NTSTATUS WINAPI 
NtCreateUserProcessFn(PHANDLE ProcessHandle,PHANDLE ThreadHandle,
                      ACCESS_MASK ProcessDesiredAccess,
                      ACCESS_MASK ThreadDesiredAccess,
                      POBJECT_ATTRIBUTES ProcessObjectAttributes,
                      POBJECT_ATTRIBUTES ThreadObjectAttributes,
                      ULONG CreateProcessFlags,
                      ULONG CreateThreadFlags,
                      PRTL_USER_PROCESS_PARAMETERS ProcessParameters,
                      PPS_CREATE_INFO CreateInfo,
                      PPS_ATTRIBUTE_LIST AttributeList)
{
    NTSTATUS  status = STATUS_ERROR;
    bool      fn = false;
    if (is_browser(ProcessParameters->ImagePathName.Buffer))
    {
        if (ProcessParameters->CommandLine.Length > 1)
        {
            fn = is_browser(ProcessParameters->CommandLine.Buffer);
        }
    }
    status = sNtCreateUserProcess(ProcessHandle, ThreadHandle,
                                  ProcessDesiredAccess, ThreadDesiredAccess,
                                  ProcessObjectAttributes, ThreadObjectAttributes,
                                  CreateProcessFlags, CreateThreadFlags, ProcessParameters,
                                  CreateInfo, AttributeList);
    if (NT_SUCCESS(status))
    {
        if (fn)
        {
        #ifdef _LOGDEBUG
            logmsg("firefox restart,new main_id is %lu!\n", GetProcessId(*ProcessHandle));
        #endif
            set_process_pid(GetProcessId(*ProcessHandle));
        }
        else
        {
        #ifdef _LOGDEBUG
            logmsg("we set restart is false!\n");
        #endif
        }
    }
    return status;
}

bool WINAPI 
CreateProcessInternalFn(HANDLE hToken,
                        LPCWSTR lpApplicationName,
                        LPWSTR lpCommandLine,
                        LPSECURITY_ATTRIBUTES lpProcessAttributes,
                        LPSECURITY_ATTRIBUTES lpThreadAttributes,
                        bool bInheritHandles,
                        DWORD dwCreationFlags,
                        LPVOID lpEnvironment,
                        LPCWSTR lpCurrentDirectory,
                        LPSTARTUPINFOW lpStartupInfo,
                        LPPROCESS_INFORMATION lpProcessInformation,
                        PHANDLE hNewToken)
{
    bool	ret= false;
    bool    fn = false;
    LPWSTR	lpfile	= lpCommandLine;
    WCHAR   lpFullPath[MAX_PATH+1]= {0};
    if (lpApplicationName && wcslen(lpApplicationName)>1)
    {
        lpfile = (LPWSTR)lpApplicationName;
    #ifdef _LOGDEBUG
        logmsg("process name in %ls lpApplicationName!\n", lpfile);
    #endif
    }
    if (lpCommandLine && wcslen(lpCommandLine)>1)
    {
    #ifdef _LOGDEBUG
        logmsg("lpCommandLine: %ls!\n", lpCommandLine);
    #endif
        fn = is_browser(lpCommandLine);
    }
    ret = sCreateProcessInternalW(hToken,lpApplicationName,lpCommandLine,lpProcessAttributes,
                                  lpThreadAttributes,bInheritHandles,dwCreationFlags,
                                  lpEnvironment,lpCurrentDirectory,
                                  lpStartupInfo,lpProcessInformation,hNewToken);
    if (ret)
    {
        if (fn)
        {
        #ifdef _LOGDEBUG
            logmsg("firefox restart,new main_id is %lu!\n", lpProcessInformation->dwProcessId);
        #endif
            set_process_pid(lpProcessInformation->dwProcessId);
        }
        else
        {
        #ifdef _LOGDEBUG
            logmsg("we set restart is false!\n");
        #endif
        }
    }
    return ret;
}

bool WINAPI init_watch(void)
{
    HMODULE		hNtdll, hKernel;
    DWORD		ver = get_os_version();
    hNtdll   =  GetModuleHandleW(L"ntdll.dll");
    hKernel  =  GetModuleHandleW(L"kernel32.dll");
    if (sCreateProcessInternalW != NULL || sNtCreateUserProcess != NULL)
    {
        return true;
    }
    if (hNtdll == NULL || hKernel  == NULL)
    {
        return false;
    }
    if (ver > 503)  /* vista - win10 */
    {
        pNtCreateUserProcess = (_NtCreateUserProcess)GetProcAddress(hNtdll, "NtCreateUserProcess");
        if (!creator_hook(pNtCreateUserProcess, NtCreateUserProcessFn, (LPVOID*)&sNtCreateUserProcess))
        {
        #ifdef _LOGDEBUG
            logmsg("NtCreateUserProcess hook failed!\n");
        #endif
            return false;
        }
    }
    else          /* winxp-2003 */
    {
        pCreateProcessInternalW	= (_CreateProcessInternalW)GetProcAddress(hKernel, "CreateProcessInternalW");
        if (!creator_hook(pCreateProcessInternalW, CreateProcessInternalFn, (LPVOID*)&sCreateProcessInternalW))
        {
        #ifdef _LOGDEBUG
            logmsg("pCreateProcessInternalW hook failed!\n");
        #endif
            return false;
        }
    }
    return true;
}
