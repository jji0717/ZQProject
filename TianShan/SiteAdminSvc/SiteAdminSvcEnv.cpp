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
// $Log: /ZQProjs/TianShan/SiteAdminSvc/SiteAdminSvcEnv.cpp $
// 
// 2     12/12/13 2:00p Hui.shao
// %lld for int64
// 
// 1     10-11-12 16:07 Admin
// Created.
// 
// 1     10-11-12 15:41 Admin
// Created.
// 
// 23    10-06-14 10:38 Hui.shao
// 
// 22    08-11-24 11:48 Hongquan.zhang
// 
// 21    08-08-19 12:43 Hongquan.zhang
// 
// 20    08-07-25 17:37 Xiaohui.chai
// Changed the MDB schema
// 
// 19    08-04-30 16:25 Xiaohui.chai
// 
// 18    08-04-21 15:50 Guan.han
// 
// 17    07-12-14 16:36 Xiaohui.chai
// 
// 16    07-12-13 19:44 Hui.shao
// 
// 15    07-12-13 18:36 Xiaohui.chai
// 
// 14    07-12-13 18:27 Hui.shao
// 
// 13    07-12-10 18:47 Hui.shao
// moved event out of txn
// 
// 12    07-10-25 14:09 Hongquan.zhang
// 
// 11    07-08-30 15:38 Hongquan.zhang
// 
// 10    07-06-21 15:37 Hongquan.zhang
// 
// 9     07-06-15 17:49 Hongquan.zhang
// 
// 8     07-06-06 18:28 Hui.shao
// modified ZQ adpater
// 
// 7     07-05-23 13:32 Hui.shao
// use wrappered adapter & _IceThrow
// 
// 6     07-05-14 15:50 Hongquan.zhang
// 
// 4     07-04-20 15:09 Hongquan.zhang
// 
// 3     07-04-12 13:46 Hongquan.zhang
// 
// 2     07-03-28 18:35 Hui.shao
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
// 5     06-07-17 14:47 Hui.shao
// initial impl of session manager
// 
// 2     06-07-12 13:38 Hui.shao
// added logs
// 
// 3     06-07-05 15:46 Hui.shao
// console demo ready
// ===========================================================================

#include "SiteAdminSvcEnv.h"
#include "Log.h"
#include "SiteAdminImpl.h"
#include "SiteAdminSvc.h"
#include "SiteServiceConfig.h"

extern ZQ::common::Config::Loader<SAConfig> gSiteAdminConfig;

namespace ZQTianShan {
namespace Site {

typedef ::std::vector< Ice::Identity > IdentCollection;

// -----------------------------
// class SiteAdminSvcEnv
// -----------------------------
SiteAdminSvcEnv::SiteAdminSvcEnv(ZQ::common::Log& log, ZQ::common::NativeThreadPool& threadPool, 
		Ice::CommunicatorPtr& communicator, const char* endpoint, const char* databasePath,const char* runtimeDbPath )
: _thpool(threadPool), _communicator(communicator), _log(log),
  _pAppDict(NULL), _pSiteDict(NULL) // , _factory(NULL)
{
	_endpoint = (endpoint && strlen(endpoint)>0) ? endpoint : DEFAULT_ENDPOINT_SiteAdminSvc;
	
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdminSvc, "opening adapter %s at %s"), ADAPTER_NAME_SiteAdminSvc, _endpoint.c_str());
	try
	{
		//_adapter = _communicator->createObjectAdapterWithEndpoints(ADAPTER_NAME_SiteAdminSvc, endpoint);	
        _adapter = ZQADAPTER_CREATE(_communicator, ADAPTER_NAME_SiteAdminSvc, _endpoint.c_str(), _log);
	}
	catch(Ice::Exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR,CLOGFMT(WeiwooSvcEnv,"Create adapter failed with endpoint=%s and exception is %s"),
							endpoint,ex.ice_name().c_str());
		throw ex;
	}

	char path[MAX_PATH];
	if (::GetModuleFileName(NULL, path, MAX_PATH-1)>0)
	{
		char* p = strrchr(path, FNSEPC);
		if (NULL !=p)
		{
			*p='\0';
			p = strrchr(path, FNSEPC);
			if (NULL !=p && (0==stricmp(FNSEPS "bin", p) || 0==stricmp(FNSEPS "exe", p)))
				*p='\0';
		}
		_programRootPath = path;
	}
	else _programRootPath = ".";

	_programRootPath += FNSEPS;

	if(!openDB( databasePath  , runtimeDbPath ))
    {
        ::ZQTianShan::_IceThrow<::TianShanIce::ServerError> (_log, EXPFMT(SiteAdminSvcEnv, 201, "Failed to initialize work DB."));
    }

    if( gSiteAdminConfig.lTxnDataEnabled )
    {
        if(!initTxnMDB())
        {
            ::ZQTianShan::_IceThrow<::TianShanIce::ServerError> (_log, EXPFMT(SiteAdminSvcEnv, 202, "Failed to initialize TxnData MDB"));
        }

    }
	initWithConfig();

	// init the object factory for SiteAdminSvc objects
	_factory = new SASFactory(*this);
	_txnWatchDog= new LivetxnWatchDog(*this);
	_liveTxnTransfer=new LiveTxnTransfer(*this);
