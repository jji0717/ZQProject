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
#include "AquaFileIo.h"
#include "ErrorCode.h"
#include "BaseClass.h"
#include "TimeUtil.h"
#include "AquaFileIoFactory.h"
#include <CdmiFuseOps.h>

#ifdef ZQ_OS_MSWIN
#include "io.h"
#endif

#ifdef ZQ_OS_LINUX
#include<fcntl.h>
#endif

#define MOLOG			    (*_log)
#define AquafsIo			"AquafsIo"

using namespace ZQ::common;

namespace ZQTianShan 
{
namespace ContentProvision
{

AquaFileIo::AquaFileIo(AquaFileIoFactory* pFileIoFactory)
	:FileIo(pFileIoFactory)
{
	_log = pFileIoFactory->getLog();
	_hOutputFile = NULL;
#ifdef ZQ_OS_LINUX
	_hOutputFileDirect = -1;	
#endif	

	_pCdmiOps = pFileIoFactory->getCdmiOps();
}

AquaFileIo::~AquaFileIo()
{
	closefile();
}

ZQ::common::Log* AquaFileIo::getLog()
{
	return _log;
}

void AquaFileIo::setLog(ZQ::common::Log* pLog)
{
	_log = pLog;
}
/*
std::string AquaFileIo::getNtfsError(unsigned int lastError)
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

std::string AquaFileIo::getNtfsError()
{
	DWORD lastError = GetLastError();
	return getNtfsError(lastError);
}
*/

unsigned int AquaFileIo::convertPosition(Position pos)
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
bool AquaFileIo::openfile(const std::string& fileName)
{
	int64 lStart =  ZQ::common::now(); 

	_strFileName = _pCdmiOps->pathToUri(fileName);

	std::string uri = _pCdmiOps->pathToUri(fileName) + "?metadata";
	std::string location;
	Json::Value value;

	CdmiFuseOps::CdmiRetCode retCode = _pCdmiOps->cdmi_ReadDataObject(value, uri , location);

	if(!CdmiRet_SUCC(retCode))
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaFileIo, "failed to get file[%s]with error[%d==>%s]"), fileName.c_str(), retCode, CdmiFuseOps::cdmiRetStr(retCode));
		return false;
	}
	_fileOffset = 0;
	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(AquaFileIo, "open file[%s] successfully took %ldms"),
		fileName.c_str(), (ZQ::common::now() - lStart));
	return true;
}
bool AquaFileIo::openfile(const char* szFileName, AccessMode accessMode,ShareMode shareMode,CreationWay howToCreate, FileAttrib fileAttrib)
{	
    MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(AquaFileIo, "openfile() file[%s]"), szFileName);

	if(accessMode == FileIo::ACCESS_READ)
		return openfile(szFileName);

    //std::string strOpenMode = convertAccessMode(accessMode, shareMode, howToCreate, fileAttrib);
    int64 lStart =  ZQ::common::now(); 
    
    std::string contentType = "";
    std::string value = "";
    _strFileName = _pCdmiOps->pathToUri(szFileName);
    CdmiFuseOps::CdmiRetCode retCode = _pCdmiOps->nonCdmi_CreateDataObject(_strFileName, "");

    if(!CdmiRet_SUCC(retCode))
    {
        MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaFileIo, "openfile() failed to create file[%s] with errorCode[%d:%s]"), _strFileName.c_str(), retCode, CdmiFuseOps::cdmiRetStr(retCode));
		char buf[1024];
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "openfile() failed to create file[%s] with errorCode[%d:%s]", _strFileName.c_str(), retCode, CdmiFuseOps::cdmiRetStr(retCode));
		_strErr = buf;
		_errCode = ERRCODE_AQUA_CREATEFILE;
        return false;
    }

    _fileOffset = 0;
    MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(AquaFileIo, "openfile() create file[%s] successfully took %ldms"),
        _strFileName.c_str(), (ZQ::common::now() - lStart));

	return true;
}

bool AquaFileIo::setOption()
{
	return true;
}

