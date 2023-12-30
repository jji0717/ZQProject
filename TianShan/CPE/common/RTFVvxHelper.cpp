// ===========================================================================
// Copyright (c) 2008 by
// ZQ Interactive, Inc., Shanghai, PRC.,
// All Rights Reserved.  Unpublished rights reserved under the copyright
// laws of the United States.
// 
// The software contained  on  this media is proprietary to and embodies the
// confidential technology of ZQ Interactive, Inc. Possession, use,
// duplication or dissemination of the software and media is authorized only
// pursuant to a valid written license from ZQ Interactive, Inc.
// 
// This software is furnished under a  license  and  may  be used and copied
// only in accordance with the terms of  such license and with the inclusion
// of the above copyright notice.  This software or any other copies thereof
// may not be provided or otherwise made available to  any other person.  No
// title to and ownership of the software is hereby transferred.
//
// The information in this software is subject to change without notice and
// should not be construed as a commitment by ZQ Interactive, Inc.
//
// Ident : 
// Branch: 
// Author: 
// Desc  : 
//
// Revision History: 
// ===========================================================================


#include "ZQ_common_conf.h"
//#include "vstrmtypes.h"
typedef INT64		QUADWORD;
typedef UINT64		UQUADWORD;

#include "vvx.h"
#include "RTFVvxHelper.h"
#include "BaseClass.h"
#include "NtfsFileIoFactory.h"


