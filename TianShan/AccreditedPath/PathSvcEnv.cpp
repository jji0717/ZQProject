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
// Ident : $Id: PathSvcEnv.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/AccreditedPath/PathSvcEnv.cpp $
// 
// 25    11/10/15 4:50p Hui.shao
// 
// 27    11/10/15 4:44p Hui.shao
// 
// 26    8/20/15 4:23p Hui.shao
// fixed Niu's unsigned index in dumpUsageStat()
// 
// 25    6/18/15 11:03a Hui.shao
// 
// 24    6/17/15 7:00p Hui.shao
// 
// 23    6/17/15 5:23p Hui.shao
// ticket#17865 to export serviceGroup usage via csv
// 
// 22    5/20/15 11:41a Hui.shao
// merged from main tree about AliveStreamerCollector
// 
// 22    5/06/15 6:00p Hui.shao
// removed dict-index, reimpl AliveStreamer Collector
// 
// 20    4/16/15 3:58p Hui.shao
// 
// 19    3/16/15 2:33p Build
// 
// 18    3/11/15 5:11p Hui.shao
// 
// 17    8/13/14 5:44p Hui.shao
// re-impl doValidation()
// 
// 16    2/20/14 5:25p Hui.shao
// merged from V1.16
// 
// 17    2/14/14 11:52a Hui.shao
// added config  _streamLinksByMaxTicket
// 
// 16    8/28/13 3:22p Hui.shao
// about pathticket restoring
// 
// 15    8/20/13 1:00p Hui.shao
// correct the initial value of _gTicketCount
// 
// 14    4/11/13 5:11p Hui.shao
// 
// 13    3/26/13 10:10a Hui.shao
// 
// 12    2/06/13 11:25a Hui.shao
// 
// 11    2/06/13 10:15a Hui.shao
// enlarged DB_CONFIG
// 
// 10    1/06/12 11:39a Hui.shao
// 
// 9     12/28/11 10:30p Hui.shao
// 
// 8     12/28/11 10:25p Hui.shao
// rolled back the paths db into a same dir
// 
// 7     3/10/11 3:56p Hongquan.zhang
// 
// 6     3/10/11 2:20p Hongquan.zhang
// 
// 5     3/09/11 4:26p Hongquan.zhang
// 
// 2     3/07/11 4:55p Fei.huang
// + migrate to linux
// 
// 1     10-11-12 16:02 Admin
// Created.
// 
// 1     10-11-12 15:35 Admin
// Created.
// 
// 44    10-06-18 18:40 Hui.shao
// take createDBFolder() to init DB folder instead
// 
// 43    10-06-18 17:09 Build
// 
// 42    10-06-18 16:05 Hui.shao
// 
// 41    09-02-25 11:30 Hongquan.zhang
// receive and update replicas information
// 
// 40    08-08-14 14:44 Hui.shao
// merged from TianShan 1.7.10
// 
// 40    08-07-08 13:41 Hui.shao
// fixed for ICE 3.3 new syntax
// 
// 39    08-04-23 11:14 Hongquan.zhang
// Add Ice Performance tunning Configuration in OpenDB
// 
// 38    07-12-14 18:04 Hongquan.zhang
// Use ZQTianShan::Adapter
// 
// 37    07-12-14 11:37 Hongquan.zhang
// Check in for updating ErrorCode
// 
// 36    07-09-18 12:55 Hongquan.zhang
// 
// 35    07-06-21 15:36 Hongquan.zhang
// 
// 34    07-05-24 11:19 Hongquan.zhang
// 
// 33    07-05-09 17:43 Hongquan.zhang
// 
// 31    07-03-14 12:33 Hongquan.zhang
// 
// 30    07-01-13 18:43 Hongquan.zhang
// 
// 29    07-01-12 12:08 Hongquan.zhang
// 
// 28    07-01-05 10:59 Hongquan.zhang
// 
// 27    06-12-28 16:45 Hongquan.zhang
// 
// 26    06-12-25 19:33 Hui.shao
// support embedded pathsvc mode
// 
// 25    06-12-25 16:58 Hui.shao
// fixed glog to envlog; _throw with envlog
// 
// 24    9/21/06 4:34p Hui.shao
// batch checkin on 20060921
// 
// 23    06-09-19 11:47 Hui.shao
// 18    06-08-10 12:46 Hui.shao
// moved bandwidth into the privateData
// 
// 14    06-07-26 15:22 Hui.shao
// modified the list link apis
// 9     06-06-12 15:56 Hui.shao
// added file header
// ===========================================================================

#include "PathSvcEnv.h"
#include "Log.h"
#include "WeiwooConfig.h"
#include "FileSystemOp.h"
#include "ServiceCommonDefintion.h"
#include "TianShanIceHelper.h"
#include "PathCommand.h"
#include <TsSRM.h>

extern "C"
{
#ifdef ZQ_OS_MSWIN
	#include <io.h>
#else
	#include <sys/stat.h>
#endif
};

//use the global session manager point to get the srm session
extern TianShanIce::SRM::SessionManagerPtr _gSessionManager;


