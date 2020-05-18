#include "StdAfx.h"
#include "Logger.h"

#include <CLOCALE>
#include <CSTDLIB>
#include <CTIME>
#include <fstream>

using namespace std;

/*
* Get current date or date_time. Should free memory after use returned value
*/
wstring getCurrentDateTime(DATETYPE type)
{
	time_t now = time(0);
	struct tm  tstruct;
	wchar_t buf[80] = { 0 };
	tstruct = *localtime(&now);

	if (type == DATE_TIME)
		wcsftime(buf, sizeof(buf), L"%Y-%m-%d %X", &tstruct);
	else if (type == DATE_ONLY)
		wcsftime(buf, sizeof(buf), L"%Y-%m-%d", &tstruct);

	return wstring(buf);
	/*TCHAR *ret = (TCHAR *)calloc(_tcslen(buf) + 1, sizeof(TCHAR));
	_tcscpy(ret, buf);
	return ret;*/
};


/**
* Format wstring with argument list
*/
wstring format2wstr(const wchar_t *fmt, ...)
{
	wchar_t buf[MAX_LENGTH] = { 0 };
	va_list args;
	va_start(args, fmt);
	
	vswprintf(buf, fmt, args);
	va_end(args);
	return wstring(buf);
}

string wstring2string(wstring &wstr)
{
	// Setup converter
	setlocale(LC_ALL, "en_US.utf8");
    // UTF-8 narrow multibyte encoding
    char mbstr[MAX_LENGTH];
    wcstombs(mbstr, wstr.c_str(), MAX_LENGTH);

	string cstr(mbstr);
	return cstr;
}

/**
* Append log to file
*/
void log2File(wstring log_str)
{
	DWORD dwLen = MAX_PATH;
	wchar_t strPath[MAX_PATH];
	GetCurrentDirectoryW(dwLen, strPath);
	
	wstring w_path(strPath);
	if (!w_path.empty()) {
		w_path = format2wstr(L"%s\\%s.log", w_path.c_str(), getCurrentDateTime(DATE_ONLY).c_str());
		string path = wstring2string(w_path);
		wofstream outfile;
		outfile.open(path.c_str(), ios_base::app);
		outfile << log_str.c_str() << endl;
		outfile.close();
	}
}

/*
* Make log on several levels
*/
void log(LOGLEVEL level, const wchar_t *msg) {
	const wchar_t *level_str[3] = { L"ERROR", L"WARNING", L"INFORMATION" };
	
	wstring cur_date_time = getCurrentDateTime(DATE_TIME);
	wstring formatted_log = format2wstr(L"%s  (%s) ==> %s\n", cur_date_time.c_str(), level_str[level], msg);
	
#ifdef LOG2FILE
	log2File(formatted_log);
#endif
	
#ifdef LOG2CONSOLE
	wprintf(formatted_log.c_str());
#endif
	
}
