#include "StdAfx.h"
#include "RegUtil.h"

#include <STRING>
#include <WINDOWS.H>

using namespace std;

LONG GetDWORDRegKey(HKEY hKey, const wstring &strValueName, DWORD &nValue, DWORD nDefaultValue)
{
	nValue = nDefaultValue;
	DWORD dwBufferSize(sizeof(DWORD));
	DWORD nResult(0);
	LONG nError = ::RegQueryValueExW(hKey,
		strValueName.c_str(),
		0,
		NULL,
		reinterpret_cast<LPBYTE>(&nResult),
		&dwBufferSize);
	if (ERROR_SUCCESS == nError)
	{
		nValue = nResult;
	}
	return nError;
}

LONG GetBoolRegKey(HKEY hKey, const wstring &strValueName, bool &bValue, bool bDefaultValue)
{
	DWORD nDefValue((bDefaultValue) ? 1 : 0);
	DWORD nResult(nDefValue);
	LONG nError = GetDWORDRegKey(hKey, strValueName.c_str(), nResult, nDefValue);
	if (ERROR_SUCCESS == nError)
	{
		bValue = (nResult != 0) ? true : false;
	}
	return nError;
}

LONG GetStringRegKey(HKEY hKey, const wstring &strValueName, wstring &strValue, const wstring &strDefaultValue)
{
	strValue = strDefaultValue;
	WCHAR szBuffer[512];
	DWORD dwBufferSize = sizeof(szBuffer);
	ULONG nError;
	nError = RegQueryValueExW(hKey, strValueName.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
	if (ERROR_SUCCESS == nError)
	{
		strValue = szBuffer;
	}
	return nError;
}

LONG SetStringRegKey(HKEY hKey, const wstring &strValueName, wstring &strValue)
{
	wprintf(L"valuename %s value %s", strValueName.c_str(), strValue.c_str());

	ULONG nError;
	
	nError = RegSetValueExW(hKey, strValueName.c_str(), 0, REG_SZ, (LPBYTE)(strValue.c_str()), (DWORD)(wcslen(strValue.c_str()) * sizeof(wchar_t) + 1));
	
	return nError;
}

#define BIN_KEY		L"DLIJ"

bool registAppRegistery()
{
	HKEY hKey;
	LONG lRes = RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hKey);
	bool bExistsAndSuccess (lRes == ERROR_SUCCESS);
	bool bDoesNotExistsSpecifically (lRes == ERROR_FILE_NOT_FOUND);
	
	if (!bExistsAndSuccess && !bDoesNotExistsSpecifically) {
		return false;
	}

	wstring strValueOfBinDir;
		
	LONG nError = GetStringRegKey(hKey, BIN_KEY, strValueOfBinDir, L"");
	
	if (nError != ERROR_SUCCESS || strValueOfBinDir.empty()) {		// this program is not set to auto start
		WCHAR szPath[MAX_PATH];
		if (GetModuleFileNameW(NULL, szPath, MAX_PATH)) {
			strValueOfBinDir = wstring(szPath);			

			nError = SetStringRegKey(hKey, BIN_KEY, strValueOfBinDir);
		} else {
			nError = ERROR_ACCESS_DENIED;
		}
	}

	RegCloseKey(hKey);

	return nError == ERROR_SUCCESS;
}