namespace ZQTianShan {
namespace AccreditedPath {

#define TRY_BEGIN()	try {
#define TRY_END(_PROMPT)	}	catch(const Ice::Exception& ex) \
	{ std::ostringstream s; s << _PROMPT; s << ex; throw DatabaseException(s.str()); } \
	catch(...)	{ throw DatabaseException(_PROMPT " unkown exception"); }

// -----------------------------
// class PathSvcEnv
// -----------------------------
PathSvcEnv::PathSvcEnv(ZQ::common::Log& log, ZQ::common::NativeThreadPool& threadPool, 
					   Ice::CommunicatorPtr& communicator, const char* endpoint, 
					   const char* databasePath, const char* phoPath,
					   const char* configFile,const char* logFolder)
: _thpool(threadPool), _communicator(communicator), _adapter(NULL), _pathHelperMgr(*this), _log(log),
  _pStreamerDict(NULL), _pStorageDict(NULL), _pServiceGroupDict(NULL), _streamLinksByMaxTicket(0),
  _serviceState(TianShanIce::stNotProvisioned)
{
	_endpoint = (endpoint && strlen(endpoint)>0) ? endpoint : DEFAULT_ENDPOINT_PathManager;
	
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(PathSvcEnv, "open adapter %s at %s"), ADAPTER_NAME_PathManager, _endpoint.c_str());
	//_adapter = _communicator->createObjectAdapterWithEndpoints(ADAPTER_NAME_PathManager, endpoint);
	_adapter = ZQADAPTER_CREATE(_communicator, ADAPTER_NAME_PathManager, _endpoint.c_str(), _log);

	std::string path = FS::getImagePath();
	if(!path.empty()) {
		std::string::size_type pos = path.rfind(FNSEPC);
		if (pos != std::string::npos)
		{
			std::string tmp = path.substr(0, pos);
			pos = tmp.rfind(FNSEPC);
			if(pos != std::string::npos) 
			{
				tmp = tmp.substr(pos, 4);
				if(tmp == FNSEPS "bin" || tmp == FNSEPS "exe") 
				{
					path = path.substr(0, pos);		
				}
			}
		}
		_programRootPath = path;
	}
	else _programRootPath = ".";

	_programRootPath += FNSEPS;

	openDB((NULL == databasePath) ? (_programRootPath + "data" FNSEPS).c_str() : databasePath);

	_pathHelperMgr.setExtraData(configFile,logFolder);
#ifdef _DEBUG
	_pathHelperMgr.populate(_programRootPath.c_str());
	_pathHelperMgr.populate((_programRootPath+ "bin" FNSEPS).c_str());
	_pathHelperMgr.populate((_programRootPath+ "exe" FNSEPS).c_str());
#endif // _DEBUG

	_pathHelperMgr.populate((_programRootPath+ "modules" FNSEPS).c_str());

	// init the object factory for Weiwoo objects
	_factory = new PathFactory(*this);

	// mStreamerreplicaUpdater = new StreamerReplicaUpdater(*this);
	// mStreamerreplicaUpdater->start( gWeiwooServiceConfig.lEnableReplicaSubscriber, gWeiwooServiceConfig.lReplicaUpdateInterval );
	// _adapter->ZQADAPTER_ADD( _adapter->getCommunicator(), mStreamerreplicaUpdater, "ReplicaSubscriber");
	_aliveStreamerCol       = new AliveStreamerCollector(*this, gWeiwooServiceConfig.lEnableReplicaSubscriber?true:false, gWeiwooServiceConfig.lReplicaUpdateInterval);
	_aliveStreamerCol->start();
	_adapter->ZQADAPTER_ADD( _adapter->getCommunicator(), _aliveStreamerCol, "ReplicaSubscriber");

