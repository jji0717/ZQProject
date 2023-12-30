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

#include "NGODCSEnv.h"
#include "TianShanIceHelper.h"


namespace ZQTianShan {
	namespace ContentStore {

void NGODCSEnv::setThreadPool( ::ZQ::common::NativeThreadPool* pThreadPool )
{
	_threadPool = pThreadPool;
}

void NGODCSEnv::setConfig( ZQTianShan::NSS::NSSBaseConfig::NSSHolder* pBaseCfg )
{
	_pNSSBaseConfig = pBaseCfg;
}

void NGODCSEnv::setDataPath( const char* szDataPath )
{
	if (szDataPath)
		_strDataPath = szDataPath;
}

void NGODCSEnv::setLogPath( const char* szLogPath )
{
	if (szLogPath)
		_strLogPath = szLogPath;
}

void NGODCSEnv::setIceAdapter( ZQADAPTER_DECLTYPE& adapter )
{
	_pAdapter = &adapter;
}

bool NGODCSEnv::initEnv()
{
	if (!_threadPool || !_pAdapter)
	{
		//
		return false;
	}

	// init log
	std::string strLogFile = ZQTianShan::Util::fsConcatPath(_strLogPath , _strServiceName) + "_NGODCS.log";
	try
	{
		_NGODCSLogger.open(strLogFile.c_str(), ZQ::common::Log::L_DEBUG, 20, 1024*1024*20);
	}
	catch (ZQ::common::FileLogException& ex)
	{
		printf("failed to open log file %s with error %s\n", strLogFile.c_str(), ex.what());
		return false;
	}

	_NGODCSLogger(ZQ::common::Log::L_INFO, CLOGFMT(NGODCS, "==================================== initializing ContentStore ===================================="));

	std::string strEvtLogFile = ZQTianShan::Util::fsConcatPath( _strLogPath , _strServiceName) + "_NGODCS_events.log";
	try
	{
		_NGODCSEventLogger.open(strEvtLogFile.c_str(), ZQ::common::Log::L_DEBUG, 5, 1024*1024*20);
	}
	catch (ZQ::common::FileLogException& ex)
	{
		_NGODCSLogger(ZQ::common::Log::L_ERROR, CLOGFMT(NGODCS, "failed to open NGODCS event log file %s with error %s"), strEvtLogFile.c_str(), ex.what());
		return false;
	}

	std::string strDatabasePath;
	strDatabasePath = ZQTianShan::Util::fsConcatPath( _strDataPath , _strServiceName);
	_store = new ::ZQTianShan::ContentStore::ContentStoreImpl(_NGODCSLogger, _NGODCSEventLogger, *_threadPool, *_pAdapter, strDatabasePath.c_str());
	if(! _store->initializeContentStore())
	{
		_NGODCSLogger(ZQ::common::Log::L_ERROR, CLOGFMT(NGODCS, "failed to initialize ContentStore"));
		return false;
	}

	std::string volName;
	std::string volPath;
	try
	{
		for (size_t i = 0; i <_pNSSBaseConfig->_videoServer.vols.size(); i++)
		{
			volName = _pNSSBaseConfig->_videoServer.vols[i].mount;
			volPath = _pNSSBaseConfig->_videoServer.vols[i].targetName;
			if (volPath.empty())
				volPath = volName;

			volPath += FNSEPS;
			bool bDefaultVolume = (_pNSSBaseConfig->_videoServer.vols[i].defaultVal != 0);

			_store->mountStoreVolume(volName.c_str(), volPath.c_str(), bDefaultVolume);
		}
	}
	catch(...)
	{
		_NGODCSLogger(ZQ::common::Log::L_ERROR, CLOGFMT(NGODCS, "mount store volume[%s] path[%s] caught an exception"), volName.c_str(),volPath.c_str());
		return false;
	}

	return true;
}

void NGODCSEnv::uninitEnv()
{
	_store->unInitializeContentStore();
	_NGODCSLogger.flush();
	_NGODCSEventLogger.flush();
}

NGODCSEnv::NGODCSEnv()
{
	_threadPool = NULL;
	_pAdapter = NULL;
}

NGODCSEnv::~NGODCSEnv()
{

}

void NGODCSEnv::setServiceName( const char* szServiceName )
{
	_strServiceName = szServiceName;
}


}}