// ===========================================================================
// Copyright (c) 2006 by
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
// Ident : $Id: WeiwooImpl.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/Weiwoo/WeiwooSvcEnv.cpp $
// 
// 23    1/12/16 8:53a Dejian.fei
// 
// 22    11/10/15 4:49p Hui.shao
// 
// 22    11/10/15 4:43p Hui.shao
// service state
// 
// 21    3/18/15 7:26p Build
// removed the snmp subagent of .5
// 
// 20    3/19/14 2:06p Zonghuan.xiao
// rollback
// 
// 18    9/04/13 4:12p Zonghuan.xiao
// refactor snmp code in weiwoo
// 
// 17    4/11/13 5:08p Hui.shao
// 
// 16    2/06/13 11:29a Hui.shao
// 
// 15    2/06/13 11:08a Hui.shao
// merged from V1.16
// 
// 12    2/06/13 10:15a Hui.shao
// enlarged DB_CONFIG
// 
// 11    12/21/12 11:13a Hui.shao
// log params
// 
// 10    9/20/12 6:03p Hui.shao
// merged from maintree about snmp export
// 
// 9     12/30/11 2:23p Hui.shao
// 
// 8     12/30/11 2:05p Hui.shao
// 
// 7     3/10/11 3:50p Hongquan.zhang
// 
// 6     3/10/11 3:49p Hongquan.zhang
// 
// 5     3/10/11 3:48p Hongquan.zhang
// 
// 4     3/10/11 11:51a Hongquan.zhang
// 
// 3     3/09/11 4:42p Hongquan.zhang
// 
// 2     3/07/11 5:00p Fei.huang
// + migrate to linux
// 
// 1     10-11-12 16:08 Admin
// Created.
// 
// 1     10-11-12 15:42 Admin
// Created.
// 
// 44    10-06-18 18:13 Hui.shao
// take a createDBFolder() instead
// 
// 43    10-06-18 17:04 Build
// 
// 42    10-06-18 16:19 Hui.shao
// removed index IdToSess
// 
// 41    10-06-18 15:53 Hui.shao
// always turn DB_RECOVERFATAL on, hardcoded some DB parameters to always
// be true
// 
// 40    09-07-28 12:04 Xiaoming.li
// 
// 39    09-06-05 13:42 Hongquan.zhang
// 
// 38    09-03-31 17:19 Hongquan.zhang
// get business router when service start up
// 
// 37    08-08-14 15:06 Hui.shao
// merged from 1.7.10
// 
// 37    08-07-21 14:28 Hui.shao
// removed AMI callback as members, should be new-ed whenever the
// invocaion occurs
// 
// 36    08-07-08 13:39 Hui.shao
// fixed for ICE 3.3 new syntax
// 
// 35    08-04-23 11:11 Hongquan.zhang
// migrate configuration to ConfigHelper
// Add Ice Performance Tunning configuration when OpenDB
// 
// 34    07-12-21 15:13 Hongquan.zhang
// 
// 33    07-12-14 18:03 Hongquan.zhang
// Use ZQTianShan::Adapter
// 
// 32    07-12-14 11:39 Hongquan.zhang
// Update Error Code
// 
// 31    07-10-25 14:07 Hongquan.zhang
// 
// 30    07-10-17 12:29 Hongquan.zhang
// 
// 29    07-09-18 12:56 Hongquan.zhang
// 
// 28    07-08-30 15:46 Hongquan.zhang
// 
// 27    07-06-26 13:29 Hongquan.zhang
// 
// 26    07-06-21 15:37 Hongquan.zhang
// 
// 25    07-06-06 16:16 Hongquan.zhang
// 
// 24    07-05-24 11:36 Hongquan.zhang
// 
// 23    07-05-09 17:45 Hongquan.zhang
// 
// 22    07-04-12 14:02 Hongquan.zhang
// 
// 21    07-03-28 16:42 Hui.shao
// moved business router to namespace Site
// 
// 20    07-03-13 17:12 Hongquan.zhang
// 
// 19    07-02-26 17:51 Hongquan.zhang
// 
// 18    07-01-11 17:09 Hongquan.zhang
// 
// 17    07-01-11 16:09 Hongquan.zhang
// 
// 16    07-01-05 10:59 Hongquan.zhang
// 
// 15    06-12-25 19:33 Hui.shao
// support embedded pathsvc mode
// 
// 14    06-12-25 15:37 Hui.shao
// switch to use the logger of env
// 
// 13    06-12-13 18:48 Hongquan.zhang
// 
// 12    10/09/06 4:18p Hui.shao
// reset publisher if it is unreachable
// 
// 11    06-09-12 20:19 Hui.shao
// added SessionWatchDog
// 
// 10    06-08-29 12:33 Hui.shao
// 
// 9     06-08-24 19:23 Hui.shao
// 
// 8     06-08-16 17:26 Hui.shao
// 
// 7     06-07-31 18:33 Hui.shao
// 
// 6     06-07-24 18:34 Hui.shao
// 
// 5     06-07-17 14:47 Hui.shao
// initial impl of session manager
// 
// 4     06-07-14 14:06 Hui.shao
// 
// 3     06-07-13 13:48 Hui.shao
// 
// 2     06-07-12 13:38 Hui.shao
// added logs
// 
// 1     06-07-11 12:32 Hui.shao
// 
// 5     06-07-10 11:33 Hui.shao
// 
// 4     06-07-06 20:05 Hui.shao
// 
// 3     06-07-05 15:46 Hui.shao
// console demo ready
// ===========================================================================

