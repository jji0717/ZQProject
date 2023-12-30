
#include "CECommon.h"
#include "NTFSIO.h"


#define LOG_MODULE_NAME			"NtfsIo"

using namespace ZQ::common;

#define MOLOG	(glog)

#ifdef ZQ_OS_MSWIN
NTFSIO::NTFSIO()
{
	_hFile = NULL;
}

bool NTFSIO::Open(const char* szFile, int nOpenFlag)
{
	if (nOpenFlag == BF_READ)
	{
		_hFile = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if (_hFile == INVALID_HANDLE_VALUE)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Failed to open file[%s] for read, errorcode[%d]"), 
				szFile, GetLastError());
			return false;
		}

		MOLOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "Open file[%s] for read successful"), 
			szFile);
	}
	else
	{
		MOLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Open falg[%d] not implemented"), 
			nOpenFlag);

		return false;
	}

	_strFile = szFile;
	_bOpened = true;
	return true;
}

int64 NTFSIO::GetFileSize(const char* szFile)
{
	if (!_bOpened)
		return 0;

	int64 lRet = 0;
	DWORD* pdwLow = (DWORD*)&lRet;
	DWORD* pdwHigh = pdwLow + 1;
	*pdwLow = ::GetFileSize(_hFile, pdwHigh);

	MOLOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "File[%s] size is [%lld] bytes"), 
		_strFile.c_str(), lRet);

	return lRet;
}

int NTFSIO::Read(char* pPtr, int nReadLen)
{
	if (!_bOpened)
		return 0;

	DWORD dwRet = 0;
	if (!::ReadFile(_hFile, pPtr, nReadLen, &dwRet, 0))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "ReadFile[%s] failed, errorcode[%d]"), 
			_strFile.c_str(), GetLastError());
	}

	return dwRet;
}

bool NTFSIO::Write(char* pPtr, int nWriteLen)
{
	if (!_bOpened)
		return false;

	DWORD dwWritten = 0;
	if (!::WriteFile(_hFile, pPtr, nWriteLen, &dwWritten, 0))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "WriteFile[%s] failed, errorcode[%d]"), 
			_strFile.c_str(), GetLastError());
		return false;
	}
	return true;
}

bool NTFSIO::Seek(int64 lOffset, int nPosFlag)
{
	if (!_bOpened)
		return false;

	DWORD dwPos = FILE_BEGIN;
	if (nPosFlag==FP_CURRENT)
		dwPos = FILE_CURRENT;
	else if(nPosFlag==FP_END)
		dwPos = FILE_END;

	LONGLONG lTmp = lOffset;
	LONG* pdwLow = (LONG*)&lTmp;
	LONG* pdwHigh = pdwLow + 1;
	*pdwLow = SetFilePointer(_hFile, *pdwLow, pdwLow, dwPos);
	
	if (lTmp == lOffset)
	{
		MOLOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "File[%s] seek to offset[%lld] movemethod[%d] successful"), 
			_strFile.c_str(), lOffset, dwPos);
		return true;
	}
	else
	{
		MOLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "File[%s] failed to seek to offset[%lld] movemethod[%d]"), 
			_strFile.c_str(), lOffset, dwPos);
		return false;
	}
}

void NTFSIO::Close()
{
	if (!_bOpened || !_hFile || _hFile==INVALID_HANDLE_VALUE)
		return;

	CloseHandle(_hFile);
	_hFile = NULL;
	_bOpened = false;

	MOLOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "File[%s] closed"), 
		_strFile.c_str());
}

HANDLE NTFSIO::FindFirstFile(char* name, WIN32_FIND_DATAA& w)
{	
	HANDLE fileHandle = ::FindFirstFileA(name, &w);
	if(fileHandle == INVALID_HANDLE_VALUE) 
	{
		return INVALID_HANDLE_VALUE;
	}

	return fileHandle;
}

bool NTFSIO::FindNextFile(HANDLE hHandle, WIN32_FIND_DATAA& w)
{
	return ::FindNextFile(hHandle, &w);
}

void NTFSIO::FindClose(HANDLE hHandle)
{
	::FindClose(hHandle);
}

#else
NTFSIO::NTFSIO()
{
	_hFile = -1;
}


