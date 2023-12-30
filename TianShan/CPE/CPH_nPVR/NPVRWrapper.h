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

#ifndef ZQTS_CPE_NPVRWRAPPER_H
#define ZQTS_CPE_NPVRWRAPPER_H


#include <string>
#include "BufferPool.h"


namespace ZQ
{
	namespace common{
		class Log;
		class NativeThreadPool;
	}
}


//class IMemAlloc;

namespace ZQTianShan 
{
namespace ContentProvision
{

class FileIoFactory;

class VirtualSessI;

class VirtualSessFac;

/*
class NPVRSession
{
public:
	enum SessionState
	{
		StateInit,
		StateProcessing,
		StateSuccess,
		StateFailure
	};

	virtual void getProgress(LONGLONG& procv, LONGLONG& total);

	virtual SessionState getState();

	bool initSession();
	bool startSession();
	void stopSession();
protected:
	void setState(SessionState state);

};*/

class NPVRWrapper
{
public:
	NPVRWrapper();
	~NPVRWrapper();

	// the following parameters are must(required), without it the system could not work
	void setThreadPool(ZQ::common::NativeThreadPool* pPool);
	void setLog(ZQ::common::Log* pLog);
	void setConfigPath(const char* szPath);
	void setMemoryAllocator(ZQ::common::BufferPool* pAlloc);

	bool initialize();
	void uninitialize();

	bool isValidSession(const std::string& strFilename);

	VirtualSessI* generateVirtualSession(int nBandwidth, const std::string& strFilename, const std::string& Ip, int port, bool bH264Type);

protected:
	bool loadConfig();

	bool initFileIoFactory();
	void uninitFileIoFactory();

	bool initGraphModule(FileIoFactory* pFileIoFactory);
	void uninitGraphModule();

	bool initNPVRModule();
	void uninitNPVRModule();

protected:

	bool										_bGraphModuleInited;

	ZQ::common::NativeThreadPool*				_pool;
//	ZQ::common::Log*							_log;
	ZQ::common::BufferPool*						_pAlloc;
	std::string									_cfgPath;

	VirtualSessFac*								_pVirSessFactory;
	FileIoFactory*								_pFileIoFactory;
};






}
}

#endif