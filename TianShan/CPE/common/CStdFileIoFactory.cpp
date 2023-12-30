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
#include "CStdFileIoFactory.h"
#include "CStdFileIo.h"
#include "strHelper.h"
#include "errno.h"
#ifdef ZQ_OS_MSWIN
#include "direct.h"
#include "io.h"
#else
extern "C"
{
#include <sys/stat.h>
#include <sys/types.h>
}
#endif

namespace ZQTianShan 
{
namespace ContentProvision
{

CStdFileIoFactory::CStdFileIoFactory()
{
   _bDirectIO = false;
   _bytesToSync =  -1;
   _maxSyncTimeout = -1;
}

CStdFileIoFactory::~CStdFileIoFactory()
{
	uninitialize();
}

ZQ::common::Log* CStdFileIoFactory::getLog()
{
	return _log;
}

void CStdFileIoFactory::setLog(ZQ::common::Log* pLog)
{
	_log = pLog;
}

FileIo* CStdFileIoFactory::create()
{
	 CStdFileIo* pCstdFileIo =  new CStdFileIo(this);
	 if(pCstdFileIo)
		 pCstdFileIo->setDirectIO(_bDirectIO);
	 return pCstdFileIo;
}

bool CStdFileIoFactory::deleteFile(const char* szFileName)
{
	return (remove(szFileName) == 0);
}

bool CStdFileIoFactory::moveFile(const char* szFileName, const char* szNewFileName)
{
	return rename(szFileName, szNewFileName);
}

int64 CStdFileIoFactory::getFileSize(const char* szFileName)
{
#ifdef ZQ_OS_MSWIN
	std::string filename = _strRootPath + std::string(szFileName);
	FILE* fp = fopen(filename.c_str(), "rb");
	if (!fp)
		return 0;

	int64 ret = _filelengthi64(fileno(fp));
	fclose(fp);
	return ret;
#else
	struct stat fstatbuf;
    int rt = stat(szFileName,&fstatbuf );
    if(rt != 0)
        return 0;

    return fstatbuf.st_size;
#endif
}

bool CStdFileIoFactory::initialize()
{
	return true;
}

void CStdFileIoFactory::uninitialize()
{

}

void CStdFileIoFactory::setRootDir(const char* szDir)
{
	_strRootPath = szDir;
    
	if (_strRootPath.length() > 0 &&  FNSEPC != _strRootPath[_strRootPath.length()-1])
		_strRootPath += FNSEPS;
}

std::string CStdFileIoFactory::getRootDir()
{
	return _strRootPath;
}

void CStdFileIoFactory::getLastError(std::string& strErr, int& errCode)
{
	strErr = _strErr;
	errCode = _errCode;
}

void CStdFileIoFactory::setLastError(const std::string& strErr, int errCode)
{
	_strErr = strErr;
	_errCode = errCode;
}

std::string CStdFileIoFactory::getErrorText( unsigned int errCode )
{
	const int MAX_SYS_ERROR_TEXT = 256;
	char sErrorText[MAX_SYS_ERROR_TEXT+50]={0};

#ifdef ZQ_OS_MSWIN
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, 
		errCode,
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
		sprintf(sErrorText, "error code = [%d]", errCode);
	}
#else
 	char* perr = strerror_r(errCode, sErrorText,sizeof(sErrorText));
    std::string strErr = perr;
    return strErr;
#endif

	return sErrorText;
}

std::string CStdFileIoFactory::getLastError()
{
	int nErrorNum = 0;

#ifdef ZQ_OS_MSWIN
	nErrorNum = GetLastError();
#else
	nErrorNum = errno;
#endif

	return getErrorText(nErrorNum);
}

bool CStdFileIoFactory::createDirectoryForFile(const std::string& strFilename)
{
	//get path of file
	std::string strDirectory = getPathOfFile(strFilename);	
	return createDirectory(strDirectory.c_str());
}

bool CStdFileIoFactory::createDirectory(const std::string& strDirectory)
{
	if (strDirectory.empty())
		return true;

	std::string subPath = strDirectory;
	std::vector<std::string> pathToCreate;

	while(true) 
	{
#ifdef ZQ_OS_MSWIN
		if(mkdir(subPath.c_str()) == 0)
#else
		if(mkdir(subPath.c_str(), 0755) == 0)
#endif
		{
			break;
		}

		if(errno == ENOENT) 
		{
			pathToCreate.push_back(subPath);
			subPath = ZQ::common::stringHelper::rsplit(subPath, FNSEPC, 1).at(0);		

			continue;
		}
		break;
	}

	std::vector<std::string>::reverse_iterator iter = pathToCreate.rbegin();
	for(; iter != pathToCreate.rend(); ++iter)
	{
#ifdef ZQ_OS_MSWIN
		int rt = mkdir((*iter).c_str());
#else
		int rt = mkdir((*iter).c_str(), 0755);
#endif
        if(rt != 0 && errno != EEXIST)
			return false;
	}

	return true;
}

std::string CStdFileIoFactory::getPathOfFile(const std::string& strFilename)
{
	std::string strPath;
	std::string::size_type pos = strFilename.find_last_of(FNSEPS);
	if (pos != std::string::npos)
		strPath.assign(strFilename, 0, pos);
	
	return strPath;
}
#ifdef ZQ_OS_MSWIN
void CStdFileIoFactory:: findClose(HANDLE hfile)
{

}
bool CStdFileIoFactory::findNextFile(HANDLE hfile,WIN32_FIND_DATAA& w)
{
  return false;
}
HANDLE CStdFileIoFactory::findFirstFile(char* name, WIN32_FIND_DATAA& w)
{
  return NULL;
}
#endif
int CStdFileIoFactory::getFileStats(char *filepath, FSUtils::fileInfo_t *infoptr)
{
  return 0;
}
}}

