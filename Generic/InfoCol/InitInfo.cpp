// InitInfo.cpp: implementation of the InitInfo class.
//
//////////////////////////////////////////////////////////////////////

//#include "windows.h"
#include "Log.h"
#include "InitInfo.h"
#include "KeyDefine.h"


using namespace ZQ::common;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern "C" bool fileExist(const char* sFile)
{
	WIN32_FIND_DATAA fd;

	HANDLE hFind = FindFirstFileA(sFile, &fd);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;

	FindClose(hFind);

	if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{		
		return false;
	}
	else
		return true;
}

InitInfo::InitInfo()
{
	_szFileName[0]='\0';
	_szCurSession[0]='\0';
}

InitInfo::~InitInfo()
{

}

bool InitInfo::init(const char* szFilename)
{
	if (!fileExist(szFilename))
		return false;

	strcpy(_szFileName, szFilename);
	return true;
}

//set current session name
void InitInfo::setCurrent(const char* szSessionName)
{
	strcpy(_szCurSession, szSessionName);
	glog(Log::L_DEBUG, L"InitInfo::setCurrent() to [%S]", szSessionName);
}

bool InitInfo::getValue(const char*key, int& value, bool bLog)
{
	UINT ret = GetPrivateProfileIntA(_szCurSession, key, 0xffffffff, _szFileName);
	if (ret == 0xffffffff)
	{	
		glog(Log::L_ERROR, KD_KEY_MISSING_FORMAT, key, _szCurSession);
		return false;
	}

	value = ret;
	if (bLog)
		glog(Log::L_DEBUG, L"InitInfo::getValue(), %S = %d", key, ret);

	return true;
}

bool InitInfo::getValue(const char*key, std::string& value, bool bLog, bool bEmptyFail)
{
	char tmp[512];
	GetPrivateProfileStringA(_szCurSession, key, "", tmp, sizeof(tmp), _szFileName);

	value = tmp;
	if (bLog)
		glog(Log::L_DEBUG, L"InitInfo::getValue(), %S = %S", key, tmp);

	if (bEmptyFail && value.empty())
	{
		if (bLog)
			glog(Log::L_ERROR, KD_KEY_MISSING_FORMAT, key, _szCurSession);
		return false;
	}

	return true;
}

bool InitInfo::getValue(const char*key, char* value, int size, bool bLog, bool bEmptyFail)
{
	GetPrivateProfileStringA(_szCurSession, key, "", value, size, _szFileName);
	if (bLog)
		glog(Log::L_DEBUG, L"InitInfo::getValue(), %S = %S", key, value);

	if (bEmptyFail && !value[0])
	{
		if (bLog)
			glog(Log::L_ERROR, KD_KEY_MISSING_FORMAT, key, _szCurSession);
		return false;
	}

	return true;
}

bool InitInfo::getValue(const char*key, std::string& value, const char* strDefault, bool bLog)
{
	char tmp[512];

	if (strDefault == NULL)
	{
		strDefault = "";
	}

	GetPrivateProfileStringA(_szCurSession, key, strDefault, tmp, sizeof(tmp), _szFileName);

	value = tmp;
	if (bLog)
		glog(Log::L_DEBUG, L"InitInfo::getValue(), %S = %S", key, tmp);

	return true;
}

bool InitInfo::getValue(const char*key, char* value, int size, const char* strDefault, bool bLog)
{
	if (strDefault == NULL)
	{
		strDefault = "";
	}

	GetPrivateProfileStringA(_szCurSession, key, strDefault, value, size, _szFileName);
	if (bLog)
		glog(Log::L_DEBUG, L"InitInfo::getValue(), %S = %S", key, value);

	return true;
}
