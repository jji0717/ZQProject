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
#include "LinuxIoFactory.h"
#include "LinuxFileIo.h"
#include "strHelper.h"

#ifdef ZQ_OS_LINUX

namespace ZQTianShan 
{
namespace ContentProvision
{

LinuxIoFactory::LinuxIoFactory()
{

}

LinuxIoFactory::~LinuxIoFactory()
{
	uninitialize();
}

ZQ::common::Log* LinuxIoFactory::getLog()
{
	return _log;
}

void LinuxIoFactory::setLog(ZQ::common::Log* pLog)
{
	_log = pLog;
}

FileIo* LinuxIoFactory::create()
{
	return new LinuxFileIo(this);
}

bool LinuxIoFactory::deleteFile(const char* szFileName)
{
	return DeleteFileA(szFileName);
}

LONGLONG LinuxIoFactory::getFileSize(const char* szFileName)
{
	HANDLE hFile=CreateFileA(szFileName,
		GENERIC_READ,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if(hFile==INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	LONGLONG ret;
	*((DWORD*)&ret) = GetFileSize(hFile, ((DWORD*)&ret) + 1);

	CloseHandle(hFile);
	return ret;
}

bool LinuxIoFactory::initialize()
{
	return true;
}

void LinuxIoFactory::uninitialize()
{

}

void LinuxIoFactory::setRootDir(const char* szDir)
{
	_strRootPath = szDir;

	if (FNSEPC != _strRootPath[_strRootPath.length()-1])
		_strRootPath += FNSEPS;
}

std::string LinuxIoFactory::getRootDir()
{
	return _strRootPath;
}

void LinuxIoFactory::getLastError(std::string& strErr, int& errCode)
{
	strErr = _strErr;
	errCode = _errCode;
}

void LinuxIoFactory::setLastError(const std::string& strErr, int errCode)
{
	_strErr = strErr;
	_errCode = errCode;
}

bool LinuxIoFactory::createDirectoryForFile(const std::string& strFilename)
{
	//get path of file
	std::string strDirectory = getPathOfFile(strFilename);	
	return createDirectory(strDirectory.c_str());
}

bool LinuxIoFactory::createDirectory(const std::string& strDirectory)
{
	if (strDirectory.empty())
		return true;

	std::string subPath = strDirectory;
	std::vector<std::string> pathToCreate;

	while(true) 
	{
		if(CreateDirectoryA(subPath.c_str(), 0)) 
		{
			break;
		}

		if(GetLastError() == ERROR_PATH_NOT_FOUND) 
		{
			pathToCreate.push_back(subPath);
			subPath = ZQ::common::stringHelper::rsplit(subPath, '\\', 1).at(0);		

			continue;
		}
		break;
	}

	std::vector<std::string>::reverse_iterator iter = pathToCreate.rbegin();
	for(; iter != pathToCreate.rend(); ++iter)
	{
		CreateDirectoryA((*iter).c_str(), 0);
	}

	return true;
}

std::string LinuxIoFactory::getPathOfFile(const std::string& strFilename)
{
	std::string strPath;
	std::string::size_type pos = strFilename.find_last_of(FNSEPS);
	if (pos != std::string::npos)
		strPath.assign(strFilename, 0, pos);
	
	return strPath;
}

std::string LinuxIoFactory::getErrorText( unsigned int errCode )
{
	std::string strErr;
	return strErr;
}

std::string LinuxIoFactory::getLastError()
{
	int nErrNum = 0;
	return getErrorText(nErrNum);
}

}}

#endif