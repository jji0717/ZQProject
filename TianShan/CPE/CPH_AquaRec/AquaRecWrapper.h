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

#ifndef ZQTS_CPE_AquaRecWRAPPER_H
#define ZQTS_CPE_AquaRecWRAPPER_H

#include <string>
#include "NativeThreadPool.h"

namespace ZQ
{
	namespace common{
		class Log;
		class NativeThreadPool;
	}
}


//class IMemAlloc;
class  CdmiClientBase;

namespace ZQTianShan 
{
namespace ContentProvision
{

class FileIoFactory;

class AquaRecVirtualSessI;

class AquaRecVirtualSessFac;

class AquaRecWrapper
{
public:
	AquaRecWrapper();
	~AquaRecWrapper();

	// the following parameters are must(required), without it the system could not work
	void setThreadPool(ZQ::common::NativeThreadPool* pPool);
	void setLog(ZQ::common::Log* pLog);
	void setConfigPath(const char* szPath);

	bool initialize();
	void uninitialize();

	AquaRecVirtualSessI* generateVirtualSession(const std::string& strChannelName, const std::string& contentId, const std::string& sessId, const std::string& startTimeUTC, const std::string& endTimeUTC, int nBandwidth);

protected:
	bool loadConfig();

	bool initAquaRecModule();
	void uninitAquaRecModule();
protected:
	ZQ::common::NativeThreadPool  cdmi_thpool;
	ZQ::common::NativeThreadPool*		_pool;
//	ZQ::common::Log*							_log;
	std::string									_cfgPath;

	AquaRecVirtualSessFac*						_pVirSessFactory;
	FileIoFactory*								_pFileIoFactory;

	CdmiClientBase*                             _pCdmiClient;
};






}
}

#endif