//	_txnWatchDog->start();
//	_liveTxnTransfer->start();
}

SiteAdminSvcEnv::~SiteAdminSvcEnv()
{
	try{delete _txnWatchDog;}catch (...) {}
	try{delete _liveTxnTransfer;}catch(...){ }
	closeDB();
	_adapter=NULL;
}

#define BusinessDataSubDir "Sites"
#define LiveTxnDataSubDir "TxnLive"
#define TxnYTDDataSubDir "TxnYTD"

bool SiteAdminSvcEnv::openDB( const char* databasePath , const char* runtimeDbPath )
{
	closeDB();

	std::string strRuntimeDbPath;
	if (NULL == databasePath || strlen(databasePath) <1)
		_dbPath = _programRootPath + "data" FNSEPS;
	else _dbPath = databasePath;

	if (FNSEPC != _dbPath[_dbPath.length()-1])
		_dbPath += FNSEPS;

	if( runtimeDbPath == NULL || runtimeDbPath[0] == 0 )
	{
		strRuntimeDbPath = _dbPath;
	}
	else
	{
		strRuntimeDbPath = runtimeDbPath;
	}
	if (FNSEPC != strRuntimeDbPath[strRuntimeDbPath.length()-1])
		strRuntimeDbPath += FNSEPS;


	try 
	{

		_log(ZQ::common::Log::L_INFO, CLOGFMT(SiteAdminSvcEnv, "opening database at path: %s"), _dbPath.c_str());
		
		::CreateDirectory((_dbPath + BusinessDataSubDir FNSEPS).c_str(), NULL);
		_conn = Freeze::createConnection(_adapter->getCommunicator(), (_dbPath + BusinessDataSubDir FNSEPS));

		// open the dictionaries
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdminSvcEnv, "create dictionary: %s"), DBFILENAME_AppDict);
		_pAppDict		= new TianShanIce::Site::AppDict(_conn, DBFILENAME_AppDict);
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdminSvcEnv, "create dictionary: %s"), DBFILENAME_SiteDict);
		_pSiteDict		= new TianShanIce::Site::SiteDict(_conn, DBFILENAME_SiteDict);

		// open the Indexes
#define INSTANCE_INDEX2(_IDX, _IDXTYPE) _log(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdminSvcEnv, "create index: " #_IDX)); \
		_idx##_IDX = new TianShanIce::Site::##_IDXTYPE(INDEXFILENAME(_IDXTYPE))
