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

#ifndef ZQTS_CPE_NPVR_EXTEND_FILE_H
#define ZQTS_CPE_NPVR_EXTEND_FILE_H

#include <string>
#include <vector>

namespace ZQ
{
	namespace common{
		class Log;
	}
}

namespace ZQTianShan 
{
namespace ContentProvision
{


class NPVRExtendFileI
{
public:

	typedef std::vector<std::string> SubFileList;
	typedef std::vector<std::string> SubscriberList;

	virtual ~NPVRExtendFileI(){};

	/// set log handler
	virtual void setLog(ZQ::common::Log* pLog) = 0;

	/// set the file name on the storage
	virtual void setFileName(const std::string& strFileName) = 0;
	virtual std::string getFileName() = 0;
	
	/// load from storage and parse the data, after load successfully, we could get the datas
	virtual bool load() = 0;
	
	/// save the current datas to storage
	virtual bool save() = 0;

	/// not path on the subfile, only the file name
	virtual void addSubFile(const std::string& strSubFile) = 0;	
	virtual void setSubFiles(const SubFileList& strSubFiles) = 0;
	virtual SubFileList getSubFiles() = 0;
	virtual void clearSubFiles() = 0;

	virtual void addSubscriber(const std::string& strSubscriber) = 0;
	virtual void setSubscribers(const SubscriberList& strSubscriber) = 0;
	virtual SubscriberList getSubscribers() = 0;
	virtual void clearSubscribers() = 0;

	virtual std::string getLastError() = 0;

protected:
	virtual void setLastError(const std::string& strErr) = 0;

};


}
}

#endif