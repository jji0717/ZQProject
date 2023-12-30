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
#include "VirtualSess.h"
#include "SubjectWriteI.h"
#include "LeadSessColI.h"
#include "ErrorCode.h"


#define MOLOG			(glog)
#define VirSess			"VirSess"


namespace ZQTianShan 
{
namespace ContentProvision
{
	

VirtualSess::VirtualSess(FileIoFactory* pFileIoFactory)
	:_pFileIoFac(pFileIoFactory)
{
	_nFileNum = 3;  //default for mpeg type
//	_log = &ZQ::common::NullLogger;
	_bTestMode = false;
	_bUninitialized = false;
}

VirtualSess::~VirtualSess()
{
	uninitialize();
	processOutput();
}

bool VirtualSess::initialize()
{
	_strLogHint = _strFilename;

	if (_strFilename == _sessionGroup.getPathName())
	{
		setLastError(std::string("Invalid NPVR session name: ") + _strFilename, 0);			
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(VirSess, "Invalid NPVR session filename [%s]"), _strFilename.c_str());

		return false;
	}

	MOLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(VirSess, "[%s] Entering initialize()"), _strLogHint.c_str());

	//tell the lead session collection to make a reservation for this session
	if (!makeReservation())
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(VirSess, "[%s] failed to makeReservation() on lead session"), _strLogHint.c_str());
		return false;
	}

	std::vector<std::string>::iterator iter;
	int index = 0;
	//for main file
	{
		_subfile[index].fileName = _strFilename;
		_subfile[index].suffix = "";
		_subfile[index].pFileIo.reset(_pFileIoFac->create());
		_subfile[index].pFileIo->setFileExtension(_subfile[index].suffix.c_str());
		index++;
	}

	//for trick files except index file
	for (iter = _suffixVec.begin(); iter != _suffixVec.end(); iter++)
	{
		if (stricmp((*iter).c_str(),".vvx") && stricmp((*iter).c_str(),".vv2"))
		{
			_subfile[index].fileName = _strFilename + (*iter);
			_subfile[index].suffix = *iter;
			_subfile[index].pFileIo.reset(_pFileIoFac->create());
			_subfile[index].pFileIo->setFileExtension(_subfile[index].suffix.c_str());
			index++;
		}
	}
	_nFileNum = index;

	for (int i = 0; i < index ; i++)
	{
		if (!_subfile[i].pFileIo->openfile(_subfile[i].fileName.c_str(), 
			FileIo::ACCESS_WRITE,
			(FileIo::ShareMode)(FileIo::SHARE_READ | FileIo::SHARE_WRITE),
			FileIo::WAY_CREATE_ALWAYS,
			(FileIo::FileAttrib)(FileIo::ATTRIB_NPVR)))
		{
			std::string strErr;
			int nErrorCode;
			_subfile[i].pFileIo->getLastError(strErr, nErrorCode);
			
			setLastError(strErr, nErrorCode);			
			
			return false;
		}

		MOLOG(ZQ::common::Log::L_INFO,CLOGFMT(VirSess, "[%s] Open file %s Successfully."),
			_strLogHint.c_str(),_subfile[i].fileName.c_str());

		_subfile[i].pFileIo->setOption();		///< this must be called, the fileio level would set the private options on the file
	}


	MOLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(VirSess, "[%s] initialize() successful"),_strLogHint.c_str());
	return true;
}

bool VirtualSess::makeReservation()
{
	MOLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(VirSess, "[%s] makeReservation()"), _strLogHint.c_str());

	if (!LeadSessColI::instance()->reservation(this))
	{
		return false;
	}

	return true;
}

bool VirtualSess::execute()
{
	MOLOG(ZQ::common::Log::L_DEBUG,CLOGFMT(VirSess, "[%s] registerObserver()"), _strLogHint.c_str());

	if (!LeadSessColI::instance()->registerObserver(this))
	{
		return false;
	}

	setState(StateProcessing);

	MOLOG(ZQ::common::Log::L_INFO,CLOGFMT(VirSess, "[%s] execute started"), _strLogHint.c_str());
	return true;
}	



