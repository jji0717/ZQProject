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
// Ident : $Id: NSSEnv.cpp $
// Branch: $Name:  $
// Author: Xiaoming Li
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ComcastNGOD/NSS/NSSEnv.cpp $
// 
// 2     1/02/14 6:04p Hui.shao
// removed NullLogger
// 
// 1     10-11-12 16:03 Admin
// Created.
// 
// 1     10-11-12 15:36 Admin
// Created.
// 
// 11    09-01-06 14:27 Xiaoming.li
// remove servant in de-constructor
// 
// 10    08-12-18 11:39 Xiaoming.li
// correct config file format
// 
// 9     08-12-05 17:58 Xiaoming.li
// 
// 8     08-12-05 17:09 Xiaoming.li
// 
// 7     08-11-27 14:32 Xiaoming.li
// 
// 6     08-11-14 11:48 Xiaoming.li
// add version info, change data folder setting and service name
// 
// 5     08-08-20 17:30 Xiaoming.li
// 
// 4     08-08-13 11:00 Xiaoming.li
// 
// 3     08-07-21 15:32 Xiaoming.li
// modify nonmutating to ["cp:const", "freeze:read"]
// 
// 2     08-07-14 14:54 Xiaoming.li
// 
// 1     08-06-13 11:23 Xiaoming.li
// 
// 1     08-04-18 16:25 xiaoming.li
// initial checkin
// ===========================================================================

#include "NSSEnv.h"

namespace ZQTianShan {
namespace NSS {

NSSEnv::NSSEnv(ZQ::common::FileLog& filelog, 
			   ZQ::common::NativeThreadPool& threadPool, 
			   Ice::CommunicatorPtr& communicator,
			   const char* iceStormEndpoint,
			   const char* endpoint /* = DEFAULT_ENDPOINT_NSS */, 
			   const char* databasePath /* = NULL */, 
			   const char* runtimeDBFolder /* = NULL */):
_contentStore(NULL),
_sessionGroupList(NULL),
_thpool(threadPool), 
_communicator(communicator),
_adapter(NULL),
_logFile(filelog),
_nssEventSinkI(filelog, iceStormEndpoint)
{
	_endpoint = (endpoint && strlen(endpoint)>0) ? endpoint : DEFAULT_ENDPOINT_NSS;

	_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(NSSEnv, "open adapter %s at %s"), ADAPTER_NAME_NSS, _endpoint.c_str());
	try
	{
		//initialize adapter
		_adapter = ZQADAPTER_CREATE(_communicator, ADAPTER_NAME_NSS, _endpoint.c_str(), glog);

		 //create eventsink thread
		 _nssEventSinkI.setAdapter(_adapter);
		 _nssEventSinkI.start();

		 _factory = new NSSFactory(*this);

		 _adapter->activate();

		 openDB(databasePath,runtimeDBFolder);
	}
	catch(Ice::Exception& ex)
	{
		_logFile(ZQ::common::Log::L_ERROR,CLOGFMT(NSSEnv,"Create adapter and open db failed with endpoint=%s and exception is %s"),endpoint,ex.ice_name().c_str());
		throw ex;
	}
	catch (::TianShanIce::ServerError& ex)
	{
		_logFile(ZQ::common::Log::L_ERROR,CLOGFMT(NSSEnv,"Create adapter failed and open db with endpoint=%s and exception is %s"),endpoint,ex.ice_name().c_str());
		throw ex;
	}
}

NSSEnv::~NSSEnv()
{
	::Ice::Identity ident = _adapter->getCommunicator()->stringToIdentity(SERVICE_NAME_ContentStore);
	_adapter->remove(ident);
	_contentStore = NULL;
	closeDB();
}

#define NssSessDataSubDir "NSS"
bool NSSEnv::openDB(const char* databasePath /* = NULL */,const char* dbRuntimePath/* =NULL */)
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
#define INSTANCE_INDEX(_IDX) _logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(NSSEnv, "create index: " #_IDX)); \
	_idx##_IDX = new ::TianShanIce::Streamer::NGODStreamServer::##_IDX(INDEXFILENAME(_IDX))

		_logFile(ZQ::common::Log::L_INFO, CLOGFMT(NSSEnv, "opening runtime database at path: %s"), _dbRuntimePath.c_str());

		//_idxSessionIdx = new ::TianShanIce::Streamer::NGODStreamServer::SessionIdx("SessionIdx");
		
		::CreateDirectory((_dbPath + NssSessDataSubDir FNSEPS).c_str(), NULL);
		::CreateDirectory((_dbRuntimePath + NssSessDataSubDir FNSEPS).c_str(), NULL);

		INSTANCE_INDEX(SessionIdx);
		{
			std::vector<Freeze::IndexPtr> indices;
			indices.push_back(_idxSessionIdx);
			
			_logFile(ZQ::common::Log::L_DEBUG, CLOGFMT(NSSEnv, "create evictor %s with index %s"), DBFILENAME_NssSession, "SessionIdx");

#if ICE_INT_VERSION / 100 >= 303
			_eNssStream = ::Freeze::createBackgroundSaveEvictor(_adapter, _dbRuntimePath + NssSessDataSubDir FNSEPS, DBFILENAME_NssSession, 0, indices);
#else
			//_eSession = Freeze::createEvictor(_adapter, _dbPath + "Contents" +FNSEPS, DBFILENAME_Content ".dat", 0, indices);
			_eNssStream = Freeze::createEvictor(_adapter, _dbRuntimePath + NssSessDataSubDir FNSEPS, DBFILENAME_NssSession, 0, indices);
#endif
			//_eNssStream = Freeze::createEvictor(_adapter, _dbRuntimePath + NssSessDataSubDir FNSEPS, DBFILENAME_NssSession, 0, indices);

			_eNssStream->setSize(100);
			_adapter->addServantLocator(_eNssStream, DBFILENAME_NssSession);	
		}

		return true;
	}
	catch(const Ice::Exception& ex)
	{
		//printf("%d\n", ex.ice_name().c_str());
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (_logFile,"NSSEnv",1001,CLOGFMT(NSSEnv, "openDB() caught exception: %s"), ex.ice_name().c_str());
	}
	catch(...)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (_logFile,"NSSEnv",1002, CLOGFMT(NSSEnv, "openDB() caught unkown exception"));
	}

	return true;
}

void NSSEnv::closeDB(void)
{
	_eNssStream = NULL;
	_idxSessionIdx = NULL;
}

}//namespace NSS

}//namespace ZQTianShan