#include "WeiwooSvcEnv.h"
#include "Log.h"

#include "WeiwooConfig.h"
#include "FileSystemOp.h"
#include <TianShanDefines.h>

namespace ZQTianShan {
namespace Weiwoo {

typedef ::std::vector< Ice::Identity > IdentCollection;


// -----------------------------
// class WeiwooSvcEnv
// -----------------------------
WeiwooSvcEnv::WeiwooSvcEnv(ZQ::common::Log& log, 
						   ZQ::common::NativeThreadPool& threadPool, 
						   Ice::CommunicatorPtr& communicator, 
						   const char* endpoint)
: _thpool(threadPool), _communicator(communicator), _adapter(NULL), _log(log),
  _factory(NULL), _watchDog(*this), _sessEventPublisher(NULL), //_weiwooSnmpAgnet(NULL),
 // configuration starts here
 _ttlIdleSess(DEFAULT_TTL_IDLE_SESSION), _ttlOutOfServiceSess(DEFAULT_TTL_OUTSERVICE_SESSION),
 _ttlPathAllocation(DEFAUL_TTL_PATH_ALLOCATION), _maxTickets(DEFAUL_MAX_PATHTICKETNUM),
 _serviceState(TianShanIce::stNotProvisioned)
 /*_proxyPathMgr(SERVICE_NAME_PathManager ":" DEFAULT_ENDPOINT_PathManager)*/
{	
	_endpoint = (endpoint && strlen(endpoint)>0) ? endpoint : DEFAULT_ENDPOINT_Weiwoo;
	// prepare 
}

bool WeiwooSvcEnv::init(const char* databasePath, const char* runtimeDBFolder)
{
	_log(ZQ::common::Log::L_DEBUG, 
		CLOGFMT(WeiwooSvcEnv, "open adapter %s at %s"),
		ADAPTER_NAME_Weiwoo, _endpoint.c_str());

	try
	{
		//_adapter = _communicator->createObjectAdapterWithEndpoints(ADAPTER_NAME_Weiwoo, endpoint);		
		_adapter = ZQADAPTER_CREATE(_communicator, ADAPTER_NAME_Weiwoo, _endpoint.c_str(), _log);
	}
	catch(Ice::Exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR,CLOGFMT(WeiwooSvcEnv,"Create adapter failed with endpoint=%s and exception is %s"),
			_endpoint.c_str(), ex.ice_name().c_str());
		return false;
	}

	//_proxyPathMgr(SERVICE_NAME_PathManager ":" DEFAULT_ENDPOINT_PathManager)
	_proxyPathMgr = SERVICE_NAME_PathManager;
	_proxyPathMgr+=":";
	_proxyPathMgr+=gWeiwooServiceConfig.szPathManagerEndpoint;


	// adjust the configuration to fit the allowed range
	if (_ttlIdleSess < MIN_TTL_IDLE_SESSION || _ttlIdleSess > MAX_TTL_IDLE_SESSION)
		_ttlIdleSess = DEFAULT_TTL_IDLE_SESSION;

