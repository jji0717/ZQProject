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
#include "VstrmFileIo.h"
#include "vstrmuser.h"
#include "vsiolib.h"
#include "ErrorCode.h"
#include "VstrmFileIoFactory.h"
#include "VstrmLoader.h"
#include "TimeUtil.h"

#define MOLOG			(*_log)
#define VstrmIo			"VstrmIo"

#define VSTRM_MAX_IO_SIZE		64*1024



using namespace ZQ::common;

namespace ZQTianShan 
{
namespace ContentProvision
{

VstrmFileIo::VstrmFileIo(VstrmFileIoFactory* pFileIoFactory)
	:FileIo(pFileIoFactory), _vstrmLoader(pFileIoFactory->getVstrmLoader())
{
	_hVstrm = pFileIoFactory->getVstrmClassHandle();
	_bwmgrClientId = pFileIoFactory->getBandwidthManageClientId();
	_bDisableBufDrvThrottle = pFileIoFactory->getDisableBufDrvThrottle();
	_log = pFileIoFactory->getLog();
	
	_hOutputFile = INVALID_HANDLE_VALUE;
	_objectId = 0;
	_dwBandwidth = 0;
	_bwTicket = 0;
	_errCode = 0;
}

VstrmFileIo::~VstrmFileIo()
{
	closefile();
}

ZQ::common::Log* VstrmFileIo::getLog()
{
	return _log;
}

void VstrmFileIo::setLog(ZQ::common::Log* pLog)
{
	_log = pLog;
}

unsigned int VstrmFileIo::convertAccessCode(AccessMode accessMode)
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

unsigned int VstrmFileIo::convertShareMode(ShareMode shareMode)
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

unsigned int VstrmFileIo::convertCreationWay(CreationWay howToCreate)
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

unsigned int VstrmFileIo::convertFileAttrib(FileAttrib fileAttrib)
{
	uint32 uFileAttrib = 0;

#ifdef FILE_FLAG_CACHED
	if (fileAttrib & ATTRIB_INDEXFILE)
	{
		if (((VstrmFileIoFactory*)_pFileIoFactory)->getEnableCacheForIndex())
		{
			uFileAttrib |= FILE_FLAG_CACHED;

			MOLOG(Log::L_DEBUG, CLOGFMT(VstrmIo, "[%s] FILE_FLAG_CACHED flag enabled for index"),
				_strLogHint.c_str());
		}
	}
	else
	{
		uFileAttrib |= FILE_FLAG_CACHED;
	}
#endif

	if (fileAttrib & ATTRIB_INDEXFILE)
	{
		if (((VstrmFileIoFactory*)_pFileIoFactory)->getEnableRAID1ForIndex())
		{
			uFileAttrib |= SetRaidAttribute(RAID_1);

			MOLOG(Log::L_DEBUG, CLOGFMT(VstrmIo, "[%s] RAID_1 level flag enabled"),
				_strLogHint.c_str());
		}
	}

#ifdef FILE_FLAG_NPVR
	if (fileAttrib & ATTRIB_NPVR)
	{
		uFileAttrib |= FILE_FLAG_NPVR;
	}
#endif

	return uFileAttrib;
}

unsigned int VstrmFileIo::convertPosition(Position pos)
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

bool VstrmFileIo::openfile(const char* szFileName, AccessMode accessMode,ShareMode shareMode,CreationWay howToCreate, FileAttrib fileAttrib)
{
	int64 lStart = ZQ::common::now();
	_strFilename = szFileName;
	_strLogHint = _strFilename;
	
	VSTATUS vstat = _vstrmLoader.fpVsOpenEx( &_hOutputFile,
		(char*)_strFilename.c_str(), 
		convertAccessCode(accessMode),
		convertShareMode(shareMode),
		convertCreationWay(howToCreate),
		convertFileAttrib(fileAttrib),
		0,
		&_objectId);

	if(!IS_VSTRM_SUCCESS(vstat))
	{
		std::string errstr = getVstrmError(vstat);

		char msg[256];
		sprintf(msg, "VsOpenEx() failed with error: %s", errstr.c_str());
		int nErrorCode;
		if (vstat == VSTRM_DISK_FULL)
			nErrorCode = ERRCODE_VSTRM_DISK_FULL;
		else if (vstat == VSTRM_NETWORK_NOT_READY)
			nErrorCode = ERRCODE_VSTRM_NOT_READY;
		else
			nErrorCode = ERRCODE_VSTRM_API_ERROR;				

		setLastError(msg, nErrorCode);

		MOLOG(Log::L_ERROR, CLOGFMT(VstrmIo, "[%s] Failed to create output file: %s with error: %s"),
			_strLogHint.c_str(), _strFilename.c_str(), errstr.c_str());

		return false;
	}

	//backup the access mode for reservebandwidth
	_accessMode = accessMode;

	MOLOG(Log::L_INFO, CLOGFMT(VstrmIo, "open file [%s] on vstrm successful took %lldms"),
		_strFilename.c_str(), ZQ::common::now() - lStart);
	
	return true;
}

bool VstrmFileIo::setOption()
{
	// disable BufDrv throttling to have better Vstrm IO performance
	if(_bDisableBufDrvThrottle)
	{
		std::string errmsg;
		if(disableBufDrvThrottle(errmsg))
		{
			MOLOG(Log::L_DEBUG, CLOGFMT(VstrmIo, "[%s] BufDrv throttling disabed"), _strLogHint.c_str());
		}
		else
		{
			MOLOG(Log::L_WARNING, CLOGFMT(VstrmIo, "[%s] failed to disable BufDrv throttling with error: %s"), _strLogHint.c_str(), errmsg.c_str());
		}
	} // end of if(_disableBufDrvThrottle)
	
	return true;
}

bool VstrmFileIo::readfile(char* pBuf, unsigned int bufLen, unsigned int& rcvLen)
{
	int64 lStart = ZQ::common::now();
	DWORD amountRead;
	if(!_vstrmLoader.fpVsRead(_hOutputFile, pBuf, bufLen, &amountRead, NULL))
	{
		unsigned int vstat = _vstrmLoader.fpVstrmGetLastError();
		std::string errstr = getVstrmError(vstat);

		if (vstat == VSTRM_INVALID_DEVICE_REQUEST)
		{
			rcvLen = 0;
			return true;
		}
		int nErrorCode;
		if (vstat == VSTRM_DISK_FULL)
			nErrorCode = ERRCODE_VSTRM_DISK_FULL;
		else if (vstat == VSTRM_NETWORK_NOT_READY)
			nErrorCode = ERRCODE_VSTRM_NOT_READY;
		else if (vstat == VSTRM_NO_MEMORY)
			nErrorCode = ERRCODE_VSTRM_BUFFERQUEUE_FULL;
		else
			nErrorCode = ERRCODE_VSTRM_API_ERROR;				

		setLastError(std::string("Vstrm failed to read file ") + _strFilename + " with error:" + errstr, nErrorCode);
		MOLOG(Log::L_ERROR, CLOGFMT(VstrmIo, "[%s] VsRead() failed with error: %s)"), _strLogHint.c_str(), errstr.c_str());

		return false;
	}

	rcvLen = amountRead;

	if((ZQ::common::now() - lStart) > 0)
	MOLOG(Log::L_INFO, CLOGFMT(VstrmIo, "read file [%s] byte[%d] on vstrm  took %lldms"),
		_strFilename.c_str(), rcvLen, ZQ::common::now() - lStart);
	return true;
}

bool VstrmFileIo::writefile(char* pBuf, unsigned int bufLen, unsigned int& writeLen)
{
	int64 lStart = ZQ::common::now();
	int nBufLeft = (int)bufLen;
	
	do
	{
		int nBufWrite;
		if (nBufLeft>VSTRM_MAX_IO_SIZE)
		{
			nBufWrite = VSTRM_MAX_IO_SIZE;
		}
		else
		{
			nBufWrite = nBufLeft;
		}

		DWORD amountWritten;
		if(!_vstrmLoader.fpVsWrite(_hOutputFile, nBufWrite,  pBuf + bufLen - nBufLeft, &amountWritten, NULL))
		{
			unsigned int vstat = _vstrmLoader.fpVstrmGetLastError();
			std::string errstr = getVstrmError(vstat);

			int nErrorCode;
			if (vstat == VSTRM_DISK_FULL)
				nErrorCode = ERRCODE_VSTRM_DISK_FULL;
			else if (vstat == VSTRM_NETWORK_NOT_READY)
				nErrorCode = ERRCODE_VSTRM_NOT_READY;
			else if (vstat == VSTRM_NO_MEMORY)
				nErrorCode = ERRCODE_VSTRM_BUFFERQUEUE_FULL;
			else
				nErrorCode = ERRCODE_VSTRM_API_ERROR;				

			setLastError(std::string("Vstrm failed to write file ") + _strFilename + " with error:" + errstr, nErrorCode);
			MOLOG(Log::L_ERROR, CLOGFMT(VstrmIo, "[%s] VsWrite() failed with error: %s)"), _strLogHint.c_str(), errstr.c_str());

			return false;
		}
		
		nBufLeft -= nBufWrite;
	}while(nBufLeft>0);
	
	writeLen = bufLen;
   if((ZQ::common::now() - lStart) > 0)
	MOLOG(Log::L_INFO, CLOGFMT(VstrmIo, "write file [%s] byte[%d] on vstrm  took %lldms"),
		_strFilename.c_str(), writeLen, ZQ::common::now() - lStart);
	return true;
}

bool VstrmFileIo::seekfile(LONGLONG offset, Position pos)
{
	LARGE_INTEGER	tmp;
	tmp.QuadPart = offset;

	VSTATUS vStat = _vstrmLoader.fpVsSeek(_hOutputFile, _objectId, &tmp, convertPosition(pos));
	if (!IS_VSTRM_SUCCESS(vStat))
	{
		std::string errstr = getVstrmError(vStat);
		
		setLastError(std::string("Failed to VsSeek() on file ") + _strFilename + " with error:" + errstr, ERRCODE_VSTRM_API_ERROR);
		MOLOG(Log::L_ERROR, CLOGFMT(VsmFsIO, "[%s] VsSeek() to offset [%lld] failed with error: %s)"), _strLogHint.c_str(),
			offset, errstr.c_str());
		return false;
	}

	return true;
}

bool VstrmFileIo::setEndOfFile(LONGLONG offset)
{
	MOLOG(Log::L_DEBUG, CLOGFMT(VstrmIo, "[%s] VsSetEndOfFile() offset %lld"), _strFilename.c_str(), offset);

	LARGE_INTEGER	tmp;
	tmp.QuadPart = offset;		

	VSTATUS vstat = _vstrmLoader.fpVsSetEndOfFile(_hOutputFile, _objectId, &tmp);
	if (!IS_VSTRM_SUCCESS(vstat))
	{
		std::string errstr = getVstrmError(vstat);
		setLastError(std::string("Vstrm failed to write file ") + _strFilename + " with error:" + errstr, ERRCODE_VSTRM_API_ERROR);
		MOLOG(Log::L_ERROR, CLOGFMT(VstrmIo, "[%s] VsSetEndOfFile() offset [%d] failed with error: %s)"), _strLogHint.c_str(),
			tmp.LowPart, errstr.c_str());
		return false;
	}

	return true;
}

bool VstrmFileIo::closefile()
{
	if(_hOutputFile != INVALID_HANDLE_VALUE)
	{
		releaseBandwidth();

		MOLOG(Log::L_INFO, CLOGFMT(VstrmIo, "to VsClose() file [%s]"), _strFilename.c_str());
		_vstrmLoader.fpVsClose(_hOutputFile, _objectId);
		_hOutputFile = INVALID_HANDLE_VALUE;
		_objectId = 0;
		MOLOG(Log::L_INFO, CLOGFMT(VstrmIo, "VsClose() file [%s] successfully"), _strFilename.c_str());
	}

	return true;
}


//bandwidth management
bool VstrmFileIo::reserveBandwidth(unsigned int dwBandwidth)
{
	if(!_bwmgrClientId || !dwBandwidth)
		return true;

	_dwBandwidth = dwBandwidth;

	std::string strAccessMode;

	// reserve VStrm bandwidth
	VSTATUS	statusTicket = ERROR_SUCCESS;
	VSTRM_BANDWIDTH_RESERVE_BLOCK   rbFile = {0};
	PVSTRM_BANDWIDTH_RESERVE_BLOCK	pRbFile=&rbFile;

	// The Bw Mgr considers bandwidth requests
	// to be from the perspective of the PCI Bus, not the disks. So, to get data
	// onto the disks they must READ from the PCI Bus, so ask for READ BW here,
	// even tho we are putting data onto the disks using writes. 
	rbFile.ClientId         = _bwmgrClientId;

	// bandwidth type
	if (_accessMode & AccessMode::ACCESS_WRITE)
	{
		rbFile.Type				= kVSTRM_BANDWIDTH_TYPE_READ;
		strAccessMode = "write";
	}
	else
	{
		rbFile.Type				= kVSTRM_BANDWIDTH_TYPE_WRITE;
		strAccessMode = "read";
	}

	rbFile.Type				= kVSTRM_BANDWIDTH_TYPE_READ;
	rbFile.TargetType		= kVSTRM_BANDWIDTH_TARGETTYPE_FILE;

	rbFile.BwTarget         = (void*)(_strFilename.c_str()); 

	rbFile.MaxBandwidth		= dwBandwidth;	// passed in with request
	rbFile.MinBandwidth		= dwBandwidth;	// passed in with request
	rbFile.ReservedBandwidth = NULL;

	statusTicket = _vstrmLoader.fpVstrmClassReserveBandwidth(_hVstrm, pRbFile, &_bwTicket);

	if (statusTicket != VSTRM_SUCCESS)
	{
		_bwTicket = 0;

		char szBuf[255] = {0};
		_vstrmLoader.fpVstrmClassGetErrorText(_hVstrm, statusTicket, szBuf, sizeof(szBuf));

		MOLOG(Log::L_ERROR, CLOGFMT(VstrmIo, "[%s] VstrmClassReserveBandwidth(BW - %dbps, %s) failed with error %s"), 
			_strFilename.c_str(), dwBandwidth, strAccessMode.c_str(), szBuf);		

		setLastError(std::string("reserve vstrm bandwith failed with error: ") + szBuf, ERRCODE_VSTRM_BANDWIDTH_EXCEEDED);
		return false;
	}

	MOLOG(Log::L_INFO, CLOGFMT(VstrmIo, "[%s] VstrmClassReserveBandwidth(BW - %dbps, %s) return ticket 0x%I64X"),  
		_strFilename.c_str(), dwBandwidth, strAccessMode.c_str(), _bwTicket);

	return true;
}

void VstrmFileIo::releaseBandwidth()
{
	if(!_bwmgrClientId || !_bwTicket)
		return;

	VSTATUS	statusTicket = ERROR_SUCCESS;

	MOLOG(Log::L_INFO, CLOGFMT(VstrmIo, "[%s] VstrmClassReleaseBandwidth(BW - %dbps) for ticket: 0x%I64X"), 
		_strLogHint.c_str(), _dwBandwidth, _bwTicket);

	statusTicket = _vstrmLoader.fpVstrmClassReleaseBandwidth(_hVstrm, _bwTicket);

	//  it always return failure: VSTRM_NOT_SUPPORTED even the bandwidth was released indeed in some vstream version
	if (statusTicket != VSTRM_SUCCESS)
	{
		char szBuf[255] = {0};
		_vstrmLoader.fpVstrmClassGetErrorText(_hVstrm, statusTicket, szBuf, sizeof(szBuf));

		MOLOG(Log::L_INFO, CLOGFMT(VstrmIo, "[%s] VstrmClassReleaseBandwidth() for ticket 0x%I64X failed with error %s"), 
			_strLogHint.c_str(), _bwTicket, szBuf);
	}

	_bwTicket = 0;
	MOLOG(Log::L_INFO, CLOGFMT(VstrmIo, "[%s] VstrmClassReleaseBandwidth(BW - %dbps) succeed"),  _strLogHint.c_str(), _dwBandwidth);
}


void VstrmFileIo::getLastError(std::string& strErr, int& errCode)
{
	strErr = _strErr;
	errCode = _errCode;
}

std::string VstrmFileIo::getFileName()
{
	return _strFilename;
}

bool VstrmFileIo::disableBufDrvThrottle(std::string& errMsg)
{
	HANDLE vstrmHandle = _hVstrm;
	HANDLE fileHandle = _hOutputFile;
	OBJECT_ID objectId = _objectId; 

	VSTATUS vstat;
	ULONG disableThrottle = 1;
	ATTRIBUTE_ARRAY attributes;

	attributes.setAllorFailAll = 1;
	attributes.attributeCount = 1;
	attributes.objectId = objectId;

	attributes.attributeArray[0].setExactly = 1;
	attributes.attributeArray[0].attributeCode = VSTRM_ATTR_GEN_OVERRIDE_IO_THROTTLE;
	attributes.attributeArray[0].attributeQualifier = 0;
	attributes.attributeArray[0].attributeValueP = &disableThrottle;
	attributes.attributeArray[0].attributeValueLength = sizeof(disableThrottle);

	vstat = _vstrmLoader.fpVstrmClassSetSessionAttributesEx(fileHandle, objectId, &attributes);
	if (!IS_VSTRM_SUCCESS(vstat))
	{
		// check free space before returning an error - if there's 0 bytes available
		// then the set attributes request will return an INVALID_SESSION error, not a
		// disk space error even though it looks like SeaFile did the right thing

		// if Vstrm Class handle does not pass in, create it.
		HANDLE vstrmClass = INVALID_HANDLE_VALUE;
		vstrmClass = vstrmHandle;

		LARGE_INTEGER	free, total;

		VSTATUS vStatus = _vstrmLoader.fpVstrmClassGetStorageData(vstrmClass, &free, &total);
		if(IS_VSTRM_SUCCESS(vStatus))
		{
			if(0 == free.QuadPart)
			{
				errMsg = "VstrmClassSetSessionAttributesEx() set session attributes failed, no free space on disk";
			}
			else
			{
				errMsg = "VstrmClassSetSessionAttributesEx() set session attributes failed with error: " + getVstrmError(vStatus);
			}
		}

		return false;
	}

	return true;
}

std::string VstrmFileIo::getVstrmError()
{
	unsigned int status = _vstrmLoader.fpVstrmGetLastError();
	return getVstrmError(status);
}

std::string VstrmFileIo::getVstrmError(unsigned int status)
{
	return getVstrmError(_hVstrm, status);
}

std::string VstrmFileIo::getVstrmError(HANDLE hVstrmClass, unsigned int status)
{
	char sErrorText[512]={0};
	_vstrmLoader.fpVstrmClassGetErrorText(hVstrmClass, status, sErrorText, sizeof(sErrorText)-1);
	sErrorText[sizeof(sErrorText)-1] = '\0';

	char errcode[24];
	sprintf(errcode, "[0x%08x]", status);

	return std::string(sErrorText) + errcode;	
}

void VstrmFileIo::setLastError(const std::string& strErr, int errCode)
{
	_strErr = strErr;
	_errCode = errCode;
}


std::string VstrmFileIo::getFileExtension()
{
	return _strFileExtension;
}

void VstrmFileIo::setFileExtension(const char* szFileExt)
{
	_strFileExtension = szFileExt;
}



}}