	_serviceState = TianShanIce::stProvisioned;
}

#ifdef EMBED_PATHSVC
PathSvcEnv::PathSvcEnv(::ZQTianShan::Weiwoo::WeiwooSvcEnv& masterEnv, 
					   Ice::CommunicatorPtr ic,
					   ZQ::common::Log& log, 
					   ZQ::common::NativeThreadPool& threadPool,
					   const char* phoPath,
					   const char* configFile,
					   const char* logFolder)
: _thpool(threadPool), _communicator(masterEnv._communicator), 
  _pathHelperMgr(*this), _log(log), //_adapter(masterEnv._adapter),
  _pStreamerDict(NULL), _pStorageDict(NULL), _pServiceGroupDict(NULL), _streamLinksByMaxTicket(0)
{
	
	_log(ZQ::common::Log::L_INFO, CLOGFMT(PathSvcEnv, "====== initial PathSvc at embedded mode"));
	if(ic)
	{
		_communicator = ic;
	}
	//Path must have his own adapter to avoid deadlock
	_endpoint = gWeiwooServiceConfig.szPathManagerEndpoint;
	if( _endpoint.length() == 0  )
	{
		_endpoint = DEFAULT_ENDPOINT_PathManager;
	}
//	_endpoint = masterEnv._endpoint;
	
	//_adapter = _communicator->createObjectAdapterWithEndpoints(ADAPTER_NAME_PathManager,_endpoint);
	_adapter = ZQADAPTER_CREATE(_communicator, ADAPTER_NAME_PathManager, _endpoint.c_str(), _log);


	

	_programRootPath = masterEnv._programRootPath;
	
	openDB(masterEnv._dbPath.c_str(),masterEnv._dbRuntimePath.c_str());

	_pathHelperMgr.setExtraData(configFile,logFolder);
#ifdef _DEBUG
//	_pathHelperMgr.populate(_programRootPath.c_str());
//	_pathHelperMgr.populate((_programRootPath+ "bin" FNSEPS).c_str());
//	_pathHelperMgr.populate((_programRootPath+ "exe" FNSEPS).c_str());
//	_pathHelperMgr.populate((_programRootPath+ "modules" FNSEPS).c_str());
#endif // _DEBUG
	_pathHelperMgr.populate(phoPath);
	//_pathHelperMgr.populate(phoPath);

	// init the object factory for Weiwoo objects
	_factory = new PathFactory(*this);

	masterEnv._proxyPathMgr = std::string(SERVICE_NAME_PathManager ":") + _endpoint;
	
	// mStreamerreplicaUpdater = new StreamerReplicaUpdater(*this);
	// mStreamerreplicaUpdater->start( gWeiwooServiceConfig.lEnableReplicaSubscriber, gWeiwooServiceConfig.lReplicaUpdateInterval );
	// _adapter->ZQADAPTER_ADD( _adapter->getCommunicator(), mStreamerreplicaUpdater, "ReplicaSubscriber");	
	_aliveStreamerCol = new AliveStreamerCollector(*this, gWeiwooServiceConfig.lEnableReplicaSubscriber?true:false, gWeiwooServiceConfig.lReplicaUpdateInterval);
	_aliveStreamerCol->start();
	_adapter->ZQADAPTER_ADD( _adapter->getCommunicator(), _aliveStreamerCol, "ReplicaSubscriber");

	_log(ZQ::common::Log::L_INFO, CLOGFMT(PathSvcEnv, "force SessionManager to use embedded PathManger at \"%s\""), masterEnv._proxyPathMgr.c_str());

	if (logFolder)
		_logFolder = logFolder;
}
#endif // EMBED_PATHSVC

PathSvcEnv::~PathSvcEnv()
{
	//ifinfo.ifid = _theAdapter->getCommunicator()->stringToIdentity(ifinfo.name);
	_adapter->remove( _adapter->getCommunicator()->stringToIdentity("ReplicaSubscriber") );
	_adapter=NULL;

	// mStreamerreplicaUpdater->stop( );
	// mStreamerreplicaUpdater = NULL;
	_aliveStreamerCol->stop( );
	_aliveStreamerCol = NULL;
	
	closeDB();
}

#define TranspotDBName		"Paths"
#define PathTicketDBName	"PathTickets"

//#define PathDataSubDir		"Paths"
//#define PathTicketSubDir	"PathTickets"
#define PATH_DBSZ           (160*1000) // 160MB
#define TICKET_DBSZ         (160*1000) // 160MB
ZQ::common::AtomicInt _gTotalTickets;

bool PathSvcEnv::openDB(const char* databasePath,const char* dbRuntimePath)
{
	closeDB();

	if (NULL != databasePath)
		_dbPath = databasePath;

	if (FNSEPC != _dbPath[_dbPath.length()-1])
		_dbPath += FNSEPS;

	if (NULL != dbRuntimePath ) 
	{
		_dbRuntimePath = dbRuntimePath;
	}
	else
	{
		_dbRuntimePath = _dbPath;
	}
	if (FNSEPC != _dbRuntimePath[_dbRuntimePath.length()-1])
		_dbRuntimePath += FNSEPS;

#ifdef EMBED_PATHSVC
	std::string transportDbPathname = createDBFolder(_log, "Weiwoo", _dbPath.c_str(), TranspotDBName, PATH_DBSZ);
	std::string ticketDbPathname = createDBFolder(_log, "Weiwoo", _dbRuntimePath.c_str(), PathTicketDBName, TICKET_DBSZ);
#else
	std::string transportDbPathname = createDBFolder(_log, "PathMgr", _dbPath.c_str(), TranspotDBName, PATH_DBSZ);
	std::string ticketDbPathname = createDBFolder(_log, "PathMgr", _dbRuntimePath.c_str(), PathTicketDBName, TICKET_DBSZ);
#endif// EMBED_PATHSVC

#if 1 // a piece of temporary code to recover a bug of V1.15.1 pirior to build 204. when all the sites are upgraded, this piece should be removed
	do
	{
		FILE* f1=NULL, *f2=NULL;
		std::string goodloc = _dbPath + "Weiwoo\\Paths\\" + "StreamLink";
		if ((f1 = fopen(goodloc.c_str(), "rb")) == NULL)
			break; // no need to copy if StreamLink even doesn't exist
		fclose(f1);

		goodloc = _dbPath + "Weiwoo\\Paths\\" + "Streamers";
		std::string badloc = _dbPath + "Paths\\" + "Streamers";

		if ((f1 = fopen(goodloc.c_str(), "rb")) == NULL && (f2 = fopen(badloc.c_str(), "rb")) !=NULL)
		{
			std::string cmd = std::string("copy \"") + _dbPath + "Paths\\" + "Streamers.*" +"\" \"" + _dbPath + "Weiwoo\\Paths\\\"";
			system(cmd.c_str());
			cmd = std::string("copy \"") + _dbPath + "Paths\\" + "Storages.*" +"\" \"" + _dbPath + "Weiwoo\\Paths\\\"";
			system(cmd.c_str());
			cmd = std::string("copy \"") + _dbPath + "Paths\\" + "ServiceGroups.*" +"\" \"" + _dbPath + "Weiwoo\\Paths\\\"";
			system(cmd.c_str());
		}

		if (f1!=NULL)
			fclose(f1);
		if (f2!=NULL)
			fclose(f2);
	} while(0);
#endif

	try {

		Ice::PropertiesPtr proper = _adapter->getCommunicator()->getProperties();
		proper->setProperty("Freeze.Evictor.UseNonmutating",      "1");

		std::string dbAttrPrefix = std::string("Freeze.DbEnv.") + transportDbPathname;
		proper->setProperty(dbAttrPrefix + ".DbRecoverFatal",      "1");
		proper->setProperty(dbAttrPrefix + ".DbPrivate",           "0");
		proper->setProperty(dbAttrPrefix + ".OldLogsAutoDelete",   "1");
		proper->setProperty("Freeze.Evictor." DBFILENAME_StorageLink ".PageSize",              "8192");
		proper->setProperty("Freeze.Evictor." DBFILENAME_StorageLink ".$default.BtreeMinKey",  "20");
		proper->setProperty("Freeze.Evictor." DBFILENAME_StreamLink ".PageSize",               "8192");
		proper->setProperty("Freeze.Evictor." DBFILENAME_StreamLink ".$default.BtreeMinKey",   "20");
		
		dbAttrPrefix = std::string("Freeze.DbEnv.") + ticketDbPathname;
		proper->setProperty(dbAttrPrefix + ".DbRecoverFatal",      "1");
		proper->setProperty(dbAttrPrefix + ".DbPrivate",           "0");
		proper->setProperty(dbAttrPrefix + ".OldLogsAutoDelete",   "1");
		proper->setProperty("Freeze.Evictor." DBFILENAME_PathTicket ".PageSize",              "8192");
		proper->setProperty("Freeze.Evictor." DBFILENAME_PathTicket ".$default.BtreeMinKey",  "20");

		if ( strlen( gWeiwooServiceConfig.szPathIceEnvCheckPointPeriod ) > 0 )
		{
			std::string strCheckPointPeriod = "Freeze.DbEnv." + transportDbPathname;
			strCheckPointPeriod = strCheckPointPeriod + ".CheckpointPeriod";			
			proper->setProperty( strCheckPointPeriod,  gWeiwooServiceConfig.szPathIceEnvCheckPointPeriod  );
	
			_log(ZQ::common::Log::L_INFO,CLOGFMT(PathSvcEnv,"prop[%s]: %s"),
				strCheckPointPeriod.c_str(),
				gWeiwooServiceConfig.szPathIceEnvCheckPointPeriod);

			strCheckPointPeriod = "Freeze.DbEnv." +ticketDbPathname;
			strCheckPointPeriod = strCheckPointPeriod + ".CheckpointPeriod";
			proper->setProperty( strCheckPointPeriod,  gWeiwooServiceConfig.szPathIceEnvCheckPointPeriod  );
			_log(ZQ::common::Log::L_INFO,CLOGFMT(PathSvcEnv,"prop[%s]: %s"),
				strCheckPointPeriod.c_str(),
				gWeiwooServiceConfig.szPathIceEnvCheckPointPeriod);

		}
		if ( strlen( gWeiwooServiceConfig.szPathIceEnvDbRecoverFatal ) ) 
		{
			std::string strDBRecoverFatal = "Freeze.DbEnv." + transportDbPathname;
			strDBRecoverFatal = strDBRecoverFatal + ".DbRecoverFatal";
			proper->setProperty( strDBRecoverFatal, gWeiwooServiceConfig.szPathIceEnvDbRecoverFatal );
			_log(ZQ::common::Log::L_INFO,CLOGFMT(PathSvcEnv,"prop[%s]: %s"),
				strDBRecoverFatal.c_str(),
				gWeiwooServiceConfig.szPathIceEnvDbRecoverFatal);

			strDBRecoverFatal = "Freeze.DbEnv." + ticketDbPathname;
			strDBRecoverFatal = strDBRecoverFatal + ".DbRecoverFatal";
			proper->setProperty( strDBRecoverFatal, gWeiwooServiceConfig.szPathIceEnvDbRecoverFatal );
			_log(ZQ::common::Log::L_INFO,CLOGFMT(PathSvcEnv, "prop[%s]: %s"),
				strDBRecoverFatal.c_str(),
				gWeiwooServiceConfig.szPathIceEnvDbRecoverFatal);
		}

		
		if ( strlen( gWeiwooServiceConfig.szFreezeStorageLinkSavePeriod) > 0 )
		{
			std::string strSessionSavePeriod = "Freeze.Evictor." + transportDbPathname;
			strSessionSavePeriod = strSessionSavePeriod + "." + DBFILENAME_StorageLink +".SavePeriod";
			proper->setProperty( strSessionSavePeriod, gWeiwooServiceConfig.szFreezeStorageLinkSavePeriod );
			_log(ZQ::common::Log::L_INFO,CLOGFMT(PathSvcEnv,"prop[%s]: %s"),
				strSessionSavePeriod.c_str(),
				gWeiwooServiceConfig.szFreezeStorageLinkSavePeriod);
		}
		if ( strlen(gWeiwooServiceConfig.szFreezeStorgaeLinkSaveSizeTrigger) > 0 ) 
		{
			std::string strSessSaveSizeTrigger = "Freeze.Evictor." + transportDbPathname ;
			strSessSaveSizeTrigger = strSessSaveSizeTrigger + "." + DBFILENAME_StorageLink + ".SaveSizeTrigger";
			proper->setProperty(strSessSaveSizeTrigger, gWeiwooServiceConfig.szFreezeStorgaeLinkSaveSizeTrigger);
			_log(ZQ::common::Log::L_INFO,CLOGFMT(PathSvcEnv,"prop[%s]: %s"),
				strSessSaveSizeTrigger.c_str(),
				gWeiwooServiceConfig.szFreezeStorgaeLinkSaveSizeTrigger);
		}

		if ( strlen( gWeiwooServiceConfig.szFreezeStreamLinkSavePeriod) > 0 )
		{
			std::string strSessionSavePeriod = "Freeze.Evictor." + transportDbPathname;
			strSessionSavePeriod = strSessionSavePeriod + "." + DBFILENAME_StreamLink +".SavePeriod";
			proper->setProperty( strSessionSavePeriod, gWeiwooServiceConfig.szFreezeStreamLinkSavePeriod );
			_log(ZQ::common::Log::L_INFO,CLOGFMT(PathSvcEnv,"prop[%s]: %s"),
				strSessionSavePeriod.c_str(),
				gWeiwooServiceConfig.szFreezeStreamLinkSavePeriod);
		}
		if ( strlen(gWeiwooServiceConfig.szFreezeStreamLinkSaveSizeTrigger) > 0 ) 
		{
			std::string strSessSaveSizeTrigger = "Freeze.Evictor." + transportDbPathname ;
			strSessSaveSizeTrigger = strSessSaveSizeTrigger + "." + DBFILENAME_StreamLink + ".SaveSizeTrigger";
			proper->setProperty(strSessSaveSizeTrigger, gWeiwooServiceConfig.szFreezeStreamLinkSaveSizeTrigger);
			_log(ZQ::common::Log::L_INFO,CLOGFMT(PathSvcEnv,"prop[%s]: %s"),
				strSessSaveSizeTrigger.c_str(),
				gWeiwooServiceConfig.szFreezeStreamLinkSaveSizeTrigger);
		}

		if ( strlen( gWeiwooServiceConfig.szFreezePathticketSavePeriod) > 0 )
		{
			std::string strSessionSavePeriod = "Freeze.Evictor." + transportDbPathname;
			strSessionSavePeriod = strSessionSavePeriod + "." + DBFILENAME_PathTicket +".SavePeriod";
			proper->setProperty( strSessionSavePeriod, gWeiwooServiceConfig.szFreezePathticketSavePeriod );
			_log(ZQ::common::Log::L_INFO,CLOGFMT(PathSvcEnv,"prop[%s]: %s"),
				strSessionSavePeriod.c_str(),
				gWeiwooServiceConfig.szFreezePathticketSavePeriod);
		}
		if ( strlen(gWeiwooServiceConfig.szFreezePathticketSaveSizeTrigger) > 0 ) 
		{
			std::string strSessSaveSizeTrigger = "Freeze.Evictor." + transportDbPathname ;
			strSessSaveSizeTrigger = strSessSaveSizeTrigger + "." + DBFILENAME_PathTicket + ".SaveSizeTrigger";
			proper->setProperty(strSessSaveSizeTrigger, gWeiwooServiceConfig.szFreezePathticketSaveSizeTrigger);
			_log(ZQ::common::Log::L_INFO,CLOGFMT(PathSvcEnv,"prop[%s]: %s"),
				strSessSaveSizeTrigger.c_str(),
				gWeiwooServiceConfig.szFreezePathticketSaveSizeTrigger);
		}

		_log(ZQ::common::Log::L_INFO, CLOGFMT(PathSvcEnv, "opening database at path: %s"), _dbPath.c_str());
		//::CreateDirectory((_dbPath + PathDataSubDir FNSEPS).c_str(), NULL);
		FS::createDirectory(transportDbPathname, true);

		_conn = Freeze::createConnection(_adapter->getCommunicator(), transportDbPathname);

		// open the dictionaries
		{
			ZQ::common::MutexGuard gd(_lockStreamerDict);
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(PathSvcEnv, "creating dictionary: %s"), DBFILENAME_StreamerDict);
			_pStreamerDict		= new TianShanIce::Transport::StreamerDict(_conn, DBFILENAME_StreamerDict);
		}

		{
			ZQ::common::MutexGuard gd(_lockStorageDict);
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(PathSvcEnv, "creating dictionary: %s"), DBFILENAME_StorageDict);
			_pStorageDict		= new TianShanIce::Transport::StorageDict(_conn, DBFILENAME_StorageDict);
		}