	if (_ttlOutOfServiceSess <= 0 || _ttlOutOfServiceSess > MAX_TTL_OUTSERVICE_SESSION)
		_ttlOutOfServiceSess = DEFAULT_TTL_OUTSERVICE_SESSION;

	if (_ttlPathAllocation <= 0 || _ttlPathAllocation > DEFAUL_TTL_PATH_ALLOCATION)
		_ttlPathAllocation = DEFAUL_TTL_PATH_ALLOCATION;

	if (_ttlPathAllocation < MIN_TTL_PATH_ALLOCATION)
		_ttlPathAllocation = MIN_TTL_PATH_ALLOCATION;

	std::string path = FS::getImagePath();
	if (!path.empty())
	{
		std::string::size_type pos = path.rfind(FNSEPC);
		/* /xxx/bin/yyy*/
		if (pos != std::string::npos)
		{
			/* /xxx/bin */
			std::string tmp = path.substr(0, pos);
			std::string::size_type pos2 = tmp.rfind(FNSEPC);
			if(pos2 != std::string::npos) {
				std::string tmp2 =tmp.substr(pos2);
				if(tmp2 == (FNSEPS "bin") || tmp2 == (FNSEPS "exe")) {
					_programRootPath = tmp;
				}
			}
		}
	}
	else {
		_programRootPath = ".";
	}

	_programRootPath += FNSEPS;

	openDB(databasePath,runtimeDBFolder);

	// init the object factory for Weiwoo objects
	_factory = new WeiwooFactory(*this);
#if ICE_INT_VERSION / 100 >= 306
	commitCbPtr = new TxnStateCB(*this);
	traceCbPtr = new TxnStateCB(*this);
	postYTDCbPtr = new TxnStateCB(*this);
	setUserCbPtr = new TxnStateCB(*this);
	detachCBPtr = new Purchase_detachCB();

	//commitCB = Ice::newCallback(commitCbPtr, &TxnStateCB::commitStateChange);
	//traceCB = Ice::newCallback(traceCbPtr, &TxnStateCB::trace);
	//postYTDCB = Ice::newCallback(postYTDCbPtr, &TxnStateCB::postYTD);
	//setUserCB = Ice::newCallback(setUserCbPtr, &TxnStateCB::setUserProperty);
	//detachCB = Ice::newCallback(detachCBPtr, &Purchase_detachCB::detach);
#else	
	_commitStateChangePtr	= new TxnCommitStateChangeAmiImpl(*this);
	_postYTDPtr				= new TxnpostYTDAmiImpl(*this);
	_setUserPropertyPtr		= new setUserPropertyAmiImpl(*this);
	_txntracePtr			= new TxnTraceAmiImpl(*this);	
	_BussinessRouterEndpoint=gWeiwooServiceConfig.szBusinessRouterEndpoint;
#ifdef _DEBUG
	_ttlIdleSess = 10 * 1000;
#endif // _DEBUG	
	_purchaseDetachPtr = new AMI_Purchase_detachImpl();
#endif
	//
	// bussiness router proxy init
	//
	{		
		const char* pBusinessRouter = gWeiwooServiceConfig.szBusinessRouterEndpoint;
		std::string strRouterEndpoint;
		if (strstr(pBusinessRouter,":") == NULL) 
		{
			strRouterEndpoint = SERVICE_NAME_BusinessRouter":";
			strRouterEndpoint += pBusinessRouter;
		}
		else
		{
			strRouterEndpoint = pBusinessRouter;
		}
		
		_log(ZQ::common::Log::L_INFO, CLOGFMT(WeiwooSvcEnv,"initialize business router proxy with endpint[%s]"),strRouterEndpoint.c_str());

		int64 dwStartTime = ZQTianShan::now();
		try
		{
			_bizPrx = ::TianShanIce::Site::BusinessRouterPrx::uncheckedCast(
				_communicator->stringToProxy(
				strRouterEndpoint.c_str()));
			_log(ZQ::common::Log::L_DEBUG,
				CLOGFMT(WeiwooSvcEnv,"initialize business router proxy successfuly and time count = [%d]"),
				ZQTianShan::now()-dwStartTime);
		}
		catch (const Ice::Exception& ex) 
		{
			_log(ZQ::common::Log::L_ERROR,CLOGFMT(WeiwooSvcEnv,"fail to init Bussiness Router proxy with endpint[%s], caught ICE exception[%s]"),
				strRouterEndpoint.c_str() , ex.ice_name().c_str()	);
			return false;
		}
	}

