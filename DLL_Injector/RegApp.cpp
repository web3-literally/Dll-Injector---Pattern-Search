#include "StdAfx.h"
#include "RegApp.h"

#include "Logger.h"

#include <SHLOBJ.H>
#include <STRING>
#include <WINDOWS.H>

using namespace std;

wstring GetStartupFolderPath()
{
    wchar_t szPath[MAX_PATH];
    if (SHGetSpecialFolderPathW(NULL, szPath, CSIDL_STARTUP, FALSE)) {
		wstring path(szPath);
        return path;
	} else {
		log_error(L"GetStartupFolderPath Failed");
		return L"";
	}
}

#define SHORTCUT_NAME	L"Dll_Injection.exe.lnk"
#define SHORTCUT_DESC	"Dll Injection shortcut"

bool registAppStartup()
{
	wstring startup_path = GetStartupFolderPath();
	wstring shortcut_path = format2wstr(L"%s\\%s", startup_path.c_str(), SHORTCUT_NAME);
	if (fileExists(shortcut_path)) {
		log_inform(L"Shortcut exist in the startup path");
		return true;
	} else {
		log_inform(L"Shortcut not exist. Creating shortcut %s", shortcut_path.c_str());

		char module_path[MAX_PATH] = { 0 };
		if (GetModuleFileName(NULL, module_path, MAX_PATH)) {
			
			HRESULT hres; 
			IShellLink* psl; 
			
			hres = CoInitialize(NULL);
			if (!SUCCEEDED(hres)) {
				printError(L"CoInitialize Failed");
				return false;
			}

			// Get a pointer to the IShellLink interface. It is assumed that CoInitialize
			// has already been called.
			hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl); 
			if (SUCCEEDED(hres)) 
			{ 
				IPersistFile* ppf; 
				
				// Set the path to the shortcut target and add the description. 
				hres = psl->SetPath(module_path);
				hres = psl->SetDescription(SHORTCUT_DESC);
				
				// Query IShellLink for the IPersistFile interface, used for saving the 
				// shortcut in persistent storage. 
				hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf); 
				
				if (SUCCEEDED(hres)) 
				{
					// Add code here to check return value from MultiByteWideChar 
					// for success.
					
					// Save the link by calling IPersistFile::Save.
					hres = ppf->Save(shortcut_path.c_str(), TRUE); 
					ppf->Release(); 
				}
				psl->Release(); 
			}
			CoUninitialize();
			return hres == S_OK;
		} else {
			return false;
		}
	}
}


