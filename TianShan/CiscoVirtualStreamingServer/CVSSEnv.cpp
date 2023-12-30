// ===========================================================================
// Copyright (c) 2004 by
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
// Ident : $Id: CVSSEnv.cpp $
// Branch: $Name:  $
// Author: Xiaoming Li
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/CiscoVirtualStreamingServer/CVSSEnv.cpp $
// 
// 2     1/02/14 6:06p Hui.shao
// removed NullLogger
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:36 Admin
// Created.
// 
// 2     09-01-20 9:50 Xiaoming.li
// modify for cisco
// 
// 1     08-12-15 9:07 Xiaoming.li
// initial checkin
// ===========================================================================

#include "CVSSEnv.h"

namespace ZQTianShan {
namespace CVSS {

CVSSEnv::CVSSEnv(ZQ::common::FileLog& filelog, 
			   ZQ::common::NativeThreadPool& threadPool, 
			   Ice::CommunicatorPtr& communicator,
			   const char* iceStormEndpoint,
			   const char* endpoint /* = DEFAULT_ENDPOINT_CVSS */, 
			   const char* databasePath /* = NULL */, 
			   const char* runtimeDBFolder /* = NULL */):
_contentStore(NULL),
_thpool(threadPool), 
_communicator(communicator),
_adapter(NULL),
_logFile(filelog),
_cvssEventSinkI(filelog, iceStormEndpoint)
{
	_endpoint = (endpoint && strlen(endpoint)>0) ? endpoint : DEFAULT_ENDPOINT_CVSS;

	_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(CVSSEnv, "open adapter %s at %s"), ADAPTER_NAME_CVSS, _endpoint.c_str());
	try
	{
		//initialize adapter
		_adapter = ZQADAPTER_CREATE(_communicator, ADAPTER_NAME_NSS, _endpoint.c_str(), glog);

		 //create eventsink thread
		 _cvssEventSinkI.setAdapter(_adapter);
		 _cvssEventSinkI.start();		 
	}
	catch(Ice::Exception& ex)
	{
		_logFile(ZQ::common::Log::L_ERROR,CLOGFMT(CVSSEnv,"Create adapter failed with endpoint=%s and exception is %s"),endpoint,ex.ice_name().c_str());
		throw ex;
	}

	_factory = new CVSSFactory(*this);
	
	openDB(databasePath,runtimeDBFolder);

	 _adapter->activate();
}

CVSSEnv::~CVSSEnv()
{
	_contentStore = NULL;
	closeDB();
}

#define CVSSDataSubDir "CVSS"
bool CVSSEnv::openDB(const char* databasePath /* = NULL */,const char* dbRuntimePath/* =NULL */)
{
	closeDB();

	if (NULL == databasePath || strlen(databasePath) <1)
		_dbPath = _programRootPath + "data" FNSEPS;
	else 
		_dbPath = databasePath;

	if (FNSEPC != _dbPath[_dbPath.length()-1])
		_dbPath += FNSEPS;

	if ( NULL == dbRuntimePath || strlen(dbRuntimePath)<1 ) 
	{
		_dbRuntimePath = _dbPath;
	}
	else
	{
		_dbRuntimePath = dbRuntimePath;
	}
	if (FNSEPC != _dbRuntimePath[_dbRuntimePath.length()-1])
		_dbRuntimePath += FNSEPS;
	try 
	{	
		
		// open the Indexes
#define INSTANCE_INDEX(_IDX) _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(CVSSEnv, "create index: " #_IDX)); \
	_idx##_IDX = new ::TianShanIce::Streamer::CiscoVirtualStreamServer::##_IDX(INDEXFILENAME(_IDX))

		_logFile(ZQ::common::Log::L_INFO, CLOGFMT(CVSSEnv, "opening runtime database at path: %s"), _dbRuntimePath.c_str());

		//_idxSessionIdx = new ::TianShanIce::Streamer::NGODStreamServer::SessionIdx("SessionIdx");
		
		::CreateDirectory((_dbPath + CVSSDataSubDir FNSEPS).c_str(), NULL);
		::CreateDirectory((_dbRuntimePath + CVSSDataSubDir FNSEPS).c_str(), NULL);

		INSTANCE_INDEX(SessionIdx);
		{
			std::vector<Freeze::IndexPtr> indices;
			indices.push_back(_idxSessionIdx);
			
			_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(CVSSEnv, "create evictor %s with index %s"), DBFILENAME_CVSSSession, "SessionIdx");

#if ICE_INT_VERSION / 100 >= 303
			_eCvssStream = ::Freeze::createBackgroundSaveEvictor(_adapter, _dbRuntimePath + CVSSDataSubDir FNSEPS, DBFILENAME_CVSSSession, 0, indices);
#else
			//_eSession = Freeze::createEvictor(_adapter, _dbPath + "Contents" +FNSEPS, DBFILENAME_Content ".dat", 0, indices);
			_eCvssStream = Freeze::createEvictor(_adapter, _dbRuntimePath + CVSSDataSubDir FNSEPS, DBFILENAME_CVSSSession, 0, indices);
#endif
			//_eNssStream = Freeze::createEvictor(_adapter, _dbRuntimePath + NssSessDataSubDir FNSEPS, DBFILENAME_NssSession, 0, indices);

			_eCvssStream->setSize(100);
			_adapter->addServantLocator(_eCvssStream, DBFILENAME_CVSSSession);	
		}

		return true;
	}
	catch(const Ice::Exception& ex)
	{
		//printf("%d\n", ex.ice_name().c_str());
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (_logFile,"CVSSEnv",1001,CLOGFMT(CVSSEnv, "openDB() caught exception: %s"), ex.ice_name().c_str());
	}
	catch(...)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (_logFile,"CVSSEnv",1002, CLOGFMT(CVSSEnv, "openDB() caught unkown exception"));
	}

	return true;
}

void CVSSEnv::closeDB(void)
{
	_eCvssStream = NULL;
	_idxSessionIdx = NULL;
}

}//namespace CVSS

}//namespace ZQTianShan