	//
	// init PathManager proxy
	//
	{
		_log(ZQ::common::Log::L_DEBUG,CLOGFMT(SessStateInService,"initialize PathManager proxy with endpoint[%s]"),
			_proxyPathMgr.c_str()	);

		try
		{
			_pathmgr = ::TianShanIce::Transport::PathManagerPrx::uncheckedCast(_communicator->stringToProxy(_proxyPathMgr));
			_log(ZQ::common::Log::L_INFO,CLOGFMT(SessStateInService,"initialize PathManager proxy successfuly"));
		}
		catch(const ::Ice::Exception& ex)
		{
			_log(ZQ::common::Log::L_ERROR,CLOGFMT(WeiwooSvcEnv,"fail to init PathManager proxy with endpoint[%s], caught ICE exception[%s]"),
				_proxyPathMgr.c_str() , ex.ice_name().c_str()	);
			return false;
		}
	}

	//
	// init TxnService proxy
	//
	{
		const char* pTxnService = gWeiwooServiceConfig.szBusinessRouterEndpoint;
		std::string strTxnService;
		if (strstr(pTxnService,":") == NULL) 
		{
			strTxnService = SERVICE_NAME_BusinessRouter":";
			strTxnService += pTxnService;
		}
		else
		{
			strTxnService = pTxnService;
		}

		_log(ZQ::common::Log::L_DEBUG,CLOGFMT(WeiwooSvcEnv,"init Txn Service router proxy with endpint[%s]"),strTxnService.c_str());
		try
		{
			_tnxPrx = ::TianShanIce::Site::TxnServicePrx::uncheckedCast(
				_communicator->stringToProxy(
				strTxnService.c_str()));

			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(WeiwooSvcEnv, "initialize Txn Service router proxy [%s] successfuly "), strTxnService.c_str() );
		}
		catch (const Ice::Exception& ex) 
		{
			_log(ZQ::common::Log::L_ERROR,
				CLOGFMT(WeiwooSvcEnv,"fail to init Txn service with endpint[%s], caught ICE exception[%s]"),
				strTxnService.c_str() , ex.ice_name().c_str()	);

			return false;
		}
	}


	_log(ZQ::common::Log::L_INFO, CLOGFMT(WeiwooSvcEnv, "initialize weiwoo env successfully"));

	// if (NULL == _weiwooSnmpAgnet)
	// 	_weiwooSnmpAgnet = new ZQ::Snmp::Subagent(200, 5);
	
	// registerSnmp();
	// if (NULL != _weiwooSnmpAgnet)
	//	_weiwooSnmpAgnet->start();

	// _log(ZQ::common::Log::L_INFO, CLOGFMT(WeiwooSvcEnv, "initialize weiwoo snmp %s"), (NULL == _weiwooSnmpAgnet ? "failed" : "successfully"));
	_serviceState = TianShanIce::stProvisioned;
	return true;
}

