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

#include "Log.h"
#include "LinuxFileIo.h"
#include "ErrorCode.h"
#include "BaseClass.h"
#include "LinuxIoFactory.h"


#ifdef ZQ_OS_LINUX

#define MOLOG			(*_log)
#define NtfsIo			"NtfsIo"



using namespace ZQ::common;

namespace ZQTianShan 
{
namespace ContentProvision
{

LinuxFileIo::LinuxFileIo(LinuxIoFactory* pFileIoFactory)
	:FileIo(pFileIoFactory)
{
	_strPath = pFileIoFactory->getRootDir();
	_log = pFileIoFactory->getLog();
	_hOutputFile = NULL;
}

LinuxFileIo::~LinuxFileIo()
{
	closefile();
}

ZQ::common::Log* LinuxFileIo::getLog()
{
	return _log;
}

void LinuxFileIo::setLog(ZQ::common::Log* pLog)
{
	_log = pLog;
}

unsigned int LinuxFileIo::convertAccessCode(AccessMode accessMode)
{
	uint32 uAccessMode = 0;
	
	if (accessMode & ACCESS_READ)
	{
		uAccessMode |= GENERIC_READ;
	}

	if (accessMode & ACCESS_WRITE)
	{
		uAccessMode |= GENERIC_WRITE;
	}
	
	if (accessMode & ACCESS_EXECUTE)
	{
		uAccessMode |= GENERIC_EXECUTE;
	}

	if (accessMode & ACCESS_ALL)
	{
		uAccessMode |= GENERIC_ALL;
	}

	return uAccessMode;
}

unsigned int LinuxFileIo::convertShareMode(ShareMode shareMode)
{
	uint32 uShareMode = 0;

	if (shareMode & SHARE_READ)
	{
		uShareMode |= FILE_SHARE_READ;
	}

	if (shareMode & SHARE_WRITE)
	{
		uShareMode |= FILE_SHARE_WRITE;
	}

	if (shareMode & SHARE_DELETE)
	{
		uShareMode |= FILE_SHARE_DELETE;
	}

	return uShareMode;
}

unsigned int LinuxFileIo::convertCreationWay(CreationWay howToCreate)
{
	uint32 uCreationWay = 0;

	switch (howToCreate)
	{
	case WAY_CREATE_NEW:
		uCreationWay = CREATE_NEW;
		break;
	case WAY_CREATE_ALWAYS:
		uCreationWay = CREATE_ALWAYS;
		break;
	case WAY_OPEN_EXISTING:
		uCreationWay = OPEN_EXISTING;
		break;
	case WAY_OPEN_ALWAYS:
		uCreationWay = OPEN_ALWAYS;
		break;
	case WAY_TRUNCATE_EXISTING:
		uCreationWay = TRUNCATE_EXISTING;
		break;
	default:;
	};

	return uCreationWay;
}

unsigned int LinuxFileIo::convertFileAttrib(FileAttrib fileAttrib)
{
	return 0;
}

unsigned int LinuxFileIo::convertPosition(Position pos)
{
	if (pos == POS_BEGIN)
	{
		return FILE_BEGIN;
	}
	else if (pos == POS_CURRENT)
	{
		return FILE_CURRENT;
	}
	else
	{
		return FILE_END;
	}
}

bool LinuxFileIo::openfile(const char* szFileName, AccessMode accessMode,ShareMode shareMode,CreationWay howToCreate, FileAttrib fileAttrib)
{	
	_strLogHint = szFileName;
	_strFileName = _strPath + szFileName;

	LinuxIoFactory::createDirectoryForFile(_strFileName);

	_hOutputFile = CreateFile(_strFileName.c_str(),
		convertAccessCode(accessMode),
		convertShareMode(shareMode),
		NULL,
		convertCreationWay(howToCreate),
		NULL,
		NULL);
	if (_hOutputFile == INVALID_HANDLE_VALUE)
	{
		std::string errstr = LinuxIoFactory::getLastError();

		setLastError(std::string("CreateFile() failed with error: ") + errstr, ERRCODE_NTFS_CREATEFILE);

		MOLOG(Log::L_ERROR, CLOGFMT(NtfsIo, "[%s] Failed to create output file: %s with error: %s"),
			_strLogHint.c_str(), _strFileName.c_str(), errstr.c_str());

		return false;
	}
	
	return true;
}

bool LinuxFileIo::setOption()
{
	return true;
}

bool LinuxFileIo::readfile(char* pBuf, unsigned int bufLen, unsigned int& rcvLen)
{
	DWORD actualLen;
	if (!ReadFile(_hOutputFile, pBuf, bufLen, &actualLen, NULL))
	{
		std::string errstr = LinuxIoFactory::getLastError();

		setLastError(std::string("ReadFile() failed with error: ") + errstr, ERRCODE_NTFS_READFILE);

		MOLOG(Log::L_ERROR, CLOGFMT(NtfsIo, "[%s] Failed to ReadFile() with error: %s"),
			_strLogHint.c_str(), errstr.c_str());

		return false;
	}

	rcvLen = actualLen;
	return true;
}

bool LinuxFileIo::writefile(char* pBuf, unsigned int bufLen, unsigned int& writeLen)
{
	DWORD actualWriteLen;
	if (!WriteFile(_hOutputFile, pBuf, bufLen, &actualWriteLen,NULL))
	{
		std::string errstr = LinuxIoFactory::getLastError();

		setLastError(std::string("WriteFile() failed with error: ") + errstr, ERRCODE_NTFS_WRITEFILE);

		MOLOG(Log::L_ERROR, CLOGFMT(NtfsIo, "[%s] Failed to WriteFile() with error: %s"),
			_strLogHint.c_str(), errstr.c_str());

		return false;
	}

	writeLen = actualWriteLen;
	return true;
}

bool LinuxFileIo::seekfile(LONGLONG offset, Position pos)
{
	LONG lDistanceToMove = (LONG)offset;
	PLONG lpDistanceToMoveHigh = ((LONG*)&offset) + 1;

	DWORD dwRet = SetFilePointer(_hOutputFile,
		lDistanceToMove,
		lpDistanceToMoveHigh,
		convertPosition(pos));

	return (dwRet != 0xFFFFFFFF);
}

bool LinuxFileIo::setEndOfFile(LONGLONG offset)
{
	LONG lDistanceToMove = (LONG)offset;
	PLONG lpDistanceToMoveHigh = ((LONG*)&offset) + 1;

	DWORD dwRet = SetFilePointer(_hOutputFile,
		lDistanceToMove,
		lpDistanceToMoveHigh,
		FILE_BEGIN);
	if (dwRet == 0xFFFFFFFF)
		return false;

	SetEndOfFile(_hOutputFile);
	return true;
}

bool LinuxFileIo::closefile()
{
	if (_hOutputFile && _hOutputFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(_hOutputFile);
		_hOutputFile = NULL;			
	}
	return true;
}


//bandwidth management
bool LinuxFileIo::reserveBandwidth(unsigned int dwBandwidth)
{
	return true;
}

void LinuxFileIo::releaseBandwidth()
{

}


void LinuxFileIo::getLastError(std::string& strErr, int& errCode)
{
	strErr = _strErr;
	errCode = _errCode;
}

std::string LinuxFileIo::getFileName()
{
	return _strFileName;
}

void LinuxFileIo::setLastError(const std::string& strErr, int errCode)
{
	_strErr = strErr;
	_errCode = errCode;
}

std::string LinuxFileIo::getFileExtension()
{
	return _strFileExtension;
}

void LinuxFileIo::setFileExtension(const char* szFileExt)
{
	_strFileExtension = szFileExt;
}


}}

#endif