void VirtualSess::uninitialize()
{
	if (_bUninitialized)
		return;

	LeadSessColI::instance()->removeObserver(this);

	for (int i = 0; i < _nFileNum; i++)
	{
		if (_subfile[i].pFileIo.get())
		{
			_subfile[i].pFileIo->closefile();
			_subfile[i].pFileIo.reset(0);
		}
	}

	if (getState()==StateProcessing)
	{
		if (!_llProcv)
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(VirSess, "[%s] the main file size 0, no data saved, set failure"), _strLogHint.c_str());

			setLastError("the main file size 0, no data save", ERRCODE_READ_SOURCE);
		}
		else
		{
			setState(StateSuccess);
			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(VirSess, "[%s] set provision state success"), _strLogHint.c_str());
		}		
	}

	_bUninitialized = true;
}

bool VirtualSess::notifyWrite(const std::string& file, void* pBuf, int nLen)
{	
	FileIo* pFileIo = NULL;
	if (file.empty())
	{
		pFileIo = _subfile[0].pFileIo.get();
		_llProcv += nLen;

		if (_bEnableMD5)
		{
			//main file
			_md5ChecksumUtil.checksum((const char*)pBuf, nLen);
		}
	}
	else 
	{
		for (int i=1; i < _nFileNum;i++)
		{
			if (!stricmp(_subfile[i].suffix.c_str(),file.c_str()))
			{
				pFileIo = _subfile[i].pFileIo.get();
				_subfile[i].processBytes += nLen;
				break;
			}
		}
	}

	if (!pFileIo)
		return false;

	if (_bTestMode)
	{
		unsigned int writelen;
		if (!pFileIo->writefile((char*)pBuf,nLen,writelen))
		{
			std::string strErr;
			int nErrCode;
			pFileIo->getLastError(strErr, nErrCode);
			setLastError(strErr, nErrCode);
			return false;
		}
	}

	return true;
}	

void VirtualSess::notifyDestroy(const std::string& strErr, int nErrorCode)
{
	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(VirSess, "[%s] notifyDestroy() with error[%s], code[%d]"),
		_strLogHint.c_str(), strErr.c_str(), nErrorCode);

	setLastError(strErr, nErrorCode);
	LeadSessColI::instance()->removeObserver(this);	
}	

void VirtualSess::setFileSuffix(std::vector<std::string>& suffix)
{
	_suffixVec = suffix;
}

void VirtualSess::setLog(ZQ::common::Log* pLog)
{
	_log = pLog;
}

bool VirtualSess::getMediaInfo(MediaInfo& mInfo)
{
	return LeadSessColI::instance()->getMediaInfo(this, mInfo);
}

LONGLONG VirtualSess::getSupportFileSize()
{
	LONGLONG llSupportFileSize = 0;
	for (int i=1; i < _nFileNum;i++)
	{
		llSupportFileSize += _subfile[i].processBytes;
	}

	return llSupportFileSize;
}

// get md5 check sum for the main file
std::string VirtualSess::getMD5Sum()
{
	if (!_bEnableMD5)
		return "";

	return _md5ChecksumUtil.lastChecksum();
}

std::string VirtualSess::getIndexPathName()
{
	_strLeadCopyPathName =  LeadSessColI::instance()->getLeadSessPathName(this);

	//append the index to it
	std::string strExt;
	if (_ctType == MPEG2)
		strExt = ".vvx";
	else
		strExt = ".vv2";
	
	return _strLeadCopyPathName + strExt;
}

void VirtualSess::processOutput()
{
	if (getState()!=StateFailure)
		return;

	for (int i = 0; i < _nFileNum; i++)
	{
		if (!_subfile[i].fileName.empty())
		{
			if (!_pFileIoFac->deleteFile(_subfile[i].fileName.c_str()))
			{
				std::string errstr;
				int errcode;
				_pFileIoFac->getLastError(errstr,errcode);
				MOLOG(ZQ::common::Log::L_ERROR,CLOGFMT(VirSess, "[%s] delete file %s failed with error:%s."),
					_strLogHint.c_str(),_subfile[i].fileName.c_str(),errstr.c_str());
			}
			_subfile[i].fileName = "";
		}
	}
}

}
}