WeiwooSvcEnv::~WeiwooSvcEnv()
{
	closeDB();
	_adapter=NULL;
	//if(NULL != _weiwooSnmpAgnet)
	//{// it's bad idea to log and delete thing in deconstructor here.
	//	delete _weiwooSnmpAgnet;
	//	_weiwooSnmpAgnet = NULL;
	//	_log(ZQ::common::Log::L_INFO, CLOGFMT(WeiwooSvcEnv, "uninitialize weiwoo snmp successfully"));
	//}

}
/*
int WeiwooSvcEnv::registerSnmp(void)
{
	using namespace ZQ::Snmp;
	int nRev = false;
	int registerCount = 0;

	if (NULL == _weiwooSnmpAgnet)
	{
		_log(ZQ::common::Log::L_INFO, CLOGFMT(WeiwooSvcEnv, "weiwoo registerSnmp failed, weiwooSnmpAgnet[NULL]"));
		return nRev;
	}

	try
	{
		using namespace ZQ::common;
		typedef DECLARE_SNMP_RO_TYPE(SessionWatchDog&,  int (SessionWatchDog::*)(void), int)          SessionCount;
		typedef DECLARE_SNMP_RO_TYPE(NativeThreadPool&, const int (NativeThreadPool::*)(void), int)   PendingSize;
		typedef DECLARE_SNMP_RO_TYPE(NativeThreadPool&, int (NativeThreadPool::*)(void) const, int)   BusyThreads;
		typedef DECLARE_SNMP_RO_TYPE(NativeThreadPool&, int (NativeThreadPool::*)(void) const, int)   ThreadPoolSize;

		_weiwooSnmpAgnet->addObject( Oid("1.2"), ManagedPtr(new SimpleObject(VariablePtr(new SessionCount(_watchDog, &SessionWatchDog::getExpirationsSize) ), AsnType_Integer, aReadOnly)));  ++registerCount;
		_weiwooSnmpAgnet->addObject( Oid("1.3"), ManagedPtr(new SimpleObject(VariablePtr(new PendingSize(_thpool,    &NativeThreadPool::pendingRequestSize)), AsnType_Integer, aReadOnly)));  ++registerCount;
		_weiwooSnmpAgnet->addObject( Oid("1.4"), ManagedPtr(new SimpleObject(VariablePtr(new BusyThreads(_thpool,    &NativeThreadPool::activeCount) ),       AsnType_Integer, aReadOnly)));  ++registerCount;
		_weiwooSnmpAgnet->addObject( Oid("1.5"), ManagedPtr(new SimpleObject(VariablePtr(new ThreadPoolSize(_thpool, &NativeThreadPool::size) ),              AsnType_Integer, aReadOnly)));  ++registerCount;

		nRev = true;
		_log(ZQ::common::Log::L_INFO, CLOGFMT(WeiwooSvcEnv, "initialize weiwoo registerSnmp succeed registerCount[%d]"), registerCount);
	}
	catch (...)//not  allowed  to failed
	{
		nRev = false;
	    _log(ZQ::common::Log::L_ERROR, CLOGFMT(WeiwooSvcEnv, "initialize weiwoo registerSnmp failed registerCount[%d]"), registerCount);
	}

	return nRev;
}
*/

#define SessionDataSubDir "Sessions"
#define Sess_DBSZ         (160*1000) // 160MB

bool WeiwooSvcEnv::openDB(const char* databasePath, const char* dbRuntimePath)
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
#define INSTANCE_INDEX(_IDX) _log(ZQ::common::Log::L_DEBUG, CLOGFMT(WeiwooSvcEnv, "create index: " #_IDX)); \
		                     _idx##_IDX = new TianShanIce::SRM::##_IDX(INDEXFILENAME(_IDX))