#define INSTANCE_INDEX(_IDX) INSTANCE_INDEX2(_IDX, _IDX)
		
		INSTANCE_INDEX(AppToMount);
		INSTANCE_INDEX(SiteToMount);
	
		{
			std::vector<Freeze::IndexPtr> indices;
			indices.push_back(_idxAppToMount);
			indices.push_back(_idxSiteToMount);
			
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdminSvcEnv, "create evictor %s with index %s, %s"), DBFILENAME_AppMount, "AppToMount", "SiteToMount");
			
#if ICE_INT_VERSION / 100 >= 303			
			_eAppMount = Freeze::createBackgroundSaveEvictor(_adapter, _dbPath + BusinessDataSubDir FNSEPS, DBFILENAME_AppMount, 0, indices);
#else
			_eAppMount = Freeze::createEvictor(_adapter, _dbPath + BusinessDataSubDir FNSEPS, DBFILENAME_AppMount, 0, indices);
#endif
			_adapter->addServantLocator(_eAppMount, DBFILENAME_AppMount);
			_eAppMount->setSize(100);
		}

		::CreateDirectory((_dbPath + LiveTxnDataSubDir FNSEPS).c_str(), NULL);

		INSTANCE_INDEX(SiteToTxn);
		INSTANCE_INDEX(MountToTxn);
	
		INSTANCE_INDEX(TxnToEvent);

		
		_log(ZQ::common::Log::L_INFO, 
			CLOGFMT(SiteAdminSvcEnv, "opening runtime database at path: %s"),
			strRuntimeDbPath.c_str());

		::CreateDirectory(( strRuntimeDbPath + LiveTxnDataSubDir FNSEPS).c_str(), NULL);
		_conn = Freeze::createConnection(_adapter->getCommunicator(), (strRuntimeDbPath + LiveTxnDataSubDir FNSEPS));

		Ice::PropertiesPtr proper = _adapter->getCommunicator()->getProperties();
		std::string strCheckPointPeriod = "Freeze.DbEnv." +strRuntimeDbPath + LiveTxnDataSubDir FNSEPS;		
		strCheckPointPeriod = strCheckPointPeriod + ".CheckpointPeriod";
		proper->setProperty( strCheckPointPeriod,  gSiteAdminConfig.performanceTune.strCheckpointPeriod  );
		_log(ZQ::common::Log::L_INFO,CLOGFMT(SiteAdminSvcEnv,"set ice freeze property [%s--->%s]"),
				strCheckPointPeriod.c_str() ,	gSiteAdminConfig.performanceTune.strCheckpointPeriod.c_str() );

		std::string strDBRecoverFatal = "Freeze.DbEnv." + strRuntimeDbPath + LiveTxnDataSubDir FNSEPS;
		strDBRecoverFatal = strDBRecoverFatal + ".DbRecoverFatal";
		proper->setProperty( strDBRecoverFatal, gSiteAdminConfig.performanceTune.strDbRecoverFatal  );
		_log(ZQ::common::Log::L_INFO,CLOGFMT(SiteAdminSvcEnv,"set ice freeze property [%s--->%s]"),
			strDBRecoverFatal.c_str() ,	gSiteAdminConfig.performanceTune.strDbRecoverFatal.c_str() );

		std::string strSessionSavePeriod = "Freeze.Evictor." + strRuntimeDbPath + LiveTxnDataSubDir FNSEPS;
		strSessionSavePeriod = strSessionSavePeriod + "."  + DBFILENAME_TxnEvent +".SavePeriod";
		proper->setProperty( strSessionSavePeriod , gSiteAdminConfig.performanceTune.strLivTxnSavePeriod );
		_log(ZQ::common::Log::L_INFO,CLOGFMT(SiteAdminSvcEnv,"set ice freeze property [%s--->%s]"),
			strSessionSavePeriod.c_str() ,	gSiteAdminConfig.performanceTune.strLivTxnSavePeriod.c_str() );

		std::string strSessSaveSizeTrigger = "Freeze.Evictor." + strRuntimeDbPath + LiveTxnDataSubDir FNSEPS ;
		strSessSaveSizeTrigger = strSessSaveSizeTrigger + "." + DBFILENAME_TxnEvent + ".SaveSizeTrigger";
		proper->setProperty(strSessSaveSizeTrigger , gSiteAdminConfig.performanceTune.strLivTxnSaveSizeTrigger );
		_log(ZQ::common::Log::L_INFO,CLOGFMT(SiteAdminSvcEnv,"set ice freeze property [%s--->%s]"),
			strSessSaveSizeTrigger.c_str() , gSiteAdminConfig.performanceTune.strLivTxnSaveSizeTrigger.c_str() );

		strSessionSavePeriod = "Freeze.Evictor." + strRuntimeDbPath + LiveTxnDataSubDir FNSEPS;
		strSessionSavePeriod = strSessionSavePeriod + "."  + DBFILENAME_Txn +".SavePeriod";
		proper->setProperty( strSessionSavePeriod , gSiteAdminConfig.performanceTune.strLivTxnSavePeriod );
		_log(ZQ::common::Log::L_INFO,CLOGFMT(SiteAdminSvcEnv,"set ice freeze property [%s--->%s]"),
			strSessionSavePeriod.c_str() ,	gSiteAdminConfig.performanceTune.strLivTxnSavePeriod.c_str() );

		strSessSaveSizeTrigger = "Freeze.Evictor." + strRuntimeDbPath + LiveTxnDataSubDir FNSEPS ;
		strSessSaveSizeTrigger = strSessSaveSizeTrigger + "." + DBFILENAME_Txn + ".SaveSizeTrigger";
		proper->setProperty(strSessSaveSizeTrigger , gSiteAdminConfig.performanceTune.strLivTxnSaveSizeTrigger );
		_log(ZQ::common::Log::L_INFO,CLOGFMT(SiteAdminSvcEnv,"set ice freeze property [%s--->%s]"),
			strSessSaveSizeTrigger.c_str() , gSiteAdminConfig.performanceTune.strLivTxnSaveSizeTrigger.c_str() );

		{
			std::vector<Freeze::IndexPtr> indices;
			indices.push_back(_idxTxnToEvent);
			
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdminSvcEnv, "create evictor %s with index %s"), DBFILENAME_TxnEvent, "TxnToEvent");
			
