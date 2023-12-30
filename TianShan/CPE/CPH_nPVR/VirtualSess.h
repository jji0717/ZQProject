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

#ifndef ZQTS_CPE_VIRTUALSESS_H
#define ZQTS_CPE_VIRTUALSESS_H


#include "VirtualSessI.h"
#include "FileIo.h"
#include "MD5CheckSumUtil.h"



namespace ZQTianShan 
{
namespace ContentProvision
{

class FileIoFactory;

class VirtualSess : public VirtualSessI
{
public:	
	VirtualSess(FileIoFactory* pFileIoFactory);
	virtual ~VirtualSess();
	
	virtual void setFileSuffix(std::vector<std::string>& suffix);

	virtual bool initialize();
	virtual bool execute();
	virtual void uninitialize();

	virtual void setLog(ZQ::common::Log* pLog);

	//get the index file path & file name
	virtual std::string getIndexPathName();

	virtual bool getMediaInfo(MediaInfo& mInfo);

	virtual LONGLONG getSupportFileSize();

	// get md5 check sum for the main file
	virtual std::string getMD5Sum();
	virtual void setTestMode(bool btest = false){_bTestMode = btest;}
protected:

	virtual bool makeReservation();
	
	virtual bool notifyWrite(const std::string& file, void* pBuf, int nLen);
	virtual void notifyDestroy(const std::string& strErr, int nErrorCode);
	
	void processOutput();

	struct subFile
	{
		std::string					fileName;
		std::string					suffix;
		std::auto_ptr<FileIo>		pFileIo;
		LONGLONG					processBytes;

		subFile()
		{
			processBytes = 0;
		}
	};

	subFile				_subfile[10];
	FileIoFactory*		_pFileIoFac;
	int                 _nFileNum;
    bool                _bTestMode;
	::ZQ::common::MD5ChecksumUtil _md5ChecksumUtil;

	std::string			_strLogHint;

	std::string			_strLeadCopyPathName;	///<the lead copy index path name

	bool				_bUninitialized;

//	ZQ::common::Log*	_log;
};



}
}

#endif