//		_log(ZQ::common::Log::L_INFO, CLOGFMT(WeiwooSvcEnv, "opening database at path: %s"), _dbPath.c_str());
//		::CreateDirectory((_dbPath + SessionDataSubDir FNSEPS).c_str(), NULL);
		// FS::createDirectory((_dbRuntimePath + SessionDataSubDir FNSEPS).c_str(), NULL);
		_log(ZQ::common::Log::L_INFO, CLOGFMT(WeiwooSvcEnv, "opening runtime database at path: %s"), _dbRuntimePath.c_str());
		FS::createDirectory((_dbRuntimePath + "Weiwoo" FNSEPS).c_str(), NULL);
		std::string sessDbPathname = createDBFolder(_log, "Weiwoo", _dbRuntimePath.c_str(), SessionDataSubDir, Sess_DBSZ);

		//set ice freeze property for performance tunning
		Ice::PropertiesPtr proper = _adapter->getCommunicator()->getProperties();
		proper->setProperty("Freeze.Evictor.UseNonmutating",      "1");

		std::string dbAttrPrefix = std::string("Freeze.DbEnv.") + sessDbPathname;
		proper->setProperty(dbAttrPrefix + ".DbRecoverFatal",      "1");
		proper->setProperty(dbAttrPrefix + ".DbPrivate",           "0");
		proper->setProperty(dbAttrPrefix + ".OldLogsAutoDelete",   "1");

		if ( strlen( gWeiwooServiceConfig.szWeiwooIceEnvCheckPointPeriod ) > 0 )
		{
			std::string strCheckPointPeriod = "Freeze.DbEnv." + sessDbPathname;
			strCheckPointPeriod = strCheckPointPeriod + ".CheckpointPeriod";
			proper->setProperty( strCheckPointPeriod,  gWeiwooServiceConfig.szWeiwooIceEnvCheckPointPeriod  );
			_log(ZQ::common::Log::L_INFO,CLOGFMT(WeiwooSvcEnv,"set ice freeze property [%s--->%s]"),
				strCheckPointPeriod.c_str() ,
				gWeiwooServiceConfig.szWeiwooIceEnvCheckPointPeriod);
		}
		if ( strlen( gWeiwooServiceConfig.szWeiwooIceEnvDbRecoverFatal ) ) 
		{
			std::string strDBRecoverFatal = "Freeze.DbEnv." + sessDbPathname;
			strDBRecoverFatal = strDBRecoverFatal + ".DbRecoverFatal";
			proper->setProperty( strDBRecoverFatal, gWeiwooServiceConfig.szWeiwooIceEnvDbRecoverFatal );
			_log(ZQ::common::Log::L_INFO,CLOGFMT(WeiwooSvcEnv,"set ice freeze property [%s--->%s]"),
				strDBRecoverFatal.c_str() ,
				gWeiwooServiceConfig.szWeiwooIceEnvDbRecoverFatal);
		}

		if ( strlen( gWeiwooServiceConfig.szFreezeSessionSavePeriod) > 0 )
		{
			std::string strSessionSavePeriod = "Freeze.Evictor." + sessDbPathname;
			strSessionSavePeriod = strSessionSavePeriod + "."  + DBFILENAME_Session +".SavePeriod";
			proper->setProperty( strSessionSavePeriod , gWeiwooServiceConfig.szFreezeSessionSavePeriod );
			_log(ZQ::common::Log::L_INFO,CLOGFMT(WeiwooSvcEnv,"set ice freeze property [%s--->%s]"),
				strSessionSavePeriod.c_str() ,
				gWeiwooServiceConfig.szFreezeSessionSavePeriod);
		}
		if ( strlen(gWeiwooServiceConfig.szFreezeSessionSaveSizeTrigger) > 0 ) 
		{
			std::string strSessSaveSizeTrigger = "Freeze.Evictor." + sessDbPathname;
			strSessSaveSizeTrigger = strSessSaveSizeTrigger + "." + DBFILENAME_Session + ".SaveSizeTrigger";
			proper->setProperty(strSessSaveSizeTrigger , gWeiwooServiceConfig.szFreezeSessionSaveSizeTrigger);
			_log(ZQ::common::Log::L_INFO,CLOGFMT(WeiwooSvcEnv,"set ice freeze property [%s--->%s]"),
				strSessSaveSizeTrigger.c_str() ,
				gWeiwooServiceConfig.szFreezeSessionSaveSizeTrigger);
		}

//		INSTANCE_INDEX(IdToSess);
		{
			std::vector<Freeze::IndexPtr> indices;
//			indices.push_back(_idxIdToSess);
			
			BerkeleyDBConfig conf;
			if(!conf.generateConfig(sessDbPathname))
			{
				_log(ZQ::common::Log::L_WARNING, CLOGFMT(PathSvcEnv, "failed to generate DB_CONFIG for %s"), DBFILENAME_Session);
			}

			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(WeiwooSvcEnv, "create evictor %s with index %s"), DBFILENAME_Session, "IdToSess");
#if ICE_INT_VERSION / 100 >= 303
			_eSession = ::Freeze::createBackgroundSaveEvictor(_adapter, sessDbPathname, DBFILENAME_Session, 0, indices);
#else
			_eSession = Freeze::createEvictor(_adapter, sessDbPathname, DBFILENAME_Session, 0, indices);
#endif
			_adapter->addServantLocator(_eSession, DBFILENAME_Session);
			_eSession->setSize(gWeiwooServiceConfig.lEvictorWeiwooSessSize>500 ? gWeiwooServiceConfig.lEvictorWeiwooSessSize:500);
		}

		return true;
	}
	catch(const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_log,"WeiwooSvcEnv",1001,CLOGFMT(WeiwooSvcEnv, "openDB() caught exception: %s"), ex.ice_name().c_str());
	}
	catch(...)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (_log,"WeiwooSvcEnv",1002, CLOGFMT(WeiwooSvcEnv, "openDB() caught unkown exception"));
	}

	_log(ZQ::common::Log::L_INFO, CLOGFMT(WeiwooSvcEnv, "database ready"));

	return true;
}

void WeiwooSvcEnv::closeDB(void)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(WeiwooSvcEnv, "close local database"));

	_eSession      = NULL;
//	_idxIdToSess	= NULL;
}