namespace ZQTianShan 
{
namespace ContentProvision
{


RTFVVXHelper::RTFVVXHelper()
{
	_pVvxWrite = NULL;
	_nErrorCode = 0;
	_cacheFile = 0;
	_vvxByteRead = 0;
	_posCacheRead = 0;
	_vvxByteRead = 0;
}

RTFVVXHelper::~RTFVVXHelper()
{
	close();
}

void RTFVVXHelper::setVvxWriteCB(VvxWriteCB* pCB)
{
	_pVvxWrite = pCB;
}

bool RTFVVXHelper::open(const char* keyname)
{
	if (FNSEPC != _strCachePath[_strCachePath.length()-1])
		_strCachePath += FNSEPS;

	_strCacheFile = _strCachePath + keyname + ".vvx";

	ZQTianShan::ContentProvision::NtfsFileIoFactory::createDirectoryForFile(_strCacheFile);
	_cacheFile = CreateFileA(_strCacheFile.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);		
	if (INVALID_HANDLE_VALUE == _cacheFile)
	{
		setLastError(std::string("open file ") + _strCacheFile);
		return false;
	}

	return true;
}

void RTFVVXHelper::close()
{
	if (_cacheFile || _cacheFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(_cacheFile);
		_cacheFile = NULL;
	}

	if (!_strCacheFile.empty())
	{
		DeleteFileA(_strCacheFile.c_str());
		_strCacheFile.clear();
	}
}

void RTFVVXHelper::setLastError(const char* strLogHeader, int nErrorCode)
{
	_nErrorCode = nErrorCode;

	std::string strErr;
	getSystemErrorText(strErr);

	char tmp[512];
	snprintf(tmp, sizeof(tmp), "%s failed with error: %s", strLogHeader, strErr.c_str());
	tmp[sizeof(tmp) - 1] = '\0';
	_strErr = tmp;
}

void RTFVVXHelper::setLastError(const std::string& strLogHeader, int nErrorCode)
{
	setLastError(strLogHeader.c_str(), nErrorCode);
}

bool RTFVVXHelper::write(char* pBuf, unsigned int nLen, unsigned int uOffetLow)
{
	DWORD dwPtr = SetFilePointer(_cacheFile, uOffetLow, 0, FILE_BEGIN);
	if (dwPtr == INVALID_SET_FILE_POINTER) // Test for failure
	{ 
		setLastError("seek");
		return false;
	}

	DWORD amountWritten = 0;
	if(!WriteFile(_cacheFile, pBuf, nLen, &amountWritten, NULL))
	{
		setLastError("write file");
		return false;
	}
	
	return checkVvxData();
}

bool RTFVVXHelper::checkVvxData()
{
	if(INVALID_SET_FILE_POINTER == SetFilePointer(_cacheFile, _posCacheRead, NULL, FILE_BEGIN))
	{
		return false;
	}

	DWORD byteRead = 0;
	char  tmpbuffer[64*1024];
	if(!ReadFile(_cacheFile, tmpbuffer, sizeof(tmpbuffer), &byteRead, NULL))
	{
		// log for fail
		return false;
	}

	// if we get a single terminator record, and the other writer is gone,
	// then return the terminator - the next read will be 0 bytes
	if(sizeof(VVX_V7_RECORD) == byteRead)
	{
		VVX_V7_RECORD *pRecord = (VVX_V7_RECORD *)(tmpbuffer);

		if(TRICK_INDEX == pRecord->recordType && 0 == pRecord->trick.timeCode.time)
		{
			if(INVALID_SET_FILE_POINTER == SetFilePointer(_cacheFile, _vvxByteRead, 0, FILE_BEGIN))
			{
				// log for fail
				return false;
			}
			else
			{
				return true;						
			}
		}
	}

	// not enough data found in first read
	if(0 == _vvxByteRead)
	{
		if(byteRead < sizeof(VVX_V7_INDEX_HEADER))
		{
			// initial read too shot
			return false;
		}
	}

	// there are two conditions where we'll adjust buffer length
	// and reset the NT file pointer
	// 1: last record is not a complete vvx index record
	// 2: last record is a terminator

	int iPartial;
	if(0 == _vvxByteRead)
	{
		// first read - offset is calculated from the frame data offset
		VVX_V7_INDEX_HEADER *pTmp = (VVX_V7_INDEX_HEADER *)tmpbuffer;

		iPartial = (byteRead - pTmp->frameDataOffset) % sizeof(VVX_V7_RECORD);
	}
	else
	{
		// not the first read - offset is calculated from the beginning of
		// the buffer because we always reset the file pointer to the start
		// of a record
		iPartial = byteRead % sizeof(VVX_V7_RECORD);
	}

	if(iPartial)
	{
		// reduce buffer length
		byteRead -= iPartial;

		// set file pointer
		_vvxByteRead += byteRead;

		if(INVALID_SET_FILE_POINTER == SetFilePointer(_cacheFile, _vvxByteRead, 0, FILE_BEGIN))
		{
			// log for fail
			return false;
		}

		// remember the position for reading
		_posCacheRead = _vvxByteRead;				
	}
	else
	{
		// check if the last record in the buffer is a terminator
		VVX_V7_RECORD *pRecord = 
			(VVX_V7_RECORD *)(tmpbuffer + byteRead - sizeof(VVX_V7_RECORD));

		if(TRICK_INDEX == pRecord->recordType && 
			0 == pRecord->trick.timeCode.time)
		{
			// gotcha
			byteRead -= sizeof(VVX_V7_RECORD);

			_vvxByteRead += byteRead;

			if(0xFFFFFFFF == SetFilePointer(_cacheFile, _vvxByteRead, 0, FILE_BEGIN))
			{
				// log for fail
				return false;
			}
			// remember the position for reading
			_posCacheRead = _vvxByteRead;
		}
		else
		{
			_vvxByteRead += byteRead;

			// remember the position for reading
			_posCacheRead = byteRead;
		}
	}

	// pass the buffer to callback
	if (byteRead && _pVvxWrite)
	{
		_pVvxWrite->writeVvx(tmpbuffer, byteRead);
	}

	return true;
}

void RTFVVXHelper::setCachePath(const char* szPath)
{
	_strCachePath = szPath; 
}

void RTFVVXHelper::getLastError(std::string& strErr, int& errCode)
{
	strErr = _strErr;
	errCode = _nErrorCode;
}



}
}