		{
			ZQ::common::MutexGuard gd(_lockServiceGroupDict);
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(PathSvcEnv, "creating dictionary: %s"), DBFILENAME_ServiceGroupDict);
			_pServiceGroupDict	= new TianShanIce::Transport::ServiceGroupDict(_conn, DBFILENAME_ServiceGroupDict);
		}
		
#define NEW_INDEX(_IDX) _log(ZQ::common::Log::L_DEBUG, CLOGFMT(PathSvcEnv, "creating index: " #_IDX)); \
			_idx##_IDX = new TianShanIce::Transport::_IDX(INDEXFILENAME(_IDX))
		
		// open the links and their indices
		NEW_INDEX(StorageToStorageLink);
		NEW_INDEX(StreamerToStorageLink);
		{
			std::vector<Freeze::IndexPtr> indices;
			indices.push_back(_idxStorageToStorageLink);
			indices.push_back(_idxStreamerToStorageLink);
			
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(PathSvcEnv, "creating evictor %s with indices"), DBFILENAME_StorageLink);
#if ICE_INT_VERSION / 100 >= 303
			_eStorageLink = ::Freeze::createBackgroundSaveEvictor(_adapter, transportDbPathname, DBFILENAME_StorageLink, 0, indices);
#else
			_eStorageLink = ::Freeze::createEvictor(_adapter, transportDbPathname, DBFILENAME_StorageLink, 0, indices);