#if ICE_INT_VERSION / 100 >= 303			
			_eTxnEvent = Freeze::createBackgroundSaveEvictor(_adapter, strRuntimeDbPath + LiveTxnDataSubDir FNSEPS, DBFILENAME_TxnEvent, 0, indices);
#else
			_eTxnEvent = Freeze::createEvictor(_adapter, strRuntimeDbPath + LiveTxnDataSubDir FNSEPS, DBFILENAME_TxnEvent, 0, indices);
#endif
			_adapter->addServantLocator(_eTxnEvent, DBFILENAME_TxnEvent);
			_eTxnEvent->setSize(100);
		}
		{
			std::vector<Freeze::IndexPtr> indices;
			indices.push_back(_idxSiteToTxn);
			indices.push_back(_idxMountToTxn);

			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdminSvcEnv, "create evictor %s with index %s, %s, %s"), DBFILENAME_Txn, "SessToTxn", "SiteToTxn", "AppToTxn");

#if ICE_INT_VERSION / 100 >= 303
			_eLiveTxn = Freeze::createBackgroundSaveEvictor(_adapter, strRuntimeDbPath + LiveTxnDataSubDir FNSEPS, DBFILENAME_Txn, 0, indices);
#else
			_eLiveTxn = Freeze::createEvictor( _adapter, strRuntimeDbPath + LiveTxnDataSubDir FNSEPS, DBFILENAME_Txn, 0, indices);
#endif
			_adapter->addServantLocator(_eLiveTxn, DBFILENAME_Txn);
			_eLiveTxn->setSize(100);
		}