bool AquaFileIo::readfile(char* pBuf, unsigned int bufLen, unsigned int& rcvLen)
{
    MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(AquaFileIo, "readfile() file[%s] "), _strFileName.c_str());

    if (_strFileName.empty())
    {
        MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaFileIo, "readfile failed, filename is empty, please create file firstly"));
        return false;
    }

    int64 lStart =  ZQ::common::now(); 

    std::string contentType = "";
    std::string location = "";

    rcvLen = bufLen;
    CdmiFuseOps::CdmiRetCode retCode = _pCdmiOps->nonCdmi_ReadDataObject(_strFileName, contentType, location, _fileOffset, rcvLen, pBuf, true);

    if(!CdmiRet_SUCC(retCode))
    {
        MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(AquaFileIo, "readFile() failed to read file[%s] with errorCode[%d]"), _strFileName.c_str(), retCode);
		char buf[1024];
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "writefile() failed to write file[%s] with error[%d,%s]", _strFileName.c_str(), retCode, CdmiFuseOps::cdmiRetStr(retCode));
		_strErr = buf;
		_errCode = ERRCODE_AQUA_READFILE;
        return false;
    }

    _fileOffset += rcvLen;
    MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(AquaFileIo, "readFile() read file[%s] recv size[%ld] successfully took %dms"),
        _strFileName.c_str(), rcvLen, (int)(ZQ::common::now() - lStart));
	return true;
}

bool AquaFileIo::writefile(char* pBuf, unsigned int bufLen, unsigned int& writeLen)
{
    MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(AquaFileIo, "writefile() file[%s] "), _strFileName.c_str());

    if (_strFileName.empty())
    {
        MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(AquaFileIo, "writefile failed, filename is empty, please create file firstly"));
        return false;
    }

    int64 lStart =  ZQ::common::now(); 
    std::string contentType = "";
    std::string location = "";
    writeLen = bufLen;
	CdmiFuseOps::CdmiRetCode retCode = _pCdmiOps->nonCdmi_UpdateDataObject(_strFileName, location, contentType, _fileOffset, writeLen, pBuf, -1, false);

    if(!CdmiRet_SUCC(retCode))
    {
        MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(AquaFileIo, "writefile() failed to write file[%s] with errorCode[%d]"), _strFileName.c_str(), retCode);
		char buf[1024];
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "writefile() failed to write file[%s] with error[%d,%s]", _strFileName.c_str(), retCode, CdmiFuseOps::cdmiRetStr(retCode));
		_strErr = buf;
		_errCode = ERRCODE_AQUA_WRITEFILE;
        return false;
    }

    _fileOffset += writeLen;
    MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(AquaFileIo, "writefile() write file[%s] filesize[%d] successfully took %dms"),
        _strFileName.c_str(), writeLen, (int)(ZQ::common::now() - lStart));
    return true;
}

bool AquaFileIo::seekfile(int64 offset, Position pos)
{
    if (pos == POS_BEGIN)
    {
        _fileOffset = offset;
    }else if (pos == POS_CURRENT)
    {
        _fileOffset += offset;
    }else if (pos == POS_END)
    {
       int64 fileSize = _pFileIoFactory->getFileSize(_strFileName.c_str());
       _fileOffset = fileSize - offset - 1;     
    }
    return true;
}

bool AquaFileIo::setEndOfFile(int64 offset)
{
	return true;
}

bool AquaFileIo::closefile()
{
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(AquaFileIo, "[%s]closefile() file"), _strFileName.c_str());

	CdmiFuseOps::CdmiRetCode retFlushdata = _pCdmiOps->flushdata(_strFileName);

	CdmiFuseOps::CdmiRetCode retIndicateClose = _pCdmiOps->cdmi_IndicateClose(_strFileName);

	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(AquaFileIo, "[%s]flushdata [%d=>%s] cdmi_IndicateClose[%d=>%s]"), 
		_strFileName.c_str(), retFlushdata, CdmiFuseOps::cdmiRetStr(retFlushdata), retIndicateClose,CdmiFuseOps::cdmiRetStr(retIndicateClose) );
	return true;
}

bool AquaFileIo::enableSparse()
{
	return false;
}


//bandwidth management
bool AquaFileIo::reserveBandwidth(unsigned int dwBandwidth)
{
	return true;
}

void AquaFileIo::releaseBandwidth()
{

}


void AquaFileIo::getLastError(std::string& strErr, int& errCode)
{
	strErr = _strErr;
	errCode = _errCode;
}

std::string AquaFileIo::getFileName()
{
	return _strFileName;
}

void AquaFileIo::setLastError(const std::string& strErr, int errCode)
{
	_strErr = strErr;
	_errCode = errCode;
}

std::string AquaFileIo::getFileExtension()
{
	return _strFileExtension;
}

void AquaFileIo::setFileExtension(const char* szFileExt)
{
	_strFileExtension = szFileExt;
}

std::string AquaFileIo::convertAccessMode( AccessMode accessMode, ShareMode shareMode, CreationWay howToCreate, FileAttrib fileAttrib )
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
int AquaFileIo::convertAccessModeDirectIO( AccessMode accessMode, ShareMode shareMode, CreationWay howToCreate, FileAttrib fileAttrib )
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
                                         
bool AquaFileIo::fixpath( std::string& filename )
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