#endif
			_adapter->addServantLocator(_eStorageLink, DBFILENAME_StorageLink);
		}

		NEW_INDEX(StreamerToStreamLink);
		NEW_INDEX(ServiceGroupToStreamLink);
		{
			std::vector<Freeze::IndexPtr> indices;
			indices.push_back(_idxStreamerToStreamLink);
			indices.push_back(_idxServiceGroupToStreamLink);
			
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(PathSvcEnv, "creating evictor %s with indices"), DBFILENAME_StreamLink);
#if ICE_INT_VERSION / 100 >= 303
 			_eStreamLink = ::Freeze::createBackgroundSaveEvictor(_adapter, transportDbPathname, DBFILENAME_StreamLink, 0, indices);
#else
 			_eStreamLink = ::Freeze::createEvictor(_adapter, transportDbPathname, DBFILENAME_StreamLink, 0, indices);
#endif
			_adapter->addServantLocator(_eStreamLink, DBFILENAME_StreamLink);
		}

		// open the tickets and its indices
		_log(ZQ::common::Log::L_INFO, CLOGFMT(PathSvcEnv, "opening runtime database at path: %s"), _dbRuntimePath.c_str());
		//::CreateDirectory((_dbRuntimePath + PathTicketSubDir FNSEPS).c_str(), NULL);
		FS::createDirectory(ticketDbPathname, true);
		
		NEW_INDEX(StreamLinkToTicket);
		NEW_INDEX(StorageLinkToTicket);
		{
			std::vector<Freeze::IndexPtr> indices;
			indices.push_back(_idxStreamLinkToTicket);
			indices.push_back(_idxStorageLinkToTicket);

			BerkeleyDBConfig conf;
			if(!conf.generateConfig(ticketDbPathname))
			{
				_log(ZQ::common::Log::L_WARNING, CLOGFMT(PathSvcEnv, "failed to generate DB_CONFIG for %s"), DBFILENAME_PathTicket);
			}
			
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(PathSvcEnv, "creating evictor %s with indices"), DBFILENAME_PathTicket);
#if ICE_INT_VERSION / 100 >= 303
			_ePathTicket = ::Freeze::createBackgroundSaveEvictor(_adapter, ticketDbPathname, DBFILENAME_PathTicket, 0, indices);
