
#include "CECommon.h"
#include "NASIO.h"


#define LOG_MODULE_NAME			"NasIo"

using namespace ZQ::common;

#define MOLOG		glog


NASIO::NASIO()
{
	_hFile = NULL;
}

NASIO::~NASIO()
{

}

bool NASIO::Open(const char* szFile, int nOpenFlag)
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

LONGLONG NASIO::GetFileSize()
{
	if (!_bOpened)
		return 0;

	LONGLONG lRet;
	DWORD* pdwLow = (DWORD*)&lRet;
	DWORD* pdwHigh = pdwLow + 1;
	*pdwLow = ::GetFileSize(_hFile, pdwHigh);

	MOLOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "File[%s] size is [%lld] bytes"), 
		_strFile.c_str(), lRet);

	return lRet;
}

int NASIO::Read(char* pPtr, int nReadLen)
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

bool NASIO::Write(char* pPtr, int nWriteLen)
{
	if (!_bOpened)
		return false;

	DWORD dwWritten = 0;
	if (!::WriteFile(_hFile, pPtr, nWriteLen, &dwWritten, 0))
	{
		MOLOG(Log::L_ERROR, CLOGFMT(LOG_MODULE_NAME, "WriteFile[%s] failed, errorcode[%d]"), 
			_strFile.c_str(), GetLastError());
	}

	return true;
}

bool NASIO::Seek(LONGLONG lOffset, int nPosFlag)
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

void NASIO::Close()
{
	if (!_bOpened || !_hFile || _hFile==INVALID_HANDLE_VALUE)
		return;

	CloseHandle(_hFile);
	MOLOG(Log::L_INFO, CLOGFMT(LOG_MODULE_NAME, "File[%s] closed"), 
		_strFile.c_str());
}

bool NASIO::ReserveBandwidth(int nbps)
{

	return true;
}

void NASIO::ReleaseBandwidth()
{

}
