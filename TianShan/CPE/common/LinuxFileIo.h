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

#ifndef ZQTS_CPE_LINUXFILEIO_H
#define ZQTS_CPE_LINUXFILEIO_H

#include "ZQ_common_conf.h"
#include "FileIo.h"

#ifdef ZQ_OS_LINUX
extern "C"
{
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
}

namespace ZQTianShan 
{
	namespace ContentProvision
	{

class LinuxIoFactory;

class LinuxFileIo : public FileIo
{
public:
	friend class LinuxIoFactory;

	LinuxFileIo(LinuxIoFactory* pFileIoFactory);
	virtual ~LinuxFileIo();

	virtual ZQ::common::Log* getLog();
	virtual void setLog(ZQ::common::Log* pLog);

	virtual bool openfile(const char* szFileName, AccessMode accessMode,ShareMode shareMode,CreationWay howToCreate, FileAttrib fileAttrib);
	virtual bool setOption();
	virtual bool readfile(char* pBuf, unsigned int bufLen, unsigned int& rcvLen);
	virtual bool writefile(char* pBuf, unsigned int bufLen, unsigned int& writeLen);
	virtual bool seekfile(int64 offset, Position pos);
	virtual bool setEndOfFile(int64 offset);
	virtual bool closefile();

	virtual std::string getFileName();

	virtual std::string getFileExtension();

	virtual void setFileExtension(const char* szFileExt);
    virtual void setMainFileFalg(){};
	//bandwidth management
	virtual bool reserveBandwidth(unsigned int dwBandwidth);
	virtual void releaseBandwidth();

	virtual void getLastError(std::string& strErr, int& errCode);

protected:
	unsigned int convertAccessCode(AccessMode accessMode);
	unsigned int convertShareMode(ShareMode shareMode);
	unsigned int convertCreationWay(CreationWay howToCreate);
	unsigned int convertFileAttrib(FileAttrib fileAttrib);
	unsigned int convertPosition(Position pos);


protected:
	void setLastError(const std::string& strErr, int errCode);

protected:

	HANDLE								_hOutputFile;

	ZQ::common::Log*					_log;

	std::string							_strFileName;
	std::string							_strFileExtension;

	std::string							_strErr;
	int									_errCode;

	std::string							_strLogHint;

	std::string							_strPath;

	
};


}
}

#endif
#endif

