//////////////////////////////////////////////////////////////////////////
// Module: File.cpp
// Author: Han Guan
//////////////////////////////////////////////////////////////////////////

#include "File.h"
#pragma comment(lib,"version")


File::File(Directory* pDirectory,LPWIN32_FIND_DATA pFindData)
{
	m_findData = *pFindData;
	char temp[MAX_PATH];
	sprintf(temp,"%s\\%s",pDirectory->getFullName().c_str(),m_findData.cFileName);
	m_name = temp;
}

File::~File()
{
}

std::string File::getName()
{
	return m_findData.cFileName;
}

std::string File::getFullName()
{
	return(m_name);
}

unsigned int File::getSize()
{
	unsigned int nSize;
	nSize = m_findData.nFileSizeHigh * MAXDWORD + m_findData.nFileSizeHigh + m_findData.nFileSizeLow;
	return nSize;
}

std::string File::getCreateTime()
{
	SYSTEMTIME systime;
	FileTimeToSystemTime(&m_findData.ftCreationTime,&systime);
	char date[50];
	sprintf(date,"%04d-%02d-%02d %02d:%02d:%02d",systime.wYear,
		systime.wMonth,systime.wDay,systime.wHour,systime.wMinute,systime.wSecond);
	return date;
}

bool GetFileVersion(const char* filename,VS_FIXEDFILEINFO *ffi) 
{
    DWORD retBytes,dwHandle;
	char *verBuff;
	retBytes = ::GetFileVersionInfoSize(filename,&dwHandle);
	if(retBytes <= 0)
		return false;

	verBuff = new char[retBytes];

	if(!::GetFileVersionInfo(filename,dwHandle,retBytes,verBuff))
	{
		delete []verBuff;
		return false;
	}

	UINT uLen;
	LPVOID tempBuff;
	if(!::VerQueryValue(verBuff,"\\",&tempBuff,&uLen))
	{
		delete []verBuff;
		return false;
	}

	memcpy(ffi,tempBuff,sizeof(VS_FIXEDFILEINFO));

	delete []verBuff;

	return true;
}

std::string File::getVersion()
{
	VS_FIXEDFILEINFO ffi;
	if(!GetFileVersion(m_name.c_str(),&ffi))
		return "";
	else
	{
		char version[50];
		sprintf(version,"%d.%d.%d.%d",HIWORD(ffi.dwFileVersionMS),LOWORD(ffi.dwFileVersionMS),HIWORD(ffi.dwFileVersionLS),LOWORD(ffi.dwFileVersionLS));
		return version;
	}
}

bool File::isFileExist(std::string name)
{
	WIN32_FIND_DATA findData;
	HANDLE hHandle;
	hHandle = ::FindFirstFile (name.c_str(),&findData);
	if (hHandle == INVALID_HANDLE_VALUE)
		return false;
	if((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
	{
		return true;
	}
	return false;
}