#else
			_ePathTicket = ::Freeze::createEvictor(_adapter, ticketDbPathname, DBFILENAME_PathTicket, 0, indices);
#endif
			_adapter->addServantLocator(_ePathTicket, DBFILENAME_PathTicket);
		}

		_eStorageLink->setSize(gWeiwooServiceConfig.lEvictorStorlinkSize>100?
														gWeiwooServiceConfig.lEvictorStorlinkSize:100);
		_eStreamLink->setSize(gWeiwooServiceConfig.lEvictorStrmlinkSize>100?
														gWeiwooServiceConfig.lEvictorStrmlinkSize:100);
		_ePathTicket->setSize(gWeiwooServiceConfig.lEvictorPathTicketSize>100?
														gWeiwooServiceConfig.lEvictorPathTicketSize:100);

		IdentCollection identities;
		::Freeze::EvictorIteratorPtr itptr = _ePathTicket->getIterator("", 100);
		while (itptr && itptr->hasNext())
		{
			identities.push_back(itptr->next());
			// Note: the procedure of restore and orphan cleaning was covered in:
			//          Service::OnInit() => PathSvcEnv::doValidation()
		}

		_gTotalTickets.set(identities.size());
	}
	catch(const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow <TianShanIce::ServerError> (_log,"PathSvcEnv",1001, "PathSvcEnv::openDB() caught exception: %s", ex.ice_name().c_str());
	}
	catch(...)
	{
		ZQTianShan::_IceThrow <TianShanIce::ServerError> (_log,"PathSvcEnv",1002,"PathSvcEnv::openDB() unkown exception");
	}

	_log(ZQ::common::Log::L_INFO, CLOGFMT(PathSvcEnv, "database ready, initial ticket count: %d"), _gTotalTickets.get());
	return true;
}

void PathSvcEnv::closeDB(void)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(PathSvcEnv, "close local database"));

	{
		ZQ::common::MutexGuard gd(_lockStreamerDict);
		if (NULL != _pStreamerDict)
			delete _pStreamerDict;
		_pStreamerDict = NULL;
	}

	{
		ZQ::common::MutexGuard gd(_lockStorageDict);
		if (NULL != _pStorageDict)
			delete _pStorageDict;
		_pStorageDict = NULL;
	}

	{
		ZQ::common::MutexGuard gd(_lockServiceGroupDict);
		if (NULL != _pServiceGroupDict)
			delete _pServiceGroupDict;
		_pServiceGroupDict = NULL;
	}
}