bool NTFSIO::Open(const char* szFile, int nOpenFlag)
{
	int nFlags = O_RDONLY;
	if (nOpenFlag == BF_READ)
		nFlags = O_RDONLY;
	else if(nOpenFlag == BF_WRITE)
		nFlags = O_WRONLY;
	else if(nOpenFlag == BF_READWRITE)
		nFlags = O_RDWR;
	else
	{
		MOLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Open falg[%d] not implemented"), 
			nOpenFlag);
		return false;
	}
		

	_hFile = open(szFile,nFlags);
	if(_hFile == -1)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "Failed to open file[%s] for read, errorcode[%d]"), 
			szFile, errno);
		return false;
	}
	
	MOLOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "Open file[%s] successful"), 
			szFile);

	_strFile = szFile;
	_bOpened = true;
	return true;
}

int64 NTFSIO::GetFileSize(const char* szFile)
{
	if (!_bOpened)
		return 0;

	int64 lRet = 0;
	
	struct stat fState;
	int ret = fstat(_hFile, &fState);
	if(ret != 0)
		MOLOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "Get file[%s] size failed code[%d]"),_strFile.c_str(),errno);
	else
		lRet = fState.st_size;	

	MOLOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "File[%s] size is [%lld] bytes"), 
		_strFile.c_str(), lRet);

	return lRet;
}

int NTFSIO::Read(char* pPtr, int nReadLen)
{
	if (!_bOpened)
		return 0;

	int retC = read(_hFile, (void*)pPtr, nReadLen);
	if(retC == -1)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "ReadFile[%s] failed, errorcode[%d]"), 
			_strFile.c_str(), errno);
		return 0;
	}
	
	return retC;
}

bool NTFSIO::Write(char* pPtr, int nWriteLen)
{
	if (!_bOpened)
		return false;

	int ret = write(_hFile, (void*)pPtr, nWriteLen);
	if(ret == -1)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "WriteFile[%s] failed, errorcode[%d]"), 
			_strFile.c_str(), errno);
		return false;
	}

	return true;
}

bool NTFSIO::Seek(int64 lOffset, int nPosFlag)
{
	if (!_bOpened)
		return false;

	int nWhence = SEEK_SET;
	if (nPosFlag==FP_CURRENT)
		nWhence = SEEK_CUR;
	else if(nPosFlag==FP_END)
		nWhence = SEEK_END;
	
	off_t ret = lseek(_hFile, lOffset,nWhence);
	if(ret == (off_t)-1)
	{
		MOLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "File[%s] failed to seek to offset[%lld] movemethod[%d]"), 
			_strFile.c_str(), lOffset, nWhence);
		return false;
	}
	
	MOLOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "File[%s] seek to offset[%lld] movemethod[%d] successful"), 
			_strFile.c_str(), lOffset, nWhence);
	return true;
}

void NTFSIO::Close()
{
	if(!_bOpened || _hFile == -1)
		return;
	close(_hFile);
	_hFile = -1;
	_bOpened = false;

	MOLOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "File[%s] closed"), 
		_strFile.c_str());
}

DIR* NTFSIO::openDirectory(const char* chName, DIR* dirStream)
{
	dirStream = opendir(chName);
	if(dirStream == NULL)
		MOLOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "Open diredctory[%s] failed code[%d]"), 
		chName, errno);
	
	return dirStream;	
}

struct dirent* NTFSIO::readDirectory(DIR* dirStream, struct dirent* dirNode)
{
	dirNode = readdir(dirStream);
	return dirNode;
}

bool NTFSIO::closeDirectory(DIR* dirStream)
{
	int ret = closedir(dirStream);
	return (ret == 0);
}

#endif

NTFSIO::~NTFSIO()
{

}

bool NTFSIO::Init()
{
	return true;
}

BaseIOI* NTFSIO::Create()
{
	return new NTFSIO();
}

void NTFSIO::Uninit()
{
}

bool NTFSIO::ReserveBandwidth(int nbps)
{

	return true;
}

void NTFSIO::ReleaseBandwidth()
{

}

int NTFSIO::getFileStats(char* name, FSUtils::fileInfo_t* info)
{

	return 0;
}