#ifdef YTD_EVICTOR
		//prepare for YTD TXN
		::CreateDirectory((_dbPath + TxnYTDDataSubDir FNSEPS).c_str(), NULL);
		INSTANCE_INDEX2(SessToYTDTxn, SessToTxn);
		INSTANCE_INDEX2(YTDTxnToEvent, TxnToEvent);

		{
			std::vector<Freeze::IndexPtr> indices;
			indices.push_back(_idxSessToYTDTxn);
			
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdminSvcEnv, "create evictor %s with index %s"), DBFILENAME_Txn, "SessToTxn");
			
#if ICE_INT_VERSION / 100 >= 303
			_eYTDTxn = Freeze::createBackgroundSaveEvictor(_adapter, _dbPath + TxnYTDDataSubDir FNSEPS, DBFILENAME_Txn ".dat", 0, indices);
#else
			_eYTDTxn = Freeze::createEvictor(_adapter, _dbPath + TxnYTDDataSubDir FNSEPS, DBFILENAME_Txn ".dat", 0, indices);
#endif
			_adapter->addServantLocator(_eYTDTxn, "YTD" DBFILENAME_Txn);
			_eYTDTxn->setSize(10);
		}

		{
			std::vector<Freeze::IndexPtr> indices;
			indices.push_back(_idxYTDTxnToEvent);
			
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdminSvcEnv, "create YTD evictor %s with index %s"), DBFILENAME_TxnEvent, "TxnToEvent");
			
#if ICE_INT_VERSION / 100 >= 303			
			_eYTDEvent = Freeze::createBackgroundSaveEvictor(_adapter, _dbPath + TxnYTDDataSubDir FNSEPS, DBFILENAME_TxnEvent ".dat", 0, indices);
#else
			_eYTDEvent = Freeze::createEvictor(_adapter, _dbPath + TxnYTDDataSubDir FNSEPS, DBFILENAME_TxnEvent ".dat", 0, indices);
#endif
			_adapter->addServantLocator(_eYTDEvent, "YTD" DBFILENAME_TxnEvent);
			_eTxnEvent->setSize(10);
		}
#endif YTD_EVICTOR
		

		return true;
	}
	catch(const Ice::Exception& ex)
	{
		::ZQTianShan::_IceThrow<::TianShanIce::ServerError> (_log, EXPFMT(SiteAdminSvcEnv, 101, "openDB() caught exception: %s"), ex.ice_name().c_str());
	}
	catch(...)
	{
		::ZQTianShan::_IceThrow<::TianShanIce::ServerError> (_log, EXPFMT(SiteAdminSvcEnv, 102, "openDB() caught unkown exception"));
	}

	_log(ZQ::common::Log::L_INFO, CLOGFMT(SiteAdminSvcEnv, "database ready"));

	return true;
}

void SiteAdminSvcEnv::closeDB(void)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(SiteAdminSvcEnv, "close local database"));

	if (NULL != _pAppDict)
		delete _pAppDict;
	 _pAppDict=NULL;

	if (NULL != _pSiteDict)
		delete _pSiteDict;
	 _pSiteDict=NULL;

	_eAppMount      = NULL;
	_idxAppToMount	= NULL;
	_idxSiteToMount = NULL;
}