#define VALIDATE_LINK(_LINKTYPE) \
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(PathSvcEnv, "validate the configuration for each StorageLink")); \
	{ IdentCollection identities; \
	try	{ \
		ZQ::common::MutexGuard gd(_lock##_LINKTYPE); \
		::Freeze::EvictorIteratorPtr itptr = _e##_LINKTYPE->getIterator("", 1); \
		while (itptr->hasNext()) identities.push_back(itptr->next()); \
} catch(...) { ZQTianShan::_IceThrow<TianShanIce::InvalidStateOfArt> (_log,#_LINKTYPE,1011, "failed to list " #_LINKTYPE);	} \
	for (IdentCollection::iterator it = identities.begin(); it !=identities.end(); it ++) \
	{ try { \
		::TianShanIce::Transport::_LINKTYPE##ExPrx link = IdentityToObjEnv(*this, _LINKTYPE##Ex, *it); \
		if (!link) continue; \
		::TianShanIce::ValueMap pd = link->getPrivateData(); \
		link->updatePrivateData(pd); \
		} catch (Ice::Exception& ex)	{ _log(ZQ::common::Log::L_WARNING, CLOGFMT(PathSvcEnv, "exception caught when validate " #_LINKTYPE ": ex[%s]"), ex.ice_name().c_str()); } \
	} }


#include <TsSRM.h>

void PathSvcEnv::doValidation()
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(PathSvcEnv, "validating StorageLinks"));
	VALIDATE_LINK(StorageLink);

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(PathSvcEnv, "validating StreamLinks"));
	VALIDATE_LINK(StreamLink);

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(PathSvcEnv, "validating PathTickets"));
	int cTickets, cOrphans=0, cDestroyed=0, cDamaged=0;

	{
		//destroy ticket if it is an orphan
		::Freeze::EvictorIteratorPtr itptr = _ePathTicket->getIterator("", 1024); 
		IdentCollection identities;
		while (itptr->hasNext()) 
			identities.push_back(itptr->next());

		cTickets = identities.size();

		for (IdentCollection::const_iterator it = identities.begin(); it != identities.end() ; it ++ )
		{
			TianShanIce::Transport::PathTicketPrx ticket =NULL;
			Ice::Identity sessIdent;

			try
			{
				ticket = TianShanIce::Transport::PathTicketPrx::checkedCast( _adapter->createProxy(*it));
				if (ticket)
				{
					TianShanIce::ValueMap prvData =  ticket->getPrivateData();
					ZQTianShan::Util::getValueMapDataWithDefault(prvData, WEIWOO_SESS_IDENT_NAME, "", sessIdent.name);
					ZQTianShan::Util::getValueMapDataWithDefault(prvData, WEIWOO_SESS_IDENT_CATE, "", sessIdent.category);
				}
			}
			catch( const Ice::Exception& ex)
			{
				_log(ZQ::common::Log::L_ERROR,CLOGFMT(PathSvcEnv,"doValidation() read ticket[%s] caught exception[%s]"),
					it->name.c_str(), ex.ice_name().c_str());
			}
			catch(...)
			{
				_log(ZQ::common::Log::L_ERROR,CLOGFMT(PathSvcEnv,"doValidation() read ticket[%s] caught exception"),
					it->name.c_str());
			}

			bool bNeedDestroy = false;

			if (!ticket || sessIdent.name.empty())
			{
				cDamaged ++;
				bNeedDestroy = true;
			}
			else
			{
				TianShanIce::SRM::SessionPrx sess = NULL;

				try
				{
					Ice::Current dummyC;
					sess = _gSessionManager->openSession(sessIdent.name, dummyC);
					if (sess)
						sess->ice_ping();
				}
				catch(const Ice::Exception& ex)
				{
					_log(ZQ::common::Log::L_WARNING, CLOGFMT(PathSvcEnv, "ticket[%s] ping sess[%s] caught exception[%s]"), it->name.c_str(), sessIdent.name.c_str(), ex.ice_name().c_str());
					sess = NULL;
				}
				catch(...)
				{
					_log(ZQ::common::Log::L_WARNING, CLOGFMT(PathSvcEnv, "ticket[%s] ping sess[%s] caught exception"), it->name.c_str(), sessIdent.name.c_str());
					sess = NULL;
				}

				if (NULL != sess)
					_log(ZQ::common::Log::L_DEBUG, CLOGFMT(PathSvcEnv, "ticket[%s] is associated to sess[%s]"), it->name.c_str(), sessIdent.name.c_str());
				else
				{
					cOrphans++;
					bNeedDestroy = true;
					_log(ZQ::common::Log::L_DEBUG, CLOGFMT(PathSvcEnv, "ticket[%s] is an orphan, sess[%s] not found"), it->name.c_str(), sessIdent.name.c_str());
				}
			}

			if (bNeedDestroy)
			{
				try
				{
					if (ticket)
						ticket->destroy();
					else
						_ePathTicket->remove(*it);

					cDestroyed++;
					_log(ZQ::common::Log::L_WARNING, CLOGFMT(PathSvcEnv, "doValidation() cleaned ticket[%s] used belong to sess[%s]"), it->name.c_str(), sessIdent.name.c_str());
				}
				catch(const Ice::Exception& ex)
				{
					_log(ZQ::common::Log::L_WARNING, CLOGFMT(PathSvcEnv, "doValidation() clean ticket[%s] of sess[%s] caught exception[%s]"), it->name.c_str(), sessIdent.name.c_str(), ex.ice_name().c_str());
				}
				catch(...)
				{
					_log(ZQ::common::Log::L_WARNING, CLOGFMT(PathSvcEnv, "doValidation() clean ticket[%s] of sess[%s] caught exception"), it->name.c_str(), sessIdent.name.c_str());
				}
			}
			else
			{
				try
				{
					if (!ticket)
						continue;

					Ice::Int svcGrp = ticket->getStreamLink()->getServiceGroupId();
					::TianShanIce::SRM::ResourceMap tres = ticket->getResources();
					Ice::Long bandwidth =0;
					ZQTianShan::Util::getResourceDataWithDefault(tres, TianShanIce::SRM::rtTsDownstreamBandwidth, "bandwidth", 0, bandwidth);

					if (svcGrp >0 && TianShanIce::stInService == ticket->getState())
					{
						commitUsage(svcGrp, bandwidth);
						_log(ZQ::common::Log::L_DEBUG, CLOGFMT(PathSvcEnv, "doValidation() restored ticket[%s]-sess[%s] usage: serviceGroup[%d] [%lld]bps"), it->name.c_str(), sessIdent.name.c_str(), svcGrp, bandwidth);
					}
				}
				catch(const Ice::Exception& ex)
				{
					_log(ZQ::common::Log::L_WARNING, CLOGFMT(PathSvcEnv, "doValidation() restore ticket[%s]-sess[%s] usage caught exception[%s]"), it->name.c_str(), sessIdent.name.c_str(), ex.ice_name().c_str());
				}
				catch(...)
				{
					_log(ZQ::common::Log::L_WARNING, CLOGFMT(PathSvcEnv, "doValidation() restore ticket[%s]-sess[%s] usage caught exception"), it->name.c_str(), sessIdent.name.c_str());
				}
			} // if !bNeedDestroy
		} // for-loop
	}

	_log(ZQ::common::Log::L_INFO, CLOGFMT(PathSvcEnv, "doValidation() completed: tickets[%d] orphan[%d] damaged[%d] destroyed[%d]"), cTickets, cOrphans, cDamaged, cDestroyed);
}

#define bpsToKbps(_bps)  ((int)((_bps + 999) /1000))
#define NEWLINE "\n"

bool PathSvcEnv::commitUsage(Ice::Int serviceGroupId, Ice::Int bpsAllocated)
{
	ZQ::common::MutexGuard g(_lkSvcGrpStat);
	ServiceGrpStatMap::iterator itStat = _svcGrpStatMap.find(serviceGroupId);
	if (_svcGrpStatMap.end() == itStat)
	{
		// insert a new record
		ServiceGrpStat sgstat;
		sgstat.serviceGroupId   =serviceGroupId;
		sgstat.cSessions        =1;
		sgstat.kbpsCommitted    =bpsToKbps(bpsAllocated);
		sgstat.kbpsAssigned     =bpsToKbps(bpsAllocated);
		sgstat.stampAsOfAssigned=0;

		MAPSET(ServiceGrpStatMap, _svcGrpStatMap, serviceGroupId, sgstat);
		return true;
	}

	itStat->second.cSessions++;
	itStat->second.kbpsCommitted    +=bpsToKbps(bpsAllocated);

	if (itStat->second.kbpsAssigned < itStat->second.kbpsCommitted)
	{
		// adjust but mark as dirty
		itStat->second.kbpsAssigned = itStat->second.kbpsCommitted;
		itStat->second.stampAsOfAssigned=0; 
	}

	return true;
}

bool PathSvcEnv::withdrawUsage(Ice::Int serviceGroupId, Ice::Int bpsAllocated)
{
	ZQ::common::MutexGuard g(_lkSvcGrpStat);
	ServiceGrpStatMap::iterator itStat = _svcGrpStatMap.find(serviceGroupId);
	if (_svcGrpStatMap.end() == itStat)
		return false;

	if (--itStat->second.cSessions <0)
		itStat->second.cSessions =0;
	itStat->second.kbpsCommitted   -=bpsToKbps(bpsAllocated);
	if (itStat->second.kbpsCommitted <0)
		itStat->second.kbpsCommitted =0;

	return true;
}

void PathSvcEnv::sumAssigned(Ice::Int serviceGroupId)
{
#pragma message(__MSGLOC__"TODO: scan the streamlink to collect and sum the total bandwidth of the serviceGroup")
}

void PathSvcEnv::setDirtySourceGroup(Ice::Int serviceGroupId)
{
	ZQ::common::MutexGuard g(_lkSvcGrpStat);
	ServiceGrpStatMap::iterator itStat = _svcGrpStatMap.find(serviceGroupId);
	if (_svcGrpStatMap.end() == itStat)
		return;
	itStat->second.stampAsOfAssigned =0;
}

#define CSV_FILENAME "sgstat_"
bool PathSvcEnv::dumpUsageStat()
{
	char stamp[80], *p;
	ZQ::common::TimeUtil::TimeToUTC(ZQ::common::now(), stamp, sizeof(stamp)-2, true);
	if (NULL != (p = strchr(stamp, '.')))
		*p = '\0';

	std::string filename = stamp+2; // skip the centry
	// compress the stamp string
	for (size_t pos=0; std::string::npos != (pos = filename.find_first_of("T-:+.")); )
		filename.erase(pos, 1);

	// delete old csv files
	::TianShanIce::StrValues oldfiles = FS::searchFiles(_logFolder, CSV_FILENAME "*.csv");
	if (oldfiles.size() >10)
	{
		std::sort(oldfiles.begin(), oldfiles.end());
		for (int i= ((int) oldfiles.size()) -1 -10; i >=0; i--) // maximally keep 10 copies
			::unlink(oldfiles[i].c_str());
	}

	filename = _logFolder + CSV_FILENAME + filename +".csv";
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(PathSvc, "dumpUsageStat() exporting usage stat of servicegroups to %s"), filename.c_str());

	std::string tempfile = filename + "~";
	::std::ofstream file(tempfile.c_str());
	if (!file.is_open())
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(PathSvc, "dumpUsageStat() failed to open file: %s"), tempfile.c_str());
		return false;
	}

	::TianShanIce::IValues dirtySgs;
	uint32 cServiceGroups =0;

	{
		file << "time, serviceGroup, sessions, committedKbps, assignedKbps" << NEWLINE;
		ZQ::common::MutexGuard g(_lkSvcGrpStat);
		for (ServiceGrpStatMap::iterator itStat = _svcGrpStatMap.begin(); itStat != _svcGrpStatMap.end(); itStat++)
		{
			file << stamp << "," << itStat->second.serviceGroupId << "," << itStat->second.cSessions
				 << "," << itStat->second.kbpsCommitted << "," << itStat->second.kbpsAssigned << NEWLINE;

			cServiceGroups++;

			if (itStat->second.stampAsOfAssigned <=0)
				dirtySgs.push_back(itStat->second.serviceGroupId);
		}
	}

	file.flush();
	file.close();

	::unlink(filename.c_str());
	::rename(tempfile.c_str(), filename.c_str());
	::unlink(tempfile.c_str());

	try {
		(new SumServiceGroupBwCommand(*this, dirtySgs))->execute();
	}
	catch(...) {}

	_log(ZQ::common::Log::L_INFO, CLOGFMT(PathSvc, "dumpUsageStat() completed, %d dirties found, %d servicegroups has been exported to %s"), dirtySgs.size(), cServiceGroups, filename.c_str());
	return true;
}

}} // namespace
