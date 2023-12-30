
#ifndef  _CONTENT_EXPORT_COMMON_
#define  _CONTENT_EXPORT_COMMON_

#pragma warning(disable: 4275)
#pragma warning(disable: 4251)
#pragma warning(disable: 4786)


#include "Log.h"
#include <string>


using namespace std;
using namespace ZQ::common;

std::string getErrMsg(uint32 dwErrCode);

std::string getErrMsg();

bool validatePath( const char*     szPath );

//delete the end ' ' && '\\'
// for example str: "c:\RDS\home\   "  -> "c:\RDS\home"
void deleteDirEndSlash(char* str);

bool fileExist(const char* sFile);

bool dirExist(const char* sDir);

#ifdef ZQ_OS_MSWIN
inline uint64 GetFileSizeA(const char* FileName)
{
	uint64  ret = 0;
	
	HANDLE hFile=CreateFileA(FileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if(hFile!=INVALID_HANDLE_VALUE)
	{
		*((DWORD*)&ret) = GetFileSize(hFile, ((DWORD*)&ret) + 1);
		CloseHandle(hFile);
	}

	return ret;
}


inline uint64 GetFileSizeW(const wchar_t* FileName)
{
	uint64  ret = 0;	
	
	HANDLE hFile=CreateFileW(FileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if(hFile!=INVALID_HANDLE_VALUE)
	{
		*((DWORD*)&ret) = GetFileSize(hFile, ((DWORD*)&ret) + 1);
		CloseHandle(hFile);
	}

	return ret;
}
#endif

#endif