bool SiteAdminSvcEnv::initTxnMDB()
{
    try
    {
        std::vector<std::string> dbScheme;
        dbScheme.push_back("create table Sessions("
                           "Session char(50) primary key NOT NULL"
                           ", Site char(50)"
                           ", Path char(50)"
                           ", Storage char(50)"
                           ", Streamer char(50)"
                           ", ServiceGroup char(50)"
                           ", Bandwidth char(30)"
                           ", Stream char(50)"
                           ", ProvisionedAt char(30)"
                           ", InServiceAt char(30)"
                           ", OutOfServiceAt char(30)"
                           ", TeardownReason TEXT"
                           ", TerminateReason TEXT"
                           ", ClientSessionId char(50)"
                           ", ClientAddress char(50)"
                           ", OrginalUrl TEXT"
                           ")");
        dbScheme.push_back("create table Events(Session char(50) NOT NULL, stampUTC char(30), category char(30), eventCode char(20), eventMsg char(200))");
        _mdbLog.initialize(gSiteAdminConfig.szTxnDataDest, gSiteAdminConfig.szTxnDataTemplate, dbScheme, gSiteAdminConfig.lTxnDataSize, gSiteAdminConfig.lTxnDataNumber);
    }
    catch (const ZQ::common::MdbLogError& ex)
    {
        _log(ZQ::common::Log::L_ERROR, CLOGFMT(SiteAdminSvcEnv, "initialize MdbLog caught %s"), ex.getString());
        return false;
	}
    return true;
}
#if 0 && defined(WITH_ICESTORM)
void SiteAdminSvcEnv::openPublisher(::IceStorm::TopicManagerPrx topicManager)
{
#pragma message ( __MSGLOC__ "TODO: impl here")
}
#endif // WITH_ICESTORM

//////////////////////////////////////////////////////////////////////////

LiveTxnTransfer::LiveTxnTransfer(SiteAdminSvcEnv& env):_env(env)
{
	_bQuit = false;
	_hEvent =CreateEvent(NULL,FALSE,FALSE,NULL);
}
LiveTxnTransfer::~LiveTxnTransfer()
{
	_bQuit = true;
	waitHandle(1000);
	CloseHandle(_hEvent);
}
void LiveTxnTransfer::AddSess(const std::string& sessID)
{
	ZQ::common::MutexGuard gd(_listMutex);
	_sessList.push_back(sessID);
	SetEvent(_hEvent);
}
int LiveTxnTransfer::run()
{
	SetEvent(_hEvent);//run at first
	while (!_bQuit)
	{
		// yield during busy timeframe
		if (WAIT_OBJECT_0 != WaitForSingleObject(_hEvent, 10000))
			continue;

		//transfer liveTxn into YTD database
		while (!_bQuit && _sessList.size() > 0)
		{
			std::string strID;
			{
				ZQ::common::MutexGuard gd(_listMutex);
				if (_sessList.size() == 0) 
					continue;

				strID=_sessList.back();
				_sessList.pop_back();
			}
			
			Ice::ObjectPtr basePtr = NULL;
				
			try {
				envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(LiveTxnTransfer, "removing LiveTxn(%s)"), strID.c_str());
				Ice::Identity id ;
				id.name	      = strID;
				id.category   = DBFILENAME_Txn;
				basePtr       = _env._eLiveTxn->remove(id);
			}
			catch(...)
			{
				envlog(ZQ::common::Log::L_WARNING, CLOGFMT(LiveTxnTransfer, "removing LiveTxn(%s) caught exception"), strID.c_str());
			}
			
			try {
				if (!basePtr)
					continue;
//				envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(LiveTxnTransfer, "posting LiveTxn(%s) to YTD databse"), strID.c_str());
//				LiveTxnImpl::Ptr txnPtr;
//				txnPtr=IceUtil::Handle<LiveTxnImpl>::dynamicCast(basePtr);	
//				Ice::Identity YTDId;
//				YTDId.name			= strID;
//				YTDId.category		= DBFILENAME_YTDTxn;
//				_env._eYTDTxn->add(txnPtr, YTDId);
			}
			catch(...)
			{
				envlog(ZQ::common::Log::L_WARNING, CLOGFMT(LiveTxnTransfer, "posting LiveTxn(%s) to YTD databse caught exception"), strID.c_str());
			}
		} // while _sessList
	}

	return 1;
}