#ifdef WITH_ICESTORM
void WeiwooSvcEnv::openPublisher(::IceStorm::TopicManagerPrx topicManager)
{
	if (!topicManager)
	{
		_log(ZQ::common::Log::L_WARNING, 
			CLOGFMT(WeiwooSvcEnv, "openPublisher() null topic manager, ignore publishing events"));
		return;
	}

	::IceStorm::TopicPrx topic;
	try 
	{
		_log(ZQ::common::Log::L_DEBUG,
				CLOGFMT(WeiwooSvcEnv, "openPublisher() open topic \"%s\" on %s"), 
				::TianShanIce::SRM::TopicOfSession.c_str(), _communicator->proxyToString(topicManager).c_str());
		topic = topicManager->retrieve(::TianShanIce::SRM::TopicOfSession);
	}
	catch(const IceStorm::NoSuchTopic& )
	{
		_log(ZQ::common::Log::L_WARNING, 
				CLOGFMT(WeiwooSvcEnv, "openPublisher() no topic \"%s\" exists, create one"), 
				::TianShanIce::SRM::TopicOfSession.c_str());

		// create a new topic on the topic manager
		topic = topicManager->create(::TianShanIce::SRM::TopicOfSession);
	}

	try 
	{
		::Ice::ObjectPrx proxy = topic->getPublisher();
		if (!proxy->ice_isDatagram())
			proxy->ice_oneway();
		
		ZQ::common::MutexGuard gd(_lockSessEventSink);
		_sessEventPublisher = TianShanIce::SRM::SessionEventSinkPrx::uncheckedCast(proxy);
		_sessEventPublisher->ice_ping();
	}
	catch (const ::Ice::Exception& e)
	{
		_log(ZQ::common::Log::L_ERROR,
			CLOGFMT(WeiwooSvcEnv, "openPublisher() exception caught: %s"),
			e.ice_name().c_str());
		_sessEventPublisher = NULL;
	}
	
	if (!_sessEventPublisher)
		_log(ZQ::common::Log::L_ERROR,
			CLOGFMT(WeiwooSvcEnv, "openPublisher() failed to open event publisher, no events will be sent"));
}

#endif // WITH_ICESTORM

#if  ICE_INT_VERSION / 100 >= 306
TxnStateCB::TxnStateCB(WeiwooSvcEnv& env):_env(env)
{
}
void TxnStateCB::commitStateChange(const Ice::AsyncResultPtr& r)
{
		TianShanIce::Site::TxnServicePrx TxnProxy = TianShanIce::Site::TxnServicePrx::uncheckedCast(r->getProxy());
	try
	{
		TxnProxy->end_commitStateChange(r);
	}
	catch(const Ice::Exception& ex)
	{
		handleException(ex);
	}
}
void TxnStateCB::trace(const Ice::AsyncResultPtr& r)
{
		TianShanIce::Site::TxnServicePrx TxnProxy = TianShanIce::Site::TxnServicePrx::uncheckedCast(r->getProxy());
	try
	{
		TxnProxy->end_trace(r);
	}
	catch(const Ice::Exception& ex)
	{
		handleException(ex);
	}
}
void TxnStateCB::postYTD(const Ice::AsyncResultPtr& r)
{
		TianShanIce::Site::TxnServicePrx TxnProxy = TianShanIce::Site::TxnServicePrx::uncheckedCast(r->getProxy());
	try
	{
		TxnProxy->end_postYTD(r);
	}
	catch(const Ice::Exception& ex)
	{
		handleException(ex);
	}
}
void TxnStateCB::setUserProperty(const Ice::AsyncResultPtr& r)
{
		TianShanIce::Site::TxnServicePrx TxnProxy = TianShanIce::Site::TxnServicePrx::uncheckedCast(r->getProxy());
	try
	{
		TxnProxy->end_setUserProperty(r);
	}
	catch(const Ice::Exception& ex)
	{
		handleException(ex);
	}
}
void Purchase_detachCB::detach(const Ice::AsyncResultPtr& r)
{
		TianShanIce::Application::PurchasePrx purProxy = TianShanIce::Application::PurchasePrx::uncheckedCast(r->getProxy());
	try
	{
		purProxy->end_detach(r);
	}
	catch(const Ice::Exception& ex)
	{
		handleException(ex);
	}
}
#endif
}} // namespace
