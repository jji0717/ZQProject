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
#include "CStdFileIo.h"
#include "ErrorCode.h"
#include "BaseClass.h"
#include "TimeUtil.h"
#include "CStdFileIoFactory.h"

#ifdef ZQ_OS_MSWIN
#include "io.h"
#endif

#ifdef ZQ_OS_LINUX
#include<fcntl.h>
#endif

#define MOLOG			(*_log)
#define CStdfsIo			"CStdfsIo"



using namespace ZQ::common;

namespace ZQTianShan 
{
namespace ContentProvision
{

CStdFileIo::CStdFileIo(CStdFileIoFactory* pFileIoFactory)
	:FileIo(pFileIoFactory)
{
	_strPath = pFileIoFactory->getRootDir();
	_log = pFileIoFactory->getLog();
	pFileIoFactory->getFileSync(_maxSyncTimeout, _bytesToSync);
	_hOutputFile = NULL;
	_bDirectIO = false;
    _bytesToProcessSync = 0;
#ifdef ZQ_OS_LINUX
	_hOutputFileDirect = -1;	
#endif	
}

CStdFileIo::~CStdFileIo()
{
	closefile();
}

ZQ::common::Log* CStdFileIo::getLog()
{
	return _log;
}

void CStdFileIo::setLog(ZQ::common::Log* pLog)
{
	_log = pLog;
}
/*
std::string CStdFileIo::getNtfsError(unsigned int lastError)
{
	const int MAX_SYS_ERROR_TEXT = 256;
	char sErrorText[MAX_SYS_ERROR_TEXT+50]={0};

	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, 
		lastError,
		0,
		sErrorText, 
		MAX_SYS_ERROR_TEXT, 
		NULL);

	if (sErrorText[0])
	{
		char* pPtr = sErrorText + strlen(sErrorText) - 1;
		while(pPtr>=sErrorText && (*pPtr == 0x0d || *pPtr == 0x0a))
		{
			*pPtr = '\0';
			pPtr--;
		}
	}
	else
	{
		sprintf(sErrorText, "error code = [%d]", lastError);
	}

	return sErrorText;
}

std::string CStdFileIo::getNtfsError()
{
	DWORD lastError = GetLastError();
	return getNtfsError(lastError);
}
*/

unsigned int CStdFileIo::convertPosition(Position pos)
{
	if (pos == POS_BEGIN)
	{
		return SEEK_SET;
	}
	else if (pos == POS_CURRENT)
	{
		return SEEK_CUR;
	}
	else
	{
		return SEEK_END;
	}
}

bool CStdFileIo::openfile(const char* szFileName, AccessMode accessMode,ShareMode shareMode,CreationWay howToCreate, FileAttrib fileAttrib)
{	
	_strLogHint = szFileName;
	_strFileName = _strPath + szFileName;

    fixpath(_strFileName);

	CStdFileIoFactory::createDirectoryForFile(_strFileName);

	if(!_bDirectIO || !_bMainFile)
	{
		std::string strOpenMode = convertAccessMode(accessMode, shareMode, howToCreate, fileAttrib);

		_hOutputFile = fopen(_strFileName.c_str(), strOpenMode.c_str());
		if (!_hOutputFile)
		{
			std::string errstr = CStdFileIoFactory::getLastError();

			setLastError(std::string("CreateFile() failed with error: ") + errstr, ERRCODE_NTFS_CREATEFILE);

			MOLOG(Log::L_ERROR, CLOGFMT(CStdfsIo, "[%s] Failed to create output file: %s with error: %s"),
				_strLogHint.c_str(), _strFileName.c_str(), errstr.c_str());

			return false;
		}

		setvbuf( _hOutputFile, NULL, _IONBF, 0 );	
	}
	else
	{
#ifdef ZQ_OS_LINUX
		int OpenMode = convertAccessModeDirectIO(accessMode, shareMode, howToCreate, fileAttrib);

		MOLOG(Log::L_DEBUG, CLOGFMT(CStdfsIo, "[%s] open output file: %s with DirectIO mode"), _strLogHint.c_str(), _strFileName.c_str());

		if(OpenMode == O_WRONLY|O_CREAT)
		{
			_hOutputFileDirect = open(_strFileName.c_str(), OpenMode | O_DIRECT, S_IRWXU);
		}
		else
		{
          _hOutputFileDirect = open(_strFileName.c_str(), OpenMode | O_DIRECT );
		}
		

		if(_hOutputFileDirect < 0)
		{
			std::string errstr = CStdFileIoFactory::getLastError();

			setLastError(std::string("CreateFile() failed with error: ") + errstr, ERRCODE_NTFS_CREATEFILE);

			MOLOG(Log::L_ERROR, CLOGFMT(CStdfsIo, "[%s] Failed to create output file: %s with error: %s"),
				_strLogHint.c_str(), _strFileName.c_str(), errstr.c_str());
			return false;
		}
#else
		{
			setLastError(std::string("CreateFile() failed with error: invailed DirectIO"), ERRCODE_NTFS_CREATEFILE);
			return false;
		}
#endif 
	}
#ifdef ZQ_OS_LINUX
	MOLOG(Log::L_DEBUG, CLOGFMT(CStdfsIo, "[%s] set maxSyncTimeout[%dms] bytesToSync[%dbyte]"),
		_strLogHint.c_str(),  _maxSyncTimeout, _bytesToSync);
#endif

	_lFilesyncStart = ZQ::common::now();
	return true;
}

bool CStdFileIo::setOption()
{
	return true;
}

bool CStdFileIo::readfile(char* pBuf, unsigned int bufLen, unsigned int& rcvLen)
{
	int actualLen;
	if(!_bDirectIO || !_bMainFile)
	{
		actualLen = fread(pBuf, 1, bufLen, _hOutputFile);
		if (actualLen < 0)
		{
			std::string errstr = CStdFileIoFactory::getLastError();

			setLastError(std::string("ReadFile() failed with error: ") + errstr, ERRCODE_NTFS_READFILE);

			MOLOG(Log::L_ERROR, CLOGFMT(CStdfsIo, "[%s] Failed to ReadFile() with error: %s"),
				_strLogHint.c_str(), errstr.c_str());

			return false;
		}
	}
	else
	{
#ifdef ZQ_OS_LINUX
		actualLen = read(_hOutputFileDirect, pBuf, bufLen);
		if (actualLen < 0)
		{
			std::string errstr = CStdFileIoFactory::getLastError();

			setLastError(std::string("ReadFile() failed with error: ") + errstr, ERRCODE_NTFS_READFILE);

			MOLOG(Log::L_ERROR, CLOGFMT(CStdfsIo, "[%s] Failed to ReadFile() with error: %s"),
				_strLogHint.c_str(), errstr.c_str());

			return false;
		}
#else
		setLastError(std::string("ReadFile() failed with error: invailed DirectIO"), ERRCODE_NTFS_READFILE);

		MOLOG(Log::L_ERROR, CLOGFMT(CStdfsIo, "[%s] Failed to ReadFile() with error: invailed DirectIO"),
			_strLogHint.c_str());
		return false;
#endif 
	}

	rcvLen = actualLen;
	return true;
}

bool CStdFileIo::writefile(char* pBuf, unsigned int bufLen, unsigned int& writeLen)
{
	int actualWriteLen;
	if(!_bDirectIO || !_bMainFile)
	{
		actualWriteLen = fwrite(pBuf, 1, bufLen, _hOutputFile);
		if (actualWriteLen < bufLen)
		{
			std::string errstr = CStdFileIoFactory::getLastError();

			setLastError(std::string("WriteFile() failed with error: ") + errstr, ERRCODE_NTFS_WRITEFILE);

			MOLOG(Log::L_ERROR, CLOGFMT(CStdfsIo, "[%s] Failed to WriteFile() with error: %s"),
				_strLogHint.c_str(), errstr.c_str());

			return false;
		}
#ifdef ZQ_OS_LINUX
		if(_maxSyncTimeout >0 && _bytesToSync > 0)
		{
			_bytesToProcessSync += actualWriteLen;
			int fd  = -1;
			int64 timeout = ZQ::common::now() - _lFilesyncStart;
			if(_bytesToProcessSync >= _bytesToSync || timeout >= _maxSyncTimeout )
			{
				if(_bytesToProcessSync > 0)
				{
					int fd = fileno(_hOutputFile);
					if(fd == -1)
						return false;
					fsync(fd);

					MOLOG(Log::L_DEBUG, CLOGFMT(CStdfsIo, "[%s] fsync file, timeout[%lldms]bytesToProcess[%dbyte]"),
						_strLogHint.c_str(), timeout, _bytesToProcessSync);
				}
				_bytesToProcessSync = 0;
				_lFilesyncStart = ZQ::common::now();
			}
		}
#endif
	}
	else{
#ifdef ZQ_OS_LINUX
		static int64 filesize = 0;
		unsigned int addByte = 4096 - (bufLen % 4096);
		if(addByte != 4096)
		{
			int64 fileOffset = filesize + bufLen;
			MOLOG(Log::L_DEBUG, CLOGFMT(CStdfsIo, "[%s] WriteFile() Len[%d] addByte[%d] EndFileoffset[%lld]"),
				_strLogHint.c_str(), bufLen, addByte, fileOffset);

			char* pDirectIOOriginalBuffer = (char*)malloc(bufLen + 4096*2);
			if(!pDirectIOOriginalBuffer)
			{
				MOLOG(Log::L_ERROR, CLOGFMT(CStdfsIo, "[%s] Failed to create output file: %s with error: failed to malloc memory"),
					_strLogHint.c_str(), _strFileName.c_str());
				return false;
			}
			size_t offset  = 4096 - (pDirectIOOriginalBuffer - (char*)NULL) % 4096; 
			if(offset == 4096)
				offset = 0;
			char* pDirectIOBuffer = pDirectIOOriginalBuffer + offset;
 
            memcpy(pDirectIOBuffer, pBuf, bufLen);
			memset(pDirectIOBuffer+ bufLen, 0xff, addByte);
            
			actualWriteLen = write(_hOutputFileDirect, pDirectIOBuffer, bufLen + addByte);

			free(pDirectIOOriginalBuffer);

			if(actualWriteLen == bufLen + addByte)
			{
				MOLOG(Log::L_DEBUG, CLOGFMT(CStdfsIo, "[%s] WriteFile() adjust acctualWriteLen[%d]"),
					_strLogHint.c_str(), bufLen + addByte );
				actualWriteLen =  bufLen;
			}
			
			if( ftruncate(_hOutputFileDirect, fileOffset) != 0 )
			{
				MOLOG(Log::L_ERROR, CLOGFMT(CStdfsIo, "[%s] write file with error: failed to ftruncate to offset[%lld]"),
					_strLogHint.c_str(), fileOffset);
				return false;
			}
		}
		else
		{
		   actualWriteLen = write(_hOutputFileDirect, pBuf, bufLen);
		}

		if (actualWriteLen < 0)
		{
			std::string errstr = CStdFileIoFactory::getLastError();

			setLastError(std::string("WriteFile() failed with error: ") + errstr, ERRCODE_NTFS_READFILE);

			MOLOG(Log::L_ERROR, CLOGFMT(CStdfsIo, "[%s] Failed to WriteFile() with error: %s"),
				_strLogHint.c_str(), errstr.c_str());

			return false;
		}
		filesize+= actualWriteLen;
//		MOLOG(Log::L_DEBUG, CLOGFMT(CStdfsIo, "[%s] WriteFile() Len[%d] actualWriteLen[%d] currentfilesize[%lld]"),
//			_strLogHint.c_str(), bufLen, actualWriteLen, filesize);

#else
		setLastError(std::string("WriteFile() failed with error: invailed DirectIO"), ERRCODE_NTFS_READFILE);

		MOLOG(Log::L_ERROR, CLOGFMT(CStdfsIo, "[%s] Failed to WriteFile() with error: invailed DirectIO"),
			_strLogHint.c_str());
		return false;
#endif
	}
	writeLen = actualWriteLen;
	return true;
}

bool CStdFileIo::seekfile(int64 offset, Position pos)
{
#ifdef ZQ_OS_MSWIN
	DWORD dwRet = _fseeki64(_hOutputFile, 
		offset,		
		convertPosition(pos));

	return (dwRet != 0xFFFFFFFF);
#else
    if(!_bDirectIO || !_bMainFile)
	{
		int fd = fileno(_hOutputFile);
		if(fd == -1)
			return false;

		off64_t fpos = lseek64(fd,offset,pos);
		return (fpos != (off64_t)-1);
	}
	else
	{
		off64_t fpos = lseek64(_hOutputFileDirect,offset,pos);
		return (fpos != (off64_t)-1);
	}
#endif
}

bool CStdFileIo::setEndOfFile(int64 offset)
{
	fflush(_hOutputFile);
#ifdef ZQ_OS_MSWIN
	LONG dwLow = offset & 0xffffffff;
	LONG dwHigh = offset >>32;

	HANDLE h = (HANDLE)_get_osfhandle (_fileno (_hOutputFile));

	::SetFilePointer(h, dwLow, &dwHigh, FILE_BEGIN);
	::SetEndOfFile(h);

//	if (_chsize_s(fileno(_hOutputFile), offset))
//		return false;
#else
	if(!_bDirectIO || !_bMainFile)
	{
		if( ftruncate(fileno(_hOutputFile), offset) != 0 )
			return false;
	}
	else
	{
		if( ftruncate(_hOutputFileDirect, offset) != 0 )
			return false;
	}
#endif

	return true;
}

bool CStdFileIo::closefile()
{
	if(!_bDirectIO || !_bMainFile)
	{
		if (_hOutputFile)
		{
			fclose(_hOutputFile);
			_hOutputFile = NULL;			
		}
	}
	else
	{
#ifdef ZQ_OS_LINUX
		if(_hOutputFileDirect >=0)
		{
          close(_hOutputFileDirect);
		  _hOutputFileDirect =  -1;
		}
#endif
	}
	return true;
}

bool CStdFileIo::enableSparse()
{
#ifdef ZQ_OS_MSWIN
	DWORD dwTemp;
	HANDLE h = (HANDLE)_get_osfhandle (_fileno (_hOutputFile));

	DeviceIoControl(h, 
		FSCTL_SET_SPARSE,
		NULL,
		0,
		NULL,
		0,
		&dwTemp,
		NULL);

	BY_HANDLE_FILE_INFORMATION bhfi;
	::GetFileInformationByHandle(h, &bhfi);
	return (bhfi.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE) ? true : false;

#endif
	return true;
}


//bandwidth management
bool CStdFileIo::reserveBandwidth(unsigned int dwBandwidth)
{
	return true;
}

void CStdFileIo::releaseBandwidth()
{

}


void CStdFileIo::getLastError(std::string& strErr, int& errCode)
{
	strErr = _strErr;
	errCode = _errCode;
}

std::string CStdFileIo::getFileName()
{
	return _strFileName;
}

void CStdFileIo::setLastError(const std::string& strErr, int errCode)
{
	_strErr = strErr;
	_errCode = errCode;
}

std::string CStdFileIo::getFileExtension()
{
	return _strFileExtension;
}

void CStdFileIo::setFileExtension(const char* szFileExt)
{
	_strFileExtension = szFileExt;
}

std::string CStdFileIo::convertAccessMode( AccessMode accessMode, ShareMode shareMode, CreationWay howToCreate, FileAttrib fileAttrib )
{
	std::string strMode;
	if (accessMode & ACCESS_READ && accessMode & ACCESS_WRITE)
	{
		strMode = "r+b";
	}
	else if (accessMode & ACCESS_READ)
	{
		strMode = "rb";
	}
	else if (accessMode & ACCESS_WRITE)
	{
		strMode = "wb";
	}
	else if(accessMode & ACCESS_APPEND)
	{
		strMode = "ab";
	}

	return strMode;
}
#ifdef ZQ_OS_LINUX
int CStdFileIo::convertAccessModeDirectIO( AccessMode accessMode, ShareMode shareMode, CreationWay howToCreate, FileAttrib fileAttrib )
{
	int fileMode;
	if (accessMode & ACCESS_READ && accessMode & ACCESS_WRITE)
	{
		fileMode =  O_RDWR;
	}
	else if (accessMode & ACCESS_READ)
	{
		fileMode = O_RDONLY;
	}
	else if (accessMode & ACCESS_WRITE)
	{
		fileMode = O_WRONLY|O_CREAT;
	}

	return fileMode;
}
#endif
                                         
bool CStdFileIo::fixpath( std::string& filename )
{
	char* pathbuf = new char[filename.length() +2];
	if (NULL ==pathbuf)
		return false;

	strcpy(pathbuf, filename.c_str());
	pathbuf[filename.length()] = '\0';

	for (char* p = pathbuf; *p; p++)
	{
		if ('\\' == *p || '/' == *p)
			*p = FNSEPC;
	}
	filename = pathbuf;

	delete []pathbuf;
	return true;
}


}}

