// ===========================================================================
// Copyright (c) 2011 by
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
// 
// ===========================================================================

#ifndef __ZQTianShanNGODEnv_H__
#define __ZQTianShanNGODEnv_H__


#include "FileLog.h"
#include "ConfigHelper.h"
#include "NSSConfig.h"
#include "NSSCfgLoader.h"
#include "ContentImpl.h"


namespace ZQTianShan {
	namespace ContentStore {

class NGODCSEnv
{
public:
	NGODCSEnv();
	~NGODCSEnv();

	void setServiceName(const char* szServiceName);
	void setLogPath(const char* szLogPath);
	void setDataPath(const char* szDataPath);
	void setConfig(ZQTianShan::NSS::NSSBaseConfig::NSSHolder* pBaseCfg);
	void setThreadPool(::ZQ::common::NativeThreadPool*	pThreadPool);
	void setIceAdapter(ZQADAPTER_DECLTYPE& adapter);
	bool initEnv();
	void uninitEnv();
public:

	::ZQTianShan::ContentStore::ContentStoreImpl::Ptr	_store;
	ZQ::common::FileLog		_NGODCSLogger;
	ZQ::common::FileLog		_NGODCSEventLogger;
	::ZQ::common::NativeThreadPool*		_threadPool;
	std::string			_strServiceName;
	std::string			_strLogPath;
	std::string			_strDataPath;
	ZQADAPTER_DECLTYPE*		_pAdapter;
	ZQTianShan::NSS::NSSBaseConfig::NSSHolder *	_pNSSBaseConfig;
};

}}

#endif
