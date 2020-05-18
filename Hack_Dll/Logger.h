#ifndef LOGGER_H
#define LOGGER_H

#define LOG2FILE
#define LOG2CONSOLE

#pragma once

#include <FSTREAM>
#include <stdarg.h>
#include <STDIO.H>
#include <STRING>
#include <Windows.h>

typedef enum {
	FATAL = 0,
	WARNING,
	INFORMATION
} LOGLEVEL;

typedef enum {
	DATE_ONLY = 0,
	DATE_TIME
} DATETYPE;

#define MAX_LENGTH		4096

std::wstring format2wstr(const wchar_t *fmt, ...);
std::string wstring2string(std::wstring &wcstr);
std::wstring string2wstring(std::string &mbstr);
void log(LOGLEVEL level, const wchar_t *msg);

inline void log_error(const wchar_t *fmt, ...) {
	wchar_t buf[MAX_LENGTH] = { 0 };
	va_list args;
	va_start(args, fmt);
	vswprintf(buf, fmt, args);
	va_end(args);
	log(FATAL, buf);
}

inline void log_warning(const wchar_t *fmt, ...) {
	wchar_t buf[MAX_LENGTH] = { 0 };
	va_list args;
	va_start(args, fmt);
	vswprintf(buf, fmt, args);
	va_end(args);
	log(WARNING, buf);
}

inline void log_inform(const wchar_t *fmt, ...) {
	wchar_t buf[MAX_LENGTH] = { 0 };
	va_list args;
	va_start(args, fmt);
	vswprintf(buf, fmt, args);
	va_end(args);
	log(INFORMATION, buf);
}

inline void printError(const wchar_t *msg) {
	DWORD eNum;
	wchar_t sysMsg[256];
	wchar_t* p;
	
	eNum = GetLastError();
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, eNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		sysMsg, 256, NULL);
	
	// Trim the end of the line and terminate it with a null
	p = sysMsg;
	while ((*p > 31) || (*p == 9))
		++p;
	do { *p-- = 0; } while ((p >= sysMsg) &&
		((*p == '.') || (*p < 33)));
	
	// Display the message
	wchar_t buf[MAX_LENGTH] = { 0 };
	wsprintfW(buf, L"\n  WARNING: %s failed with error %d (%s)", msg, eNum, sysMsg);
	log(FATAL, buf);
}

inline bool fileExists(std::wstring& filename) {
	std::ifstream f(wstring2string(filename).c_str());
	return f.good();
}



#endif
