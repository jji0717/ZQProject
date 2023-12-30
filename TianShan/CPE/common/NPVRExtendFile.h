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

#ifndef ZQTS_CPE_NPVR_EXTENDSION_H
#define ZQTS_CPE_NPVR_EXTENDSION_H

#include "NPVRExtendFileI.h"


namespace ZQTianShan 
{
namespace ContentProvision
{

class FileIoFactory;
class FileIo;

class NPVRExtendFile:public NPVRExtendFileI
{
public:
	NPVRExtendFile(){};
	NPVRExtendFile(FileIoFactory* pFileIoFactory);

	virtual ~NPVRExtendFile();

	/// set log handler
	virtual void setLog(ZQ::common::Log* pLog);

	/// set the file name on the storage
	virtual void setFileName(const std::string& strFileName);
	virtual std::string getFileName();
	
	/// load from storage and parse the data, after load successfully, we could get the datas
	virtual bool load();
	
	/// save the current datas to storage
	virtual bool save();

	/// not path on the subfile, only the file name
	/// need to check if the subfile is already in it or not, if already in, skip
	virtual void addSubFile(const std::string& strSubFile);	
	virtual void setSubFiles(const SubFileList& strSubFiles);
	virtual SubFileList getSubFiles();
	virtual void clearSubFiles();

	/// need to check if the subscriber is already in it or not, if already in, skip
	virtual void addSubscriber(const std::string& strSubscriber);
	virtual void setSubscribers(const SubscriberList& strSubscriber);
	virtual SubscriberList getSubscribers();
	virtual void clearSubscribers();

	virtual std::string getLastError();

protected:
	virtual void setLastError(const std::string& strErr);
		
		
protected:
	
	
	std::string							_strError;
	std::string							_strFilename;
	SubFileList							_subFileList;
	SubscriberList						_subscriberList;
	std::vector<int>                    _leadCopy;

//	ZQ::common::Log*					_log;

	FileIoFactory*						_pFileIoFactory;
};


}
}

#endif