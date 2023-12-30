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
#include "VstrmFileIoFactory.h"
#include "VstrmFileIo.h"
#include "ErrorCode.h"



#define MOLOG			(glog)
#define VsmIoFac			"VsmIoFac"


using namespace ZQ::common;

namespace ZQTianShan 
{
namespace ContentProvision
{
time_t VstrmFileIoFactory::FiletimeToTimet(FILETIME ft)
{
	LONGLONG ll = *((LONGLONG*)&ft);
	ll -= 116444736000000000;
	if (ll<=0)
		ll = 0;

	return (time_t)(ll/10000000);
}
VstrmFileIoFactory::VstrmFileIoFactory()
{
	_hVstrmClass = INVALID_HANDLE_VALUE;
	_uBandwidthClientId = 0;
		_bEnableCacheForIndex = true;
	_bEnableRAID1ForIndex = false;
//	_log = &NullLogger;
}

VstrmFileIoFactory::~VstrmFileIoFactory()
{
	uninitialize();
}

ZQ::common::Log* VstrmFileIoFactory::getLog()
{
	return &glog;
}

void VstrmFileIoFactory::setLog(ZQ::common::Log* pLog)
{
	ZQ::common::setGlogger(pLog);
}

FileIo* VstrmFileIoFactory::create()
{
	return new VstrmFileIo(this);
}

bool VstrmFileIoFactory::deleteFile(const char* szFileName)
{
	if(!_vstrmLoader.fpVstrmDeleteFile(_hVstrmClass, szFileName))
	{
		unsigned int vstat = _vstrmLoader.fpVstrmGetLastError();
		std::string errstr = getVstrmError(vstat);

		MOLOG(Log::L_WARNING, CLOGFMT(VsmIoFac, "VstrmDeleteFile(%s) failed with error: %s"),
			szFileName, errstr.c_str());
		return false;		
	}

	MOLOG(Log::L_INFO, CLOGFMT(VsmIoFac, "file %s was successfully deleted"), szFileName);
	return true;
}

bool VstrmFileIoFactory::moveFile(const char* szFileName, const char* szNewFileName)
{
	if(!_vstrmLoader.fpVstrmMoveFile(_hVstrmClass, szFileName, szNewFileName))
	{
		unsigned int vstat = _vstrmLoader.fpVstrmGetLastError();
		std::string errstr = getVstrmError(vstat);

		MOLOG(Log::L_WARNING, CLOGFMT(VsmIoFac, "VstrmMoveFile(%s) failed with error: %s"),
			szFileName, errstr.c_str());
		return false;		
	}

	MOLOG(Log::L_INFO, CLOGFMT(VsmIoFac, "file %s was successfully moved to %s"), szFileName, szNewFileName);
	return true;

}
LONGLONG VstrmFileIoFactory::getFileSize(const char* szFileName)
{
	DLL_FIND_DATA_LONG findData = {0};

	VHANDLE fileHandle =_vstrmLoader.fpVstrmFindFirstFileEx(_hVstrmClass, szFileName, &findData);
	if(fileHandle == INVALID_HANDLE_VALUE)
	{
		unsigned int vstat = _vstrmLoader.fpVstrmGetLastError();
		std::string errstr = getVstrmError(vstat);

		MOLOG(Log::L_WARNING, CLOGFMT(MOLOG, "VstrmFindFirstFileEx(%s) failed with error: %s"),
			szFileName, errstr.c_str());
		return 0;
	}

	_vstrmLoader.fpVstrmFindClose(_hVstrmClass, fileHandle);

	LARGE_INTEGER fileSize = {findData.w.nFileSizeLow, findData.w.nFileSizeHigh};
	return fileSize.QuadPart;
}

std::string VstrmFileIoFactory::getVstrmError(unsigned int status)
{
	char sErrorText[512]={0};
	_vstrmLoader.fpVstrmClassGetErrorText(_hVstrmClass, status, sErrorText, sizeof(sErrorText)-1);
	sErrorText[sizeof(sErrorText)-1] = '\0';

	char errcode[24];
	sprintf(errcode, "[0x%08x]", status);

	return std::string(sErrorText) + errcode;
}

void VstrmFileIoFactory::setLastError(const std::string& strErr, int errCode)
{
	_strErr = strErr;
	_errCode = errCode;
}

bool VstrmFileIoFactory::initialize()
{
	if (!_vstrmLoader.load())
	{
		return false;
	}

	VSTATUS vStatus = _vstrmLoader.fpVstrmClassOpenEx(&_hVstrmClass);
	if (vStatus != VSTRM_SUCCESS) 
	{
		std::string errstr = getVstrmError(vStatus);

		setLastError(std::string("VstrmClassOpenEx() Failed with error: ") + errstr, ERRCODE_VSTRM_API_ERROR);
		MOLOG(Log::L_ERROR, CLOGFMT(VsmIoFac, "VstrmClassOpenEx() Failed with error: %s)"), errstr.c_str());
		return false;
	} 

	if (_uBandwidthClientId)
	{
		vStatus = _vstrmLoader.fpVstrmClassReleaseAllBandwidth(_hVstrmClass, _uBandwidthClientId, 0);
	}

	return true;
}

void VstrmFileIoFactory::uninitialize()
{
	if(_hVstrmClass != INVALID_HANDLE_VALUE)
	{
		// It is not recommended to invoke VstrmClassCloseEx function, but seems for streaming
		// but in case of !bRunningOnMC, must invoked
		_vstrmLoader.fpVstrmClassCloseEx(_hVstrmClass);

		if (_uBandwidthClientId)
		{
			_vstrmLoader.fpVstrmClassReleaseAllBandwidth(_hVstrmClass, _uBandwidthClientId, 0);
		}
	}
}

HANDLE VstrmFileIoFactory::getVstrmClassHandle()
{
	return _hVstrmClass;
}

unsigned int VstrmFileIoFactory::getBandwidthManageClientId()
{
	return _uBandwidthClientId;
}

void VstrmFileIoFactory::setBandwidthManageClientId(unsigned int uClientId)
{
	_uBandwidthClientId = uClientId;
}

void VstrmFileIoFactory::getLastError(std::string& strErr, int& errCode)
{
	strErr = _strErr;
	errCode = _errCode;
}

bool VstrmFileIoFactory::getDisableBufDrvThrottle()
{
	return _bDisableBufDrvThrottle;
}

void VstrmFileIoFactory::setDisableBufDrvThrottle(bool bDisable)
{
	_bDisableBufDrvThrottle = bDisable;
}

VstrmLoader& VstrmFileIoFactory::getVstrmLoader()
{
	return _vstrmLoader;
}
bool VstrmFileIoFactory::getEnableCacheForIndex()
{
	return _bEnableCacheForIndex;
}

void VstrmFileIoFactory::setEnableCacheForIndex( bool bEnable )
{
	_bEnableCacheForIndex = bEnable;
}

bool VstrmFileIoFactory::getEnableRAID1ForIndex()
{
	return _bEnableRAID1ForIndex;
}

void VstrmFileIoFactory::setEnableRAID1ForIndex( bool bEnable )
{
	_bEnableRAID1ForIndex = bEnable;

	MOLOG(Log::L_DEBUG, CLOGFMT(VsmIoFac, "setEnableRAID1ForIndex to %s"),
		bEnable?"true":"false");
}

void VstrmFileIoFactory::findClose(HANDLE hfile)
{
	_vstrmLoader.fpVstrmFindClose(_hVstrmClass,hfile);
}

bool VstrmFileIoFactory::findNextFile( HANDLE hfile,WIN32_FIND_DATAA& w )
{
	DLL_FIND_DATA_LONG data = {0};
	if (!_vstrmLoader.fpVstrmFindNextFileEx(_hVstrmClass, hfile, &data))
	{
		return false;
	}
	w = data.w;
	return true;
}

HANDLE VstrmFileIoFactory::findFirstFile( char* name, WIN32_FIND_DATAA& w )
{
	DLL_FIND_DATA_LONG data = {0};
	VHANDLE fileHandle = _vstrmLoader.fpVstrmFindFirstFileEx(_hVstrmClass, name, &data);
	if(fileHandle == INVALID_HANDLE_VALUE) 
	{
		return INVALID_HANDLE_VALUE;
	}

	w = data.w;
	return fileHandle;
}

int VstrmFileIoFactory::getFileStats( char *filepath, FSUtils::fileInfo_t *infoptr )
{
	bool flagslashend = false;

	if (filepath == NULL || infoptr == NULL)
		return false;

	//remove any trailing slashes if necessary
	size_t len = strlen(filepath);
	if (len <= 0)
		return false;

	if (*(filepath+len-1) == '\\' || *(filepath+len-1) == '/')
	{
		//only remove the trailing slash if it is not the only slash
		if (strchr(filepath,'\\') != (filepath+len-1) && strchr(filepath,'/') != (filepath+len-1))
		{
			*(filepath+len-1) = '\0';
			flagslashend = true;
		}
	}

	const char* f = (*filepath == '\\' || *filepath == '/') ? filepath + 1 : filepath;
	DLL_FIND_DATA_LONG data = {0};
	VHANDLE fileHandle = _vstrmLoader.fpVstrmFindFirstFileEx(_hVstrmClass, f, &data);
	if(fileHandle == INVALID_HANDLE_VALUE) 
	{
		return false;
	}
	_vstrmLoader.fpVstrmFindClose(_hVstrmClass, fileHandle);

	LARGE_INTEGER fileSize = {data.w.nFileSizeLow, data.w.nFileSizeHigh};

	memset(infoptr,0,sizeof(FSUtils::fileInfo_t));

	infoptr->timeaccess = FiletimeToTimet(data.w.ftLastAccessTime);
	infoptr->timecreate = FiletimeToTimet(data.w.ftCreationTime);
	infoptr->timemod = FiletimeToTimet(data.w.ftLastWriteTime);
	infoptr->size = fileSize.QuadPart;
	infoptr->groupid = 0;
	infoptr->userid = 0;
	infoptr->mode = 0 | ~S_IFDIR;
	infoptr->devnum = 0;
	infoptr->inodnum = 0;   //only useful in UNIX
	infoptr->nlinks = 1;  //only useful in UNIX (always 1 in WINDOWS)

	if (flagslashend != 0)
		FSUtils::checkSlashEnd(filepath,len+1);   //restore the trailing slash
	return true;
}
}}