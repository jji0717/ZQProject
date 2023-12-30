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

#ifndef ZQTS_CPE_VSTRMFILEIOFACTORY_H
#define ZQTS_CPE_VSTRMFILEIOFACTORY_H


#include "FileIo.h"
#include "VstrmLoader.h"


namespace ZQTianShan 
{
	namespace ContentProvision
	{

class VstrmFileIoFactory: public FileIoFactory
{
public:
	VstrmFileIoFactory();
	virtual ~VstrmFileIoFactory();

	virtual ZQ::common::Log* getLog();
	virtual void setLog(ZQ::common::Log* pLog);

	virtual FileIo*	create();

	virtual bool deleteFile(const char* szFileName);

	virtual bool moveFile(const char* szFileName, const char* szNewFileName);

	virtual LONGLONG getFileSize(const char* szFileName);

	//find file functions
	// 	virtual HANDLE findFirstFile(const char* szFileName);
	// 	virtual HANDLE findNext(const char* szFileName);
	// 	virtual void findClose(HANDLE hFind);
	//find file functions
	virtual void findClose(HANDLE hfile);
	virtual bool findNextFile(HANDLE hfile,WIN32_FIND_DATAA& w);
	virtual HANDLE findFirstFile(char* name, WIN32_FIND_DATAA& w);
	virtual int getFileStats(char *filepath, FSUtils::fileInfo_t *infoptr);

	virtual bool initialize();
	virtual void uninitialize();

	virtual void getLastError(std::string& strErr, int& errCode);
public:

	HANDLE getVstrmClassHandle();
	
	unsigned int getBandwidthManageClientId();
	void setBandwidthManageClientId(unsigned int uClientId);

	bool getDisableBufDrvThrottle();	
	void setDisableBufDrvThrottle(bool bDisable);

	bool getEnableCacheForIndex();
	void setEnableCacheForIndex(bool bEnable);

	bool getEnableRAID1ForIndex();
	void setEnableRAID1ForIndex(bool bEnable);
	
	VstrmLoader& getVstrmLoader();
protected:
	std::string getVstrmError(unsigned int status);

	void setLastError(const std::string& strErr, int errCode);

	time_t FiletimeToTimet(FILETIME ft);
protected:
	HANDLE									_hVstrmClass;
	unsigned int							_uBandwidthClientId;
	bool									_bDisableBufDrvThrottle;
		bool									_bEnableCacheForIndex;
	bool									_bEnableRAID1ForIndex;
protected:
//	ZQ::common::Log*						_log;


	std::string								_strErr;
	int										_errCode;

	VstrmLoader								_vstrmLoader;
};


}
}

#endif