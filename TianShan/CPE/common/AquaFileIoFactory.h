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

#ifndef ZQTS_CPE_AQUAFILEIOFACTORY_H
#define ZQTS_CPE_AQUAFILEIOFACTORY_H

#include "FileIo.h"
#include <json/json.h>

class CdmiFuseOps;

namespace ZQTianShan 
{
	namespace ContentProvision
	{

class ZQ_COMMON_API AquaFileIoFactory: public FileIoFactory
{
public:
	friend class AquaFileIo;

	AquaFileIoFactory();
	virtual ~AquaFileIoFactory();

	virtual ZQ::common::Log* getLog();
	virtual void setLog(ZQ::common::Log* pLog);

	virtual FileIo*	create();

	virtual bool deleteFile(const char* szFileName);

	virtual bool moveFile(const char* szFileName, const char* szNewFileName);

	virtual int64 getFileSize(const char* szFileName);

#ifdef ZQ_OS_MSWIN
	virtual void findClose(HANDLE hfile);
	virtual bool findNextFile(HANDLE hfile,WIN32_FIND_DATAA& w);
	virtual HANDLE findFirstFile(char* name, WIN32_FIND_DATAA& w);
#endif

	virtual bool initialize();
	virtual void uninitialize();
	virtual void getLastError(std::string& strErr, int& errCode);
public:
	void setCdmiOps(CdmiFuseOps* pCdmiOps);

	CdmiFuseOps* getCdmiOps();

	virtual int getFileStats(char *filepath, FSUtils::fileInfo_t *infoptr);

	void setLastError(const std::string& strErr, int errCode);

	static std::string getErrorText(unsigned int errCode);
	static std::string getLastError();

	static std::string getPathOfFile(const std::string& strFilename);

private:
    bool dataObjMetadataToFstat(const Json::Value& metadata, FSUtils::fileInfo_t *infoptr);
protected:
	ZQ::common::Log*						_log;

	std::string								_strErr;
	int										_errCode;

	CdmiFuseOps*							_pCdmiOps;
};

}
}

#endif