//////////////////////////////////////////////////////////////////////////
class LiveTxnTimer : public ZQ::common::ThreadRequest
{
public:
	LiveTxnTimer(ZQ::common::NativeThreadPool& pool,TianShanIce::Site::LiveTxnPrx prx):ZQ::common::ThreadRequest(pool)
	{
		_txnPrx = prx;
	}
	int run()
	{
		try
		{
			_txnPrx->onTimer();
		}
		catch (...)
		{
		}
		return 1;
	}
	void final(int retcode /* =0 */, bool bCancelled /* =false */)
	{
		delete this;
	}
private:
	TianShanIce::Site::LiveTxnPrx	_txnPrx;
};

LivetxnWatchDog::LivetxnWatchDog(SiteAdminSvcEnv& env):_env(env)
{
	_bQuit = false;
	_hEvent =CreateEvent(NULL,FALSE,FALSE,NULL);
	_nextWakeup =now()+0xffffff;
}
LivetxnWatchDog::~LivetxnWatchDog()
{
	_bQuit=true;
	SetEvent(_hEvent);
	waitHandle(1000);
	CloseHandle(_hEvent);
}
void LivetxnWatchDog::WatchMe(const std::string& txnId,Ice::Long lMilliSec)
{
	Ice::Long lExpired = lMilliSec + now();
	ZQ::common::MutexGuard gd(_mapMutex);	
//	_env._log(ZQ::common::Log::L_DEBUG,"WatchDog() Txn:%s update Timer %lld from now to %lld",
//				txnId.c_str(),lMilliSec,lExpired);
	_watchMap[txnId] = lExpired ;
	if( lExpired < _nextWakeup )
	{
		SetEvent( _hEvent );
	}
}
void LivetxnWatchDog::UnWatchMe(const std::string& txnId)
{
	ZQ::common::MutexGuard gd(_mapMutex);
	WATCHMAP::iterator it=_watchMap.find(txnId);
	if( it != _watchMap.end() )
		_watchMap.erase(it);
}

int LivetxnWatchDog::run()
{
	SetEvent(_hEvent);//run at first
	Ice::Long	waitTime=0;
	Ice::Long	nowTime;
	std::vector<std::string>		executeTxnIDs;
	std::vector<std::string>::iterator	itExecute;
	Ice::Identity idTxn;
	idTxn.category = DBFILENAME_Txn;
	while ( !_bQuit )
	{
		nowTime = now();
		//calculate the 
		executeTxnIDs.clear();
		{
			ZQ::common::MutexGuard gd(_mapMutex);
			_nextWakeup = nowTime  + 0xffffffff;
			for(WATCHMAP::const_iterator it=_watchMap.begin() ; it != _watchMap.end(); it++ )
			{
				if( it->second <= nowTime )
				{
					//need to call OnTimer
					executeTxnIDs.push_back(it->first);
				}
				else
				{
					_nextWakeup = it->second > _nextWakeup ?_nextWakeup : it->second;
				}
			}
			for( itExecute=executeTxnIDs.begin() ; itExecute != executeTxnIDs.end() ; itExecute++ )
			{
				_watchMap.erase(*itExecute);
			}
		}
		if(_bQuit)
			break;

		for(int i=0;i<(int)executeTxnIDs.size();i++)
		{
			idTxn.name = executeTxnIDs[i];
			if( !idTxn.name.empty() )
			{
				try
				{
					TianShanIce::Site::LiveTxnPrx livetxn = TianShanIce::Site::LiveTxnPrx::checkedCast(_env._adapter->createProxy(idTxn));
					
					if( _bQuit ) return 1;

					if(livetxn)
					{
						(new LiveTxnTimer(_env._thpool,livetxn))->start();
						
					}			
				}
				catch (Ice::Exception& ex)
				{
					printf("%s\n",ex.ice_name().c_str());
				}
				catch(...)
				{
				}
			}
		}
		waitTime = _nextWakeup -now() ;
		waitTime = waitTime >2 ? waitTime : 2 ;
		WaitForSingleObject(_hEvent,(DWORD)waitTime );

	}
	return 1;

}



}} // namespace
