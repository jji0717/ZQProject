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
// Ident : $Id: CPEImpl.cpp $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/ContentStoreImpl.cpp $
// 
// 38    5/07/15 4:16p Li.huang
// add log
// 
// 37    12/23/14 1:34p Hui.shao
// 
// 37    12/23/14 1:31p Hui.shao
// more log
// 
// 36    2/17/14 10:05a Zonghuan.xiao
// add PROCESSED_BY_TIME for lzz
// 
// 35    12/23/13 4:04p Zonghuan.xiao
// hash folder name refector
// 
// 34    12/12/13 1:43p Hui.shao
// %lld
// 
// 33    12/12/13 9:48a Hui.shao
// ContentStore::openContent() to dispatch if configure _hashFolderExpsn
// is defined
// 
// 32    12/11/13 7:45p Hui.shao
// 
// 31    12/11/13 7:43p Hui.shao
// added hashed folder calculation
// 
// 30    3/08/13 2:44p Hui.shao
// corrected KB at cache size
// 
// 29    3/06/13 4:47p Hui.shao
// export the indexing from mountpath to volname thru ICE
// 
// 28    3/06/13 4:33p Hui.shao
// 
// 27    12/19/12 5:29p Hui.shao
// 
// 26    9/24/12 6:24p Hui.shao
// sting content to populate attributes by provision event
// 
// 25    9/06/12 5:49p Hui.shao
// 
// 24    9/06/12 5:05p Hui.shao
// 
// 23    9/06/12 5:04p Hui.shao
// 
// 22    8/09/12 5:15p Hui.shao
// NULL pointer test for linux
// 
// 21    8/06/12 5:40p Hui.shao
// 
// 20    6/07/12 11:30a Hui.shao
// configuration schema
// 
// 19    6/06/12 5:55p Hui.shao
// 
// 18    6/06/12 4:42p Hui.shao
// 
// 17    3/26/12 4:01p Hui.shao
// added openFolderEx() to create subfolder recursively
// 
// 16    3/22/12 5:20p Li.huang
// modify opencontentbyfullname function
// 
// 15    3/22/12 5:13p Hui.shao
// 
// 14    1/18/12 6:22p Li.huang
// 
// 13    11/30/11 9:28p Hui.shao
// merged playtime interval limitation from v1.15
// 
// 14    11/29/11 4:36p Hui.shao
// ticket#9981, JIRA ACE-9345, do not call Vstrm
// VstrmClassLoadFullAssetInfoEx() too frequently
// 
// 13    8/03/11 12:13p Hongquan.zhang
// 
// 11    5/26/11 5:51p Hongquan.zhang
// 
// 10    5/16/11 4:02p Hongquan.zhang
// I am not sure but have a try
// 
// 9     3/23/11 12:57p Hui.shao
// 
// 8     2/12/11 4:20p Jie.zhang
// 
// 7     2/09/11 4:04p Hui.shao
// added a flag STOREFLAG_checkFSonOpenContentByFullName to borrow
// Folder::openContent() to populate content if exist on FS
// 
// 6     10-12-08 12:23 Hui.shao
// enable timer() persistently for volume, and added volume::setMetaData()
// 
// 5     10-11-25 18:45 Hui.shao
// added free-space monitoring configuration vars
// 
// 4     10-11-18 17:11 Hongquan.zhang
// 
// 3     10-11-17 18:32 Hui.shao
// resort the node queue
// 
// 2     10-11-17 18:13 Hui.shao
// fs-sync to yield the beginning of service start; resort the node
// replica queue
// 
// 1     10-11-12 16:04 Admin
// Created.
// 
// 1     10-11-12 15:37 Admin
// Created.
// 
// 71    10-10-28 18:12 Hongquan.zhang
// allow empty path 
// 
// 70    10-10-27 15:26 Hui.shao
// double checked the changes of V1.10 since 3/19/2009
// 
// 69    10-04-26 15:38 Hui.shao
// added minKey tuning
// 
// 68    10-04-23 11:58 Hongquan.zhang
// add edge mode
// 
// 67    10-04-14 17:00 Hui.shao
// summarize the volume usage periodically
// 
// 66    10-04-08 19:30 Xia.chen
// 
// 65    10-03-03 15:56 Xia.chen
// 
// 64    10-02-05 15:59 Yixin.tian
// add init volume subfolder
// 
// 63    10-01-26 14:46 Yixin.tian
// add ContentStoreImpl interface listSubFolders()
// 
// 62    10-01-07 15:45 Hui.shao
// set DB cache size to 16MB
//
// 61    09-12-25 15:08 Fei.huang
// + add FileSystemOp
// 
// 60    09-12-22 18:25 Hui.shao
// initialize DB_CONFIG.dat if it is not available
// 
// 59    09-12-22 17:46 Hui.shao
// abstracted folder from volume
// 58    09-12-22 17:27 Fei.huang
// * surpress initialize warnings
// 
// 57    09-12-22 14:06 Jie.zhang
// merge from TianShan1.10
// 
// 56    09-09-11 11:17 Hui.shao
// made the auto sync with filesystem optional
// 
// 55    09-09-03 13:24 Hui.shao
// separate the handling on interested file events
// 
// 54    09-09-02 15:14 Hui.shao
// 
// 53    09-09-02 14:42 Hui.shao
// added flag to mask file events to handle
// 
// 52    09-08-21 16:17 Jie.zhang
// 
// 51    09-08-06 17:59 Jie.zhang
// 
// 50    09-07-31 20:16 Fei.huang
// * fix log fmt
// 
// 49    09-07-31 19:56 Fei.huang
// * exportStoreReplica: state not initialized cause failure of
// updateReplica with unmarshal error
// 
// 48    09-07-31 17:08 Jie.zhang
// 
// 47    09-07-29 13:49 Jie.zhang
// merge from TianShan1.10
// 
// 46    09-06-26 16:48 Fei.huang
// 
// 45    09-06-11 19:05 Hui.shao
// 
// 44    09-06-11 18:56 Hui.shao
// merged back _checkResidentialInFileDeleteEvent from 1.10
// 
// 43    09-06-11 15:09 Hui.shao
// exported some timeout as configurable, enlarged the interval of
// populating if know populating fail
// 
// 42    09-06-05 11:04 Jie.zhang
// 
// 41    09-05-08 16:00 Jie.zhang
// add SaveSizeTrigger & SavePeriod to contentstore property
// 
// 40    09-03-09 15:22 Hui.shao
// 
// 39    09-03-06 17:22 Hui.shao
// 
// 38    09-02-20 17:43 Hongquan.zhang
// 
// 37    09-02-05 13:53 Hui.shao
// added the event entires from the ContentStoreLib to the impls derived
// from the base ContentStoreImpl
// 
// 36    09-01-20 16:47 Yixin.tian
// 
// 35    09-01-20 16:11 Hongquan.zhang
// 
// 34    09-01-14 15:10 Hongquan.zhang
// 
// 33    08-12-25 23:27 Jie.zhang
// ProvisionEvent set system metadata
// 
// 32    08-12-25 21:20 Jie.zhang
// 
// 31    08-12-24 18:23 Jie.zhang
// setUserMetadata when ProvisionEvent
// 
// 30    08-12-24 11:23 Hui.shao
// 
// 29    08-12-23 15:20 Hui.shao
// 
// 28    08-12-23 15:20 Hui.shao
// added importDB()
// 
// 27    08-12-17 21:23 Jie.zhang
// 
// 26    08-12-17 17:42 Hui.shao
// 
// 25    08-12-17 17:40 Hui.shao
// 
// 24    08-12-17 16:48 Hui.shao
// quit the program more quickly from sync procedure
// 
// 23    08-12-16 11:57 Hui.shao
// 
// 22    08-12-16 11:26 Hui.shao
// restrict names of volume to mount
// 
// 21    08-12-15 19:27 Hui.shao
// moved the sync fs to a separate thread request
// 
// 20    08-12-11 12:32 Hui.shao
// 
// 19    08-11-27 16:12 Hui.shao
// more freeze options
// 
// 18    08-11-27 12:43 Hui.shao
// use YiXin's native C api, and force to recover DB when open
// 
// 17    08-11-26 14:57 Yixin.tian
// modify dumpDB
//
// 16    08-11-24 13:56 Jie.zhang
// add evictor size configuration
// 
// 15    08-11-24 12:44 Hui.shao
// dump db to external file
// 
// 14    08-11-24 12:29 Jie.zhang
// add a parameter on checkResidencialStatus
// 
// 13    08-11-18 18:30 Yixin.tian
// 
// 12    08-11-18 18:22 Yixin.tian
// modify can compile for Linux OS
// 
// 11    08-11-18 13:59 Hui.shao
// added cacheLevel and attached StreamServices
// 
// 10    08-11-15 15:20 Hui.shao
// 
// 9     08-11-15 14:02 Hui.shao
// 
// 8     08-11-13 11:57 Jie.zhang
// add getStoreReplicas()
// 
// 7     08-11-10 15:06 Jie.zhang
// 
// 6     08-11-10 11:50 Hui.shao
// added OnIdle() entry, filter the expired replica
//
// 5     08-11-10 11:38 Jie.zhang
// some enhancement in map access
// 
// 4     08-11-07 11:00 Jie.zhang
// add common log define to unify log using style
// 
// 3     08-11-06 13:23 Hui.shao
// rolled back Fei's checkin, added ProvisionEvent handling
// 
// 1     08-11-03 11:26 Hui.shao
// splitted from ContentImpl.cpp
// 
// 11    08-10-30 16:17 Hui.shao
// forward ContentStoreImpl::listContents_async
// ===========================================================================
#include "ContentImpl.h"
#include "ContentState.h"
#include "ContentCmds.h"
#include "Guid.h"
#include "Log.h"

#include "expatxx.h"
#include "FileSystemOp.h"

#include <iostream>
#include <fstream>
#include <stack>

// #define ENABLE_COMPRESS

#ifdef ENABLE_COMPRESS
#  include "Bz2Stream.h"
#endif // ENABLE_COMPRESS

extern "C"
{
#ifdef ZQ_OS_MSWIN
	#include <io.h>
#else
	#include <sys/stat.h>
#endif
};

#define MOLOG	(_log)
#define MOEVENT	(_eventlog)

namespace ZQTianShan {
namespace ContentStore {

#define INDEXFILENAME(_IDX)			#_IDX "Idx"

// -----------------------------
// module ContentStoreImpl
// -----------------------------
ContentStoreImpl::ContentStoreImpl(ZQ::common::Log& log, ZQ::common::Log& eventlog,
								   ZQ::common::NativeThreadPool& threadPool,
								   ZQADAPTER_DECLTYPE& adapter,
								   const char* databasePath)
: _log(log), _eventlog(eventlog), _adapter(adapter), _thpool(threadPool),  
_watchDog(log, threadPool, adapter, "ContentStore"), _serviceState(::TianShanIce::stNotProvisioned),
_cFileEvent(0), _streamableLength(0), _fileEventFlags(~(uint32)0), _autoFileSystemSync(false), _replicaTimeout(0), 
_cacheLevel(50), _exposeStreamService(false), _cacheable(false), _ctxPortal(NULL), _storeFlags(0), _volumeEvictorSize(20),
_edgeMode(false), _stampStarted(0), _warningFreeSpacePercent(0), _stepFreeSpacePercent(0), _timeoutOfPlaytimeAtProvisioning(0)
{
	_programRootPath = ZQTianShan::getProgramRoot();
	_programRootPath += FNSEPS;
	if(databasePath && databasePath[0] != 0 )
	{
		_dbPath = databasePath;
	}
	else
	{
		_dbPath = _programRootPath+"data"+ FNSEPS;
	}

	_contentSavePeriod = 6000;
	_contentSaveSizeTrigger = 300;
	_checkResidentialInFileDeleteEvent = false;

	_timeoutNotProvisioned = MAX_NOT_PROVISIONED_TIMEOUT;
	_timeoutIdleProvisioning = MAX_IDLE_PROVISIONING_TIMEOUT;
	
	_storeAggregate = 0;
	_enableInServiceCheck = 0;
}

bool ContentStoreImpl::initializeContentStore()
{
	try
	{
		initializePortal(*this);

		openDB(_dbPath.c_str());

		// init the object factory
		_factory = new ContentFactory(*this);

		// start the watch dog
		_watchDog.start();

		enableInterface();
		_stampStarted =now();
	}
	catch(...)
	{
		_log(ZQ::common::Log::L_ERROR,CLOGFMT(ContentStore,"failed to initialize cotentStore"));
		return false;
	}
	return true;
}

void ContentStoreImpl::enableInterface(bool enable)
{
	if (enable)
	{
		try {
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "enableInterface(true) adding the interface \"%s\" on to the adapter \"%s\""), SERVICE_NAME_ContentStore, _adapter->getName().c_str());
			_localId = _adapter->getCommunicator()->stringToIdentity(SERVICE_NAME_ContentStore);
			_adapter->ZQADAPTER_ADD(_adapter->getCommunicator(), this, SERVICE_NAME_ContentStore);
		}
		catch(...) {}
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "add self under watching"));
		_watchDog.watch(_localId, 0);
		_serviceState = ::TianShanIce::stInService;
	}
	else if (!_localId.name.empty())
	{
		_serviceState = ::TianShanIce::stOutOfService;
		try {
			_log(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "enableInterface(false) removing the interface \"%s\" from the adapter \"%s\""), SERVICE_NAME_ContentStore, _adapter->getName().c_str());
			_adapter->remove(_localId);
			_localId.name = "";
		}
		catch(...) {}
	}
}
void ContentStoreImpl::unInitializeContentStore( )
{
	enableInterface(false);
	_serviceState = ::TianShanIce::stOutOfService;

	_watchDog.quit();

	uninitializePortal(*this);

	closeDB();
	_factory = NULL;
}
ContentStoreImpl::~ContentStoreImpl()
{	

}

static int toLOGIC_FNSEPC(int value)
{
	if ('\\' == value)
		return LOGIC_FNSEPC;
	return value;
}

::Ice::Identity ContentStoreImpl::toContentIdent(const std::string& contentName)
{
	::Ice::Identity ident;
	ident.category = DBFILENAME_Content;
	ident.name = contentName;

	std::transform(ident.name.begin(), ident.name.end(), ident.name.begin(), (int(*)(int)) toLOGIC_FNSEPC);

	return ident;
}

int ContentStoreImpl::chopPathname(const std::string& fullname, std::string& volumeName, std::string& folderName, std::string& contentName)
{
	size_t posFolder = fullname.find(LOGIC_FNSEPC, 1);
	if (std::string::npos == posFolder)
		return 0;

	volumeName = fullname.substr(0, posFolder);
	size_t posContent = fullname.find_last_of(LOGIC_FNSEPS);
	
	if (posContent < posFolder)
		return 1;

	folderName = fullname.substr(posFolder +1, posContent - posFolder);
	contentName = fullname.substr(posContent +1);

	while (!folderName.empty() && LOGIC_FNSEPC == folderName[folderName.length()-1])
		folderName = folderName.substr(0, folderName.length()-1);

	return 3;
}

::Ice::Identity ContentStoreImpl::toVolumeIdent(const std::string& volumeName)
{
	::Ice::Identity ident;
	ident.category = DBFILENAME_Volume;
	ident.name = volumeName;

	std::transform(ident.name.begin(), ident.name.end(), ident.name.begin(), (int(*)(int)) toLOGIC_FNSEPC);

	return ident;
}

::TianShanIce::Storage::ContentStorePrx ContentStoreImpl::proxy(bool collocationOptim)
{
	try {
		if (!_thisPrx)
		{//_thisPrx is only modified here
			static ZQ::common::Mutex thisProxyLocker;
			{
				ZQ::common::MutexGuard gd(thisProxyLocker);
				if(!_thisPrx)
					_thisPrx = ::TianShanIce::Storage::ContentStorePrx::checkedCast(_adapter->createProxy(_localId));
			}
		}

		if (collocationOptim)
			return _thisPrx;
		else
			return ::TianShanIce::Storage::ContentStorePrx::checkedCast(_thisPrx->ice_collocationOptimized(false));
	}
	catch(const Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "proxy() caught exception[%s]"), ex.ice_name().c_str());
	}

	return NULL;
}

void ContentStoreImpl::openDB(const char* databasePath)
{
	closeDB();

	if (NULL == databasePath || strlen(databasePath) <1)
		_dbPath = _programRootPath + "data" FNSEPS ;
	else 
		_dbPath = databasePath;
	
	if (FNSEPC != _dbPath[_dbPath.length()-1])
		_dbPath += FNSEPS;
	
	try 
	{	
		// open the Indexes
		MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ContentStore, "opening database at path: %s"), _dbPath.c_str());
		createDBFolderEx(MOLOG, (_dbPath + "Contents" +FNSEPS), ContentDB_CACHESIZE_KB);

		Ice::PropertiesPtr proper = _adapter->getCommunicator()->getProperties();
		proper->setProperty("Freeze.Evictor.UseNonmutating",      "1");

		std::string dbAttrPrefix = std::string("Freeze.DbEnv.") + _dbPath + "Contents" FNSEPS;
		proper->setProperty(dbAttrPrefix + ".DbRecoverFatal",      "1");
		proper->setProperty(dbAttrPrefix + ".DbPrivate",           "0");
		proper->setProperty(dbAttrPrefix + ".OldLogsAutoDelete",   "1");

		std::string dbContentAttrPrefix = dbAttrPrefix + ".Content";
		char szTmp[64];

		sprintf(szTmp, "%d", _contentSavePeriod);
		std::string strKey = dbContentAttrPrefix + ".SavePeriod";
		proper->setProperty(strKey, szTmp);
		MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ContentStore, "set ICE property %s=%s"), strKey.c_str(), szTmp);

		sprintf(szTmp, "%d", _contentSaveSizeTrigger);
		strKey = dbContentAttrPrefix + ".SaveSizeTrigger";
		proper->setProperty(strKey, szTmp);
		MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ContentStore, "set ICE property %s=%s"), strKey.c_str(), szTmp);

		{
			_idxFileOfVol = new TianShanIce::Storage::FileOfVol(INDEXFILENAME(FileOfVol));
			std::vector<Freeze::IndexPtr> indices;
			indices.push_back(_idxFileOfVol);

			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "opening database " DBFILENAME_Content " with index FileOfVol"));
#if ICE_INT_VERSION / 100 >= 303
			_eContent = ::Freeze::createBackgroundSaveEvictor(_adapter, _dbPath + "Contents" +FNSEPS, DBFILENAME_Content, 0, indices);
#else
			_eContent = Freeze::createEvictor(_adapter, _dbPath + "Contents" +FNSEPS, DBFILENAME_Content, 0, indices);
#endif
			proper->setProperty("Freeze.Evictor." DBFILENAME_Content ".PageSize",              "8192");
			proper->setProperty("Freeze.Evictor." DBFILENAME_Content ".$default.BtreeMinKey",  "20");
//			proper->setProperty("Freeze.Evictor." DBFILENAME_Content ".$index:$default." INDEXFILENAME(FileOfVol) ".BtreeMinKey","20");

			_adapter->addServantLocator(_eContent, DBFILENAME_Content);
			_eContent->setSize(_contentEvictorSize);
		}

		{
			_idxChildVolume = new TianShanIce::Storage::ChildVolume(INDEXFILENAME(ChildVolume));
				std::vector<Freeze::IndexPtr> indices;
			indices.push_back(_idxChildVolume);

			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "opening database " DBFILENAME_Volume " with index ChildVolume"));
#if ICE_INT_VERSION / 100 >= 303
			_eVolume = ::Freeze::createBackgroundSaveEvictor(_adapter, _dbPath + "Contents" + FNSEPS, DBFILENAME_Volume, 0, indices);
#else
			_eVolume = Freeze::createEvictor(_adapter, _dbPath + "Contents" +FNSEPS, DBFILENAME_Volume, 0, indices);
#endif
			_adapter->addServantLocator(_eVolume, DBFILENAME_Volume);
			_eVolume->setSize(_volumeEvictorSize);
		}
	}
	catch(const Ice::Exception& ex)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (MOLOG, EXPFMT(ContentStore, 1001, "openDB() caught exception[%s]"), ex.ice_name().c_str());
	}
	catch(...)
	{
		ZQTianShan::_IceThrow<TianShanIce::ServerError> (MOLOG, EXPFMT(ContentStore, 1001, "openDB() caught unknown exception"));
	}

	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ContentStore, "database ready"));
}

void ContentStoreImpl::closeDB(void)
{
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "close local database"));
	_eContent      = NULL;
	_eVolume       = NULL;
}

int ContentStoreImpl::dumpDB(const char* outputFilename, bool compress, const char* volumeName)
{
#ifndef ENABLE_COMPRESS
	compress =false;
#endif // ENABLE_COMPRESS

	TianShanIce::Storage::VolumeInfos volumeInfos;
	try {
		volumeInfos = proxy(false)->listVolumes("", true);

		if (volumeName && strlen(volumeName) >0)
		{
			TianShanIce::Storage::VolumeInfos tmp = volumeInfos;
			volumeInfos.clear();

			for (TianShanIce::Storage::VolumeInfos::iterator itV = tmp.begin(); itV < tmp.end(); itV ++)
			{
				if (0 == itV->name.compare(volumeName))
				{
					volumeInfos.push_back(*itV);
					break;
				}
			}
		}
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "dumpDB() failed to list volumes, caught exception[%s] %s"), ex.ice_name().c_str(), ex.message.c_str());
		return -1;
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "dumpDB() failed to list volumes, caught exception[%s]"), ex.ice_name().c_str());
		return -1;
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "dumpDB() failed to list volumes, caught unknown exception"));
		return -1;
	}

	std::string ofilename = outputFilename ? outputFilename : "Contents.xml";
	{
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "dumpDB() openning output file[%s]"), ofilename.c_str());
		::std::ofstream file((ofilename + "~").c_str(), ::std::ios::out);
		::std::ostream* pout = (::std::ostream*) &file;
#ifdef ENABLE_COMPRESS
		::std::ofstream file2z((ofilename+".bz2~").c_str(), ::std::ios::out | ::std::ios::binary);
		ZQ::common::Bz2OutStream bz2(file2z);
		if (compress)
			pout = (::std::ostream*) &bz2;
#endif // ENABLE_COMPRESS

		char buf[64];
		::ZQTianShan::TimeToUTC(::ZQTianShan::now(), buf, sizeof(buf) -2);

		(*pout) << "<ContentStore netId=\"" << _netId << "\" asof=\"" << buf << "\" group=\"" << _replicaGroupId << "\" replica=\"" << _replicaId << "\" >\n";

		for (TianShanIce::Storage::VolumeInfos::iterator itV = volumeInfos.begin(); itV < volumeInfos.end(); itV ++)
		{
			::Ice::Identity identVol;
			identVol.name = itV->name;
			identVol.category = DBFILENAME_Volume;

			try {
				::TianShanIce::Storage::VolumeInfo& volumeinfo = *itV;

				(*pout) << "\n<Volume name=\"" << volumeinfo.name << "\" isVirtual=\"" << (volumeinfo.isVirtual ?1:0) << "\" quotaSpaceMB=\"" << volumeinfo.quotaSpaceMB << "\" >\n";
				for (::TianShanIce::Properties::iterator itMeta = volumeinfo.metaData.begin(); itMeta != volumeinfo.metaData.end(); itMeta ++)
					(*pout) << "\t<MetaData name=\"" << itMeta->first << "\" value=\"" << itMeta->second << "\" />\n";
				(*pout) << "</Volume>\n";

				IdentCollection cIdents = _idxFileOfVol->find(identVol);
				MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "dumpDB() vol[%s] %d contents found"), volumeinfo.name.c_str(), cIdents.size());

				for (IdentCollection::iterator it= cIdents.begin(); it < cIdents.end(); it++)
				{
					const std::string& cname = it->name;
					try {
						::TianShanIce::Storage::UnivContentPrx content = IdentityToObjEnv(*this, UnivContent, *it);
						::TianShanIce::Storage::ContentInfo contentInfo;
						contentInfo.fullname = it->name;
						contentInfo.name = content->getName();
						contentInfo.state = content->getState();
						contentInfo.metaData = content->getMetaData();

						::std::stringstream xmlstr;
						(*pout) << "\n<Content fullname=\"" << contentInfo.fullname << "\" name=\"" << contentInfo.name << "\" volume=\"" << volumeinfo.name << "\" state=\"" << contentInfo.state << "\" >\n";
						for (::TianShanIce::Properties::iterator itMeta = contentInfo.metaData.begin(); itMeta != contentInfo.metaData.end(); itMeta ++)
							(*pout) << "\t<MetaData name=\"" << itMeta->first << "\" value=\"" << itMeta->second << "\" />\n";
						(*pout) << "</Content>\n";
					}
					catch(const ::TianShanIce::BaseException& ex)
					{
						MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "dumpDB() vol[%s] failed to access content[%s], caught exception[%s] %s"), volumeinfo.name.c_str(), cname.c_str(), ex.ice_name().c_str(), ex.message.c_str());
						continue;
					}
					catch(const ::Ice::Exception& ex)
					{
						MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "dumpDB() vol[%s] failed to access content[%s], caught exception[%s]"), volumeinfo.name.c_str(), cname.c_str(), ex.ice_name().c_str());
						continue;
					}
					catch(...)
					{
						MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "dumpDB() vol[%s] failed to access content[%s], caught unknown exception"), volumeinfo.name.c_str(), cname.c_str());
						continue;
					}
				}

			}
			catch(const ::Ice::Exception& ex)
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "dumpDB() access vol[%s] caught exception[%s]"), identVol.name.c_str(), ex.ice_name().c_str());
			}
			catch(...)
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "dumpDB() access vol[%s] caught unknown exception"), identVol.name.c_str()); 
			}
		}

		(*pout) << "</ContentStore>\n";

		file.flush();

#ifdef ENABLE_COMPRESS
		bz2.zflush();
#endif // ENABLE_COMPRESS
	}

	::rename((ofilename + (compress?".bz2~":"~")).c_str(), (ofilename + (compress?".bz2":"")).c_str());

	::unlink((ofilename+".bz2~").c_str());
	::unlink((ofilename+"~").c_str());

	return 0;
}

void ContentStoreImpl::setFlags(uint32 flags)
{
	_storeFlags = flags;
}

::std::string ContentStoreImpl::getAdminUri(const ::Ice::Current& c)
{
#pragma message ( __MSGLOC__ "TODO: getAdminUri() impl here")
	ZQTianShan::_IceThrow<TianShanIce::NotImplemented> (MOLOG, EXPFMT(ContentStore, 9999, "Not implemented yet"));
	return std::string("");
}

::TianShanIce::State ContentStoreImpl::getState(const ::Ice::Current& c)
{
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "getState() caller: %s"), invokeSignature(c).c_str());
	return _serviceState;
}

::std::string ContentStoreImpl::getNetId(const ::Ice::Current& c) const
{
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "getNetId() caller: %s"), invokeSignature(c).c_str());
	return _netId;
}

::std::string ContentStoreImpl::type(const ::Ice::Current& c) const
{
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "type() caller: %s"), invokeSignature(c).c_str());
	return _storeType;
}

bool ContentStoreImpl::isValid(const ::Ice::Current& c) const
{
	if (_storeAggregate)
	{
		NodeReplicaQueue nrqueue;
		(const_cast<ContentStoreImpl*>(this))->buildNodeReplicaQueue(nrqueue);

		if (nrqueue.empty())
			return false;
	}

	return (TianShanIce::stInService == _serviceState);
}

::Ice::Byte ContentStoreImpl::getCacheLevel(const ::Ice::Current& c) const
{
	if (!_cacheable)
		ZQTianShan::_IceThrow <TianShanIce::NotSupported> (MOLOG, EXPFMT(ContentStore, 2001, "getCacheLevel() cache mode is not support in the ContentStore"));

	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "getCacheLevel() %d caller: %s"), _cacheLevel, invokeSignature(c).c_str());
	return _cacheLevel;
}

::TianShanIce::Replicas ContentStoreImpl::getStreamServices(const ::Ice::Current& c) const
{
	if (!_cacheable || !_exposeStreamService)
		ZQTianShan::_IceThrow <TianShanIce::NotSupported> (MOLOG, EXPFMT(ContentStore, 2001, "getStreamServices() cache mode is not on, or it is not allowed to expose StreamService"));

	::TianShanIce::Replicas streamServices;

	return streamServices;
}


//void ContentStoreImpl::OnFileEvent_async(const ::TianShanIce::Storage::AMD_ContentStoreEx_OnFileEventPtr& amdCB, ::TianShanIce::Storage::FileEvent event, const ::std::string& name, const ::TianShanIce::Properties& params, const ::Ice::Current& c)
void ContentStoreImpl::OnFileEvent(::TianShanIce::Storage::FileEvent event, const ::std::string& name, const ::TianShanIce::Properties& params, const ::Ice::Current& c)
{
	if (! (FLAG(event) & _fileEventFlags))
	{
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "OnFileEvent() ignore %s(%d) per _fileEventFlags[%08x]"), FileEventCmd::eventStr(event), event, _fileEventFlags);
		return;
	}

	if (_cFileEvent < THREDHOLD_FORCESYNC)
		_cFileEvent++;
	else _cFileEvent = THREDHOLD_FORCESYNC;

	try {
		if (!FileEventCmd::findPendingEvent(event, name))
			(new FileEventCmd(*this, event, name, params))->execute();
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore,"OnFileEvent() failed to initial FileEventCmd"));
	}
}

void ContentStoreImpl::queryReplicas_async(const ::TianShanIce::AMD_ReplicaQuery_queryReplicasPtr& amdCB, const ::std::string& category, const ::std::string& groupId, bool localOnly, const ::Ice::Current& c)
{
	try {
		(new QueryReplicaCmd(*(const_cast<ContentStoreImpl*> (this)), amdCB, category, groupId, localOnly))->execute();
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore,"queryReplicas_async() failed to list"));
		amdCB->ice_exception(::TianShanIce::ServerError("ContentStore", 502, "failed to generate queryReplicas_async"));
	}
}

void ContentStoreImpl::updateReplica_async(const ::TianShanIce::AMD_ReplicaSubscriber_updateReplicaPtr& amdCB, const ::TianShanIce::Replicas& stores, const ::Ice::Current& c)
{
	try {
		(new UpdateReplicaCmd(*(const_cast<ContentStoreImpl*> (this)), amdCB, stores))->execute();
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore,"updateStoreReplica_async() failed to list"));
		amdCB->ice_exception(::TianShanIce::ServerError("ContentStore", 503, "failed to generate UpdateReplicaCmd"));
	}
}

::TianShanIce::Replicas ContentStoreImpl::exportStoreReplicas()
{
	::TianShanIce::Replicas result;
	uint8 maxPrioritySeenInGroup = _replicaPriority;

	// step 1. prepare replica info about self
	::TianShanIce::Replica selfReplic;
	selfReplic.category  = "ContentStore";
	selfReplic.groupId   =  _replicaGroupId;
	selfReplic.replicaId = _replicaId;
	selfReplic.priority  = _replicaPriority;
	selfReplic.obj		 = proxy();
	selfReplic.stampBorn = 0;  //TODO: should be the start time of this run
	selfReplic.replicaState = TianShanIce::stInService;
#ifndef _INDEPENDENT_ADAPTER
	selfReplic.stampBorn = _adapter->getActivateTime();
#endif
	selfReplic.stampChanged = ZQTianShan::now();

	result.push_back(selfReplic);

	// step 2. copy the _storeReplicas to the result
	{
		if (_replicaTimeout <=0)
			_replicaTimeout = MIN_UPDATE_INTERVAL;

		::Ice::Long expiration = ZQTianShan::now() - _replicaTimeout *2;
		::Ice::Long stampToClear = ZQTianShan::now() - _replicaTimeout *3;
		::TianShanIce::StrValues repToClear;

		ZQ::common::MutexGuard g(_lockStoreReplicas);
		for (ReplicaMap::iterator it = _storeReplicas.begin(); it != _storeReplicas.end(); it ++)
		{
			if (it->second.stampUpdated < stampToClear)
			{
				repToClear.push_back(it->first);
				continue;
			}

			if (it->second.stampUpdated < expiration)
				continue; // treated as no more available

			result.push_back(it->second.replicaData);
			maxPrioritySeenInGroup = MAX(maxPrioritySeenInGroup, it->second.replicaData.maxPrioritySeenInGroup);
		}

		for (::TianShanIce::StrValues::iterator itClear = repToClear.begin(); itClear < repToClear.end(); itClear ++)
			_storeReplicas.erase(*itClear);
	}

	//step 3. update the maxPrioritySeenInGroup
	for (size_t i=0; i< result.size(); i++)
		result[i].maxPrioritySeenInGroup = maxPrioritySeenInGroup;

	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "exportStoreReplicas() ContentStore::%s %d records exported"), _replicaGroupId.c_str(), result.size());
	return result;
}

void ContentStoreImpl::getStoreReplicas(::TianShanIce::Replicas& replicas)
{
	ZQ::common::MutexGuard g(_lockStoreReplicas);
	for (ReplicaMap::iterator it = _storeReplicas.begin(); it != _storeReplicas.end(); it ++)
	{
		replicas.push_back(it->second.replicaData);
	}
}

int ContentStoreImpl::buildNodeReplicaQueue(NodeReplicaQueue& nodequeue)
{
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "enter buildNodeReplicaQueue()"));

	if (_replicaTimeout <=0)
		_replicaTimeout = MIN_UPDATE_INTERVAL;

	::Ice::Long expiration = ZQTianShan::now() - _replicaTimeout *2;

	ZQ::common::MutexGuard g(_lockStoreReplicas);
	for (ReplicaMap::const_iterator it = _storeReplicas.begin(); it != _storeReplicas.end(); it ++)
	{
		if (it->second.replicaData.replicaId.empty())
		{
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "buildNodeReplicaQueue() replica[%s] ignored, replicaId is empty, please check replicaId configuration on this replica"),
				it->second.replicaData.replicaId.c_str());
			continue;
		}

		if (it->second.stampUpdated < expiration)
		{
			char szLastUpdate[64], szExpire[64];
			memset(szLastUpdate, 0, sizeof(szLastUpdate));
			memset(szExpire, 0, sizeof(szExpire));
			::ZQTianShan::TimeToUTC(it->second.stampUpdated, szLastUpdate, sizeof(szLastUpdate) -2);
			::ZQTianShan::TimeToUTC(expiration, szExpire, sizeof(szExpire) -2);

			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ContentStore, "buildNodeReplicaQueue() replica[%s] ignored, lastUpdate[%s], expiration[%s]"),
				it->second.replicaData.replicaId.c_str(), szLastUpdate, szExpire);
			continue; // treated as no more available
		}

		nodequeue.push(it->second);
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "buildNodeReplicaQueue() node[%s]"), it->second.replicaData.replicaId.c_str());

	}

	MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ContentStore, "Leave buildNodeReplicaQueue()"));
	return nodequeue.size();
}


::std::string ContentStoreImpl::getVolumeName(const ::Ice::Current& c) const
{
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "getVolumeName() caller: %s"), invokeSignature(c).c_str());
	if (!_defaultVol)
		ZQTianShan::_IceThrow <TianShanIce::NotSupported> (MOLOG, EXPFMT(ContentStore, 3101, "getVolumeName() no default volume specified in this ContentStore"));

	return _defaultVol->getVolumeName();
}

::std::string ContentStoreImpl::getName(const ::Ice::Current& c) const
{
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "getName() caller: %s"), invokeSignature(c).c_str());
	return getVolumeName(c);
}


void ContentStoreImpl::getCapacity(::Ice::Long& freeMB, ::Ice::Long& totalMB, const ::Ice::Current& c)
	//throw (::TianShanIce::InvalidStateOfArt, ::TianShanIce::ServerError)
{
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "getCapacity() caller: %s"), invokeSignature(c).c_str());
	if (!_defaultVol)
		ZQTianShan::_IceThrow <TianShanIce::NotSupported> (MOLOG, EXPFMT(ContentStore, 3101, "getCapacity() no default volume specified in this ContentStore"));

	_defaultVol->getCapacity(freeMB, totalMB);
}

::TianShanIce::StrValues ContentStoreImpl::listContent(const ::std::string& condition, const ::Ice::Current& c) const
	//throw (::TianShanIce::InvalidStateOfArt)
{
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "listContent() caller: %s"), invokeSignature(c).c_str());
	if (!_defaultVol)
		ZQTianShan::_IceThrow <TianShanIce::NotSupported> (MOLOG, EXPFMT(ContentStore, 3101, "listContent() no default volume specified in this ContentStore"));

	return _defaultVol->listContent(condition);
}

::TianShanIce::Storage::ContentPrx ContentStoreImpl::openContent(const ::std::string& name, const ::std::string& destinationContentType, bool createIfNotExist, const ::Ice::Current& c)
	//throw (::TianShanIce::InvalidParameter, ::TianShanIce::InvalidStateOfArt, ::TianShanIce::ServerError)
{
	if (!_defaultVol)
		ZQTianShan::_IceThrow <TianShanIce::NotSupported> (MOLOG, EXPFMT(ContentStore, 3101, "openContent() no default volume specified in this ContentStore"));

	//_hashFolderExpsn ="$CUT(${NOW}, 0,4)/$CUT(${NOW}, 4,4)";
	if (!_hashFolderExpsn.empty())
	{
		// hashed folder defined
		std::string fullName = hashFolderNameByContentName(name, c);
		fullName += LOGIC_FNSEPS;
		fullName += name;

		if (LOGIC_FNSEPC == fullName[0])
		{
			::TianShanIce::Storage::FolderPrx folder;
			size_t posB=fullName.find(LOGIC_FNSEPC, 1), posN;
			std::string volName = fullName.substr(0, posB); posB++;

			try {
				::TianShanIce::Storage::VolumePrx vol = openVolume(volName, c);
				folder = ::TianShanIce::Storage::FolderPrx::checkedCast(vol->ice_collocationOptimized(false));
				if (!folder)
					return NULL;
			}
			catch (const TianShanIce::BaseException& ex) 
			{
				MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "openContent() failed to open volume[%s] per hashpath[%s] of content[%s] caught %s: %s"), volName.c_str(), fullName.c_str(), name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
				return NULL;
			}
			catch (const ::Ice::Exception& ex)
			{
				MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "openContent() failed to open volume[%s] per hashpath[%s] of content[%s] caught %s"), volName.c_str(), fullName.c_str(), name.c_str(), ex.ice_name().c_str());
				return NULL;
			}
			catch(...)
			{
				MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "openContent() failed to open volume[%s] per hashpath[%s] of content[%s]"), volName.c_str(), fullName.c_str(), name.c_str());
				return NULL;
			}

			std::string f;
			try {
				for (posN =fullName.find(LOGIC_FNSEPC, posB); std::string::npos != posN;
					posB=posN+1, posN =fullName.find(LOGIC_FNSEPC, posB))
				{
					f = fullName.substr(posB, posN-posB);
					folder = folder->openSubFolder(f, createIfNotExist, 0);
					if (!folder)
						return NULL;
				}
			}
			catch(...)
			{
				MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "openContent() failed to open subfolder[%s] per hashpath[%s] of content[%s] w/ create[%c]"), f.c_str(), fullName.c_str(), name.c_str(), createIfNotExist?'T':'F');
				return NULL;
			}

			std::string c = fullName.substr(posB);
			return folder->openContent(name, destinationContentType, createIfNotExist);
		}
	}

	return _defaultVol->openContent(name, destinationContentType, createIfNotExist);
}


void ContentStoreImpl::listContents_async(const ::TianShanIce::Storage::AMD_Folder_listContentsPtr& amdCB, const ::TianShanIce::StrValues& metaDataNames, const ::std::string& startName, ::Ice::Int maxCount, const ::Ice::Current& c) const
	//throw (::TianShanIce::ServerError)
{
	if (!_defaultVol)
		ZQTianShan::_IceThrow <TianShanIce::NotSupported> (MOLOG, EXPFMT(ContentStore, 3102, "listContents_async() no default volume specified in this ContentStore"));

	try {
		(new CSLstContsFwdr(*(const_cast<ContentStoreImpl*> (this)), amdCB, metaDataNames, startName, maxCount, _defaultVol))->execute();
	}
	catch (const ::Ice::Exception& ex)
	{
		amdCB->ice_exception(ex);
	}
	catch(...)
	{
		TianShanIce::ServerError ex("ContentStore", 3103, "listContents_async() caught unknown exception when redirect to the default volume");
		amdCB->ice_exception(ex);
	}
}

::TianShanIce::Storage::FolderPrx ContentStoreImpl::openSubFolder(const ::std::string& subname, bool createIfNotExist, ::Ice::Long quotaSpaceMB, const ::Ice::Current& c)
{
	if (subname.empty())
	{
		if (_defaultVol)
		{
			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "openSubFolder() returning the default volume"));
			return _defaultVol;
		}

		ZQTianShan::_IceThrow <TianShanIce::InvalidParameter> (MOLOG, EXPFMT(ContentStore, 3102, "openSubFolder() ContentStore has no default volume to open"));
	}

	::Ice::Identity ident;
	ident.name = (LOGIC_FNSEPC != subname[0]) ? std::string(LOGIC_FNSEPS) + subname : subname;
	ident.category = DBFILENAME_Volume;
	::TianShanIce::Storage::VolumePrx volume;

	try	{
		volume = IdentityToObjEnv(*this, VolumeEx, ident);
	}
	catch (const ::Ice::ObjectNotExistException&)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "openSubFolder() vol[%s] not exist"), ident.name.c_str());
	}
	catch (const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "openSubFolder() vol[%s] caught exception[%s]"), ident.name.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "openSubFolder() vol[%s] caught unknown exception"), ident.name.c_str());
	}

	if (createIfNotExist && volume)
		ZQTianShan::_IceThrow <TianShanIce::NotSupported> (MOLOG, EXPFMT(ContentStore, 3102, "openSubFolder() vol[%s] ContentStore is not allowed to create root volume"), ident.name.c_str());

	return volume;
}

::TianShanIce::Storage::FolderPrx  ContentStoreImpl::openFolderEx(const ::std::string& fullFolderName, bool createIfNotExist, ::Ice::Long quotaMB, const ::Ice::Current& c)
{
	std::string volumeName, pathName;

	if (fullFolderName.empty())
		ZQTianShan::_IceThrow <TianShanIce::InvalidParameter> (MOLOG, EXPFMT(ContentStore, 3102, "openFolderEx() ContentStore null full folder name"));

	if (LOGIC_FNSEPC != fullFolderName[0])
	{
		// the given fullFolderName doesn't start from the root, trust it as the subfolders of the default volume
		if (!_defaultVol)
			ZQTianShan::_IceThrow <TianShanIce::InvalidParameter> (MOLOG, EXPFMT(ContentStore, 3102, "openFolderEx() ContentStore illegal full folder name[%s] withno volumename"), fullFolderName.c_str());
		volumeName = _defaultVol->getIdent().name;
		pathName   = fullFolderName;
	}
	else
	{
		// the given fullFolderName start with with volume name
		size_t pos1 = fullFolderName.find_first_not_of(LOGIC_FNSEPS);
		if (std::string::npos == pos1)
			return _defaultVol;

		size_t pos2 = fullFolderName.find_first_of(LOGIC_FNSEPS, pos1);
		if (std::string::npos == pos2)
			return openVolume(fullFolderName, c);

		volumeName  = fullFolderName.substr(0, pos2 -pos1 +1);
		pathName    = fullFolderName.substr(pos2+1);
	}

	size_t pos = pathName.find_last_not_of("/\\");
	if (pos < pathName.length())
		pathName = pathName.substr(0, pos+1);

	std::stack <std::string> foldersToCreate;
	::Ice::Identity ident;
	ident.category = DBFILENAME_Volume;
	ident.name     = volumeName + LOGIC_FNSEPS + pathName;
	::TianShanIce::Storage::FolderPrx folder;
	while (!folder)
	{
		try	{
			folder = IdentityToObjEnv(*this, Folder, ident);
		}
		catch (const ::Ice::ObjectNotExistException&)
		{
			// note: no break here
		}
		catch (const ::Ice::Exception& ex)
		{
			break;
		}

		if (folder)
			break;

		foldersToCreate.push(ident.name);

		size_t pos = ident.name.find_last_of(LOGIC_FNSEPS);
		ident.name = ident.name.substr(0, pos);
	}

	// at least folder should be able to reach the level of Volume, so it should not be null.
	// if folder==null when reach here, then the volume event not exist or error occured. 
	// As we do not allow to create volume but only thru mountVolume, the func can quit
	if (!folder)
		return NULL;

	if (foldersToCreate.size()<=0)
		return folder; // the folder DB record has already existed, return it

	if (!createIfNotExist)
		return NULL; // the folder DB record has not existed but not ask to create, return NULL

	// to create the folder DB record recursively here
	while (folder && !foldersToCreate.empty())
	{
		// TODO: we do not allow to create the volume level DB record but thru the mountVolume() method
		std::string parentFolderName = folder->getName();
		std::string subFolderName = foldersToCreate.top().substr(parentFolderName.length()+1);
		foldersToCreate.pop();
		if (!subFolderName.empty())
			folder = folder->openSubFolder(subFolderName, true, quotaMB);
	}

	return folder;
}

::TianShanIce::Storage::VolumePrx ContentStoreImpl::openVolume(const ::std::string& fullname, const ::Ice::Current& c) const
{
	if (!isValid(c))
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore,"openVolume() failed to execute, because no node replica registered yet"));
		ZQTianShan::_IceThrow <TianShanIce::InvalidStateOfArt> (MOLOG, EXPFMT(ContentStore, 3103, "No node replica registered yet"));
	}

	if (0 == fullname.compare("$"))
		return _defaultVol;

	::Ice::Identity identVol;
	identVol.name = (LOGIC_FNSEPC != fullname[0]) ? std::string(LOGIC_FNSEPS) + fullname : fullname;
	identVol.category = DBFILENAME_Volume;
	::TianShanIce::Storage::VolumePrx volume;

	try	{
		volume = IdentityToObjEnv(*this, Volume, identVol);
	}
	catch (const ::Ice::ObjectNotExistException&)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "openVolume() vol[%s] not exist"), identVol.name.c_str());
	}
	catch (const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "openVolume() vol[%s] caught exception[%s]"), identVol.name.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "openVolume() vol[%s] caught unknown exception"), identVol.name.c_str());
	}

	if (!volume)
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "openVolume() vol[%s] failed to open the specified volume"), identVol.name.c_str());

	return volume;
}

void ContentStoreImpl::listVolumes_async(const ::TianShanIce::Storage::AMD_ContentStore_listVolumesPtr& amdCB, const ::std::string& listFrom, bool includingVirtual, const ::Ice::Current& c) const
{
	if (!isValid(c))
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore,"listVolumes_async() failed to execute, because no node replica registered yet"));
		amdCB->ice_exception(::TianShanIce::InvalidStateOfArt("ContentStore", 503, "No node replica registered yet"));
		return;
	}

	try {
		(new ListVolumesCmd(*(const_cast<ContentStoreImpl*> (this)), amdCB, listFrom, includingVirtual))->execute();
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore,"listVolumes_async() failed to generate ListVolumesCmd"));
		amdCB->ice_exception(::TianShanIce::ServerError("ContentStore", 503, "failed to generate ListVolumesCmd"));
	}
}

::TianShanIce::Storage::FolderPrx ContentStoreImpl::parent(const ::Ice::Current& c) const
{
	if (!_defaultVol)
		return NULL;

	return _defaultVol->parent();
}

::TianShanIce::Storage::ContentPrx ContentStoreImpl::openContentByFullname(const ::std::string& fullname, const ::Ice::Current& c) const
{	
	IceUtil::Time openWatchStart = IceUtil::Time::now();
	::Ice::Identity identContent;
	identContent.name = fullname;
	identContent.category = DBFILENAME_Content;

	::TianShanIce::Storage::ContentPrx contentPrx;

	try
	{
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "openContentByFullname() opening content by fullname[%s]"), identContent.name.c_str());
		contentPrx = IdentityToObj2(Content, identContent);
		IceUtil::Time openWatchEnd = IceUtil::Time::now();
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "openContentByFullname() content fullname[%s] openned, time cost[%lld]ms"),
			identContent.name.c_str() , (openWatchEnd - openWatchStart).toMilliSeconds() );

	}
	catch (const ::Ice::ObjectNotExistException&)
	{
	}
	catch (const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "openContentByFullname() failed to open content by fullname[%s]: caught exception[%s]"), identContent.name.c_str(), ex.ice_name().c_str());
	}
	catch (...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "openContentByFullname() failed to open content by fullname[%s]: caught unknown exception"), identContent.name.c_str());
	}

	if (contentPrx || ! (_storeFlags & STOREFLAG_checkFSonOpenContentByFullName))
		return contentPrx; // return only based on DB records here

	std::string volumeName, folderName, contentName;
	try
	{
		// necessary to double check FS, open via Folder::openContent()
		if (chopPathname(fullname, volumeName, folderName, contentName) <2)
			return contentPrx;

		Ice::Identity identFolder;
		identFolder.name = volumeName + LOGIC_FNSEPS + folderName;
		identFolder.category = DBFILENAME_Volume;

		::TianShanIce::Storage::FolderPrx fldr = IdentityToObjEnv(*this, VolumeEx, identFolder);
		if (!fldr)
			return contentPrx;

		if (fldr)
			contentPrx = fldr->openContent(contentName, "", false);		

		IceUtil::Time openWatchEnd = IceUtil::Time::now();
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "openContentByFullname() content fullname[%s] openned from folder, time cost[%lld]ms"),
			identContent.name.c_str() , (openWatchEnd - openWatchStart).toMilliSeconds() );
	}
	catch (const ::Ice::ObjectNotExistException&)
	{
	}
	catch (const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "openContentByFullname(%s) failed to invoke vol[%s]folder[%s]:openContent(%s) caught exception[%s]"), identContent.name.c_str(), volumeName.c_str(), folderName.c_str(), contentName.c_str(), ex.ice_name().c_str());
	}
	catch (...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "openContentByFullname(%s) failed to invoke vol[%s]folder[%s]:openContent(%s) caught exception"), identContent.name.c_str(), volumeName.c_str(), folderName.c_str(), contentName.c_str());
	}

	return contentPrx; // return only based on DB records here
}

void ContentStoreImpl::destroy(const ::Ice::Current& c)
		//throw (::TianShanIce::NotSupported, ::TianShanIce::InvalidStateOfArt)
{
	ZQTianShan::_IceThrow <TianShanIce::NotSupported> (MOLOG, EXPFMT(ContentStore, 7001, "Volume::destroy() is forbidden at interface ContentStore"));
}

::TianShanIce::Storage::VolumePrx ContentStoreImpl::mountStoreVolume(const ::std::string& name, const ::std::string& mountPath, const bool defaultVolume)
{
	::TianShanIce::Storage::VolumeExPrx volume;

	if (name.empty() || std::string::npos != name.find_first_of(ILLEGAL_MOUNTNAME_CHARS) )
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (MOLOG, EXPFMT(Volume, 7007, "mountStoreVolume() vol[%s] illegal volume name to mount path[%s]"), name.c_str(), mountPath.c_str());

	::Ice::Identity ident;
	ident.name = LOGIC_FNSEPC == name[0] ? name : (std::string(LOGIC_FNSEPS) + name);
	ident.category = DBFILENAME_Volume;

	std::string newPath = fixupPathname(*this, mountPath);
	std::string oldPath;

	if (!ContentStoreImpl::createPathOfVolume(*this, newPath, ident.name))
		::ZQTianShan::_IceThrow <TianShanIce::InvalidParameter> (MOLOG, EXPFMT(Volume, 7004, "mountStoreVolume() failed to create the path-of-volume[%s] on the contentstore portal"), newPath.c_str());


	try	{
		volume = IdentityToObjEnv(*this, VolumeEx, ident);

		if (volume)
			oldPath = volume->getMountPath();
	}
	catch (const ::Ice::ObjectNotExistException&)
	{
		// log nothing here
	}
	catch (const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "mountStoreVolume() vol[%s] open vol caught exception[%s]"), ident.name.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "mountStoreVolume() vol[%s] open vol caught unknown exception"), ident.name.c_str());
	}

	if (volume)
	{
		if (0 != newPath.compare(oldPath))
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter> (MOLOG, EXPFMT(ContentStore, 7005, "mountStoreVolume() mount an existing root volume [%s] with different path: new path [%s], old path [%s]"), ident.name.c_str(), newPath.c_str(), oldPath.c_str());
	
 		//init the volume subfolders
		initVolumeSubFolder(volume);
	}
	else
	{
#pragma message ( __MSGLOC__ "TODO: mountStoreVolume() validate if there is any conflict with the existing volume hirechy and contents")

		// create the new volume here
		try {
			VolumeImpl::Ptr vol  = new VolumeImpl(*this);
			vol->ident            = ident;
			vol->parentVolName    = LOGIC_FNSEPS;

			vol->isVirtual		  = false;
			vol->mountPath        = newPath;
			vol->quotaSpaceMB     = -1;
			vol->stampCreated     = now();

			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "mountStoreVolume() vol[%s] adding into the database"), ident.name.c_str());
			_eVolume->add(vol, vol->ident);
			volume = IdentityToObjEnv(*this, VolumeEx, ident);

		}
		catch (const TianShanIce::BaseException& ex) 
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "mountStoreVolume() vol[%s] add new record exception[%s]: %s"), ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		}
		catch (const ::Ice::Exception& ex)
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "mountStoreVolume() vol[%s] add new record exception[%s]"), ident.name.c_str(), ex.ice_name().c_str());
		}
		catch(...)
		{
			MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "mountStoreVolume() vol[%s] add new record unknown exception"), ident.name.c_str());
		}
	}

	try {
		if (volume)
		{
#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
			MOEVENT(EventFMT(_netId.c_str(), Volume, Mounted, 3, "vol[%s] path[%s]"), ident.name.c_str(), newPath.c_str());
			OnVolumeMounted(ident, newPath);

			if (_autoFileSystemSync)
			{
				MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "mountStoreVolume() vol[%s] scheduleing a sync"), ident.name.c_str());
				TianShanIce::Properties metadata;
				MAPSET(TianShanIce::Properties, metadata, SYS_PROP(PendingFileSystemSync), "1");
				volume->setMetaData(metadata);
			}

			volume->OnTimer();

			if (defaultVolume)
			{
				_defaultVol = volume; // make the _defaultVol as the volume with ice_collocationOptimization disabled
				MOLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(ContentStore, "mountStoreVolume() vol[%s] set as the default volume of this ContentStore"), ident.name.c_str());
			}
			
			MOLOG(ZQ::common::Log::L_NOTICE, CLOGFMT(ContentStore, "mountStoreVolume() vol[%s] mounted, path[%s]"), ident.name.c_str(), newPath.c_str());
		}
	}
	catch (const TianShanIce::BaseException& ex) 
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "mountStoreVolume() vol[%s] force to sync with file system exception[%s]: %s"), ident.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch (const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "mountStoreVolume() vol[%s] force to sync with file system exception[%s]"), ident.name.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "mountStoreVolume() vol[%s] force to sync with file system unknown exception"), ident.name.c_str());
	}

	ZQ::common::MutexGuard g(_lockStoreReplicas); // just simply borrow an existing locker
	MAPSET(TianShanIce::Properties, _idxMountPathToVolumeName, mountPath, name);

	return volume;
}

std::string ContentStoreImpl::mountPathToVolName(const ::std::string& mountPath, const ::Ice::Current& c)
{
	ZQ::common::MutexGuard g(_lockStoreReplicas); // just simply borrow an existing locker
	TianShanIce::Properties::iterator it = _idxMountPathToVolumeName.find(mountPath);
	return (_idxMountPathToVolumeName.end() != it) ? it->second : "";
}

::std::string ContentStoreImpl::hashFolderNameByContentName(const ::std::string& contentName, const ::Ice::Current& c)
{
	size_t pos = contentName.find_last_of(LOGIC_FNSEPS);
	if (std::string::npos != pos) // the given contentName already has folder part, cut and return directly
	{
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "hashFolderNameByContentName() contentName[%s] already has folder part, cut and return directly"), contentName.c_str());
		return contentName.substr(0, pos+1);
	}

	std::string folderName = _hashFolderExpsn;

	if (!folderName.empty()) 
	{
		// hash folder is not defined, associate the expression
		char buf[32];
		std::string NowStr = ZQ::common::TimeUtil::TimeToUTC(ZQ::common::now(), buf, sizeof(buf)-2, true);
		size_t posH, posT, posV;
		while (std::string::npos != (posH = NowStr.find_first_of("-:")))
			NowStr.replace(posH, 1, "");
		if (std::string::npos != (posH = NowStr.find_first_of(".+")))
			NowStr = NowStr.substr(0, posH);

		while(1)
		{
			posH = folderName.find("${");
			if (std::string::npos != posH)
			{
				posT = folderName.find('}', posH+2);
				if (std::string::npos != posT)
				{
					// find a variable, replace it
					std::string var = folderName.substr(posH+2, posT -posH -2);
					if (var == "ContentName")
					{
						folderName.replace(posH, posT -posH +1, contentName);
						continue;
					}

					if (var == "NOW")
					{
						folderName.replace(posH, posT -posH +1, NowStr);
						continue;
					}

					//TODO: more variables
				}

			}

			// TODO: parse func: posH = _hashFolderExpsn.find("$")
			posH = folderName.find("$");
			if (std::string::npos != posH)
			{
				posV = folderName.find("(", posH+1);
				posT = folderName.find(')', posH+1);

				if (posT > posV && posV >posH)
				{
					std::string funcName = folderName.substr(posH+1, posV -posH -1);
					if (funcName == "now")
					{
						folderName.replace(posH, posT -posH +1, NowStr);
						continue;
					}

					if (funcName == "CUT")
					{
						char buf[256], *p = NULL;
						int start=0, len=0, c;
						std::string str;

						c = sscanf(folderName.c_str() + posV+1, "%[^,) \t],%d,%d", buf, &start, &len);
						if (c>0)
							p = buf;
						if (p && c>1 && start>=0 && start <sizeof(buf)-2)
							p+= start;
						if (p && c>2 && len>=0 && len < buf+sizeof(buf)-1-p)
							*(p + len) ='\0';

						if (NULL != p)
							folderName.replace(posH, posT -posH +1, p);
						continue;
					}

					//TODO: more funcs
				}
			}

			break; // quit the loop if nothing matched
		}
	}

	if (LOGIC_FNSEPC != folderName[0])
		folderName = getVolumeName(c) + LOGIC_FNSEPS + folderName;

	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "hashFolderNameByContentName() hashed contentName[%s] to folder[%s] per expression[%s]"), contentName.c_str(), folderName.c_str(), _hashFolderExpsn.c_str());
	return folderName;
}


bool ContentStoreImpl::subscribeStoreReplica(const std::string proxyStrToSubscriber, bool masterStoreReplica)
{
	ReplicaSubscriberInfo sinfo;
	sinfo.proxyStr = proxyStrToSubscriber;
	sinfo.lastUpdated = 0;
	sinfo.timeout = _replicaTimeout;

	try {
		sinfo.subscriber = ::TianShanIce::ReplicaSubscriberPrx::uncheckedCast(_adapter->getCommunicator()->stringToProxy(sinfo.proxyStr));
	}
	catch(...) 	{}

	if (!sinfo.subscriber)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "subscribeStoreReplica() invaild store replica subscriber[%s]"), sinfo.proxyStr.c_str());
		return false;
	}

	ZQ::common::MutexGuard gd(_lockReplicaSubscriberMap);
	MAPSET(ReplicaSubscriberMap, _replicaSubscriberMap, sinfo.proxyStr, sinfo);
	if (masterStoreReplica)
		_prxstrMasterReplica = sinfo.proxyStr;

	_watchDog.watch(_localId, 0);
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "subscribeStoreReplica() store replica subscriber[%s] added"), sinfo.proxyStr.c_str());
	return true;
}

void ContentStoreImpl::OnTimer(const ::Ice::Current& c)
{
#ifdef _DEBUG
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "OnTimer()"));
#endif // _DEBUG

	::Ice::Long stampNow = ::ZQTianShan::now();

	::TianShanIce::Replicas reps;
	if (_replicaSubscriberMap.size() >0)
		reps = exportStoreReplicas();

	long minNextTimeout = UNATTENDED_TIMEOUT;

	{
		ZQ::common::MutexGuard gd(_lockReplicaSubscriberMap);
		for (ReplicaSubscriberMap::iterator it =_replicaSubscriberMap.begin(); it != _replicaSubscriberMap.end(); it ++)
		{
			if (it->second.timeout <=0)
				it->second.timeout = UNATTENDED_TIMEOUT;

			::Ice::Long nextTimeout = it->second.lastUpdated + (long) (it->second.timeout * 0.7) - stampNow;
			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "OnTimer() lastUpdated="FMT64" timeout=%d nextTimeout="FMT64),
				it->second.lastUpdated, it->second.timeout, nextTimeout);

			if (nextTimeout <= 0)
			{
				try {
					it->second.lastUpdated = stampNow; // refresh lastUpdated here to avoid entering dead loop
					minNextTimeout = (long) MIN(minNextTimeout, (it->second.timeout * 0.7));

					(new ReportReplicaCmd(*this, it->second, reps))->execute();
				}
				catch(...) 	{}
			}
			else minNextTimeout = (long) MIN(minNextTimeout, nextTimeout);
		}
	}

	// summarize the volume usages
	static ::Ice::Long stampLastVolumeSummarize=0;
	if (_serviceState == TianShanIce::stInService && stampNow - stampLastVolumeSummarize > UNATTENDED_TIMEOUT *4)
	{
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "OnTimer() summarizing volume usages"));
		IdentCollection idents;
		try {
			IdentCollection idents = _idxChildVolume->find(LOGIC_FNSEPS); // find the first layer of volumes
		}
		catch (...) {}

		for (IdentCollection::iterator it = idents.begin(); it <idents.end(); it++)
		{
			try {
				::Ice::Long freeMB=0, totalMB=0;
				::TianShanIce::Storage::VolumeExPrx volume = IdentityToObjEnv(*this, VolumeEx, *it);
				volume->getCapacity(freeMB, totalMB);
				MOEVENT(EventFMT(_netId.c_str(), Volume, Usage, 5, "vol[%s] freeMB[%lld] totalMB[%lld]"), it->name.c_str(), freeMB, totalMB);
			}
			catch (const ::Ice::Exception& ex)
			{
				MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "OnTimer() check volume[%s] usages caught exception[%s]"), it->name.c_str(), ex.ice_name().c_str());
			}
			catch (...)
			{
				MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "OnTimer() check volume[%s] usages caught runtime exception"), it->name.c_str());
			}
		}

		stampLastVolumeSummarize = stampNow;
	}

	_watchDog.watch(_localId, minNextTimeout);
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "OnTimer() minNextTimeout=%d"), minNextTimeout);
}

void ContentStoreImpl::enableAutoFileSystemSync(bool enable, const ::Ice::Current& c)
{
	_autoFileSystemSync = enable;
}

void ContentStoreImpl::OnProvisionEvent(::TianShanIce::Storage::ProvisionEvent event, const ::std::string& storeNetId, const ::std::string& volumeName, const ::std::string& contentName, const ::TianShanIce::Properties& params, const ::Ice::Current& c)
{
	if (0 != _netId.compare(storeNetId))
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "OnProvisionEvent() event[%d] netId[%s] Provision[%s,%s,%s] netId doen't match local"), event, _netId.c_str(), storeNetId.c_str(), volumeName.c_str(), contentName.c_str());
		return;
	}

	::Ice::Identity identContent;
	identContent.category = DBFILENAME_Content;
	identContent.name = volumeName + LOGIC_FNSEPS + contentName;
	if (LOGIC_FNSEPC != identContent.name[0])
		identContent.name = std::string(LOGIC_FNSEPS) + identContent.name;

	::TianShanIce::Storage::UnivContentPrx content;
	::TianShanIce::Storage::ContentState state = ::TianShanIce::Storage::csNotProvisioned;
	std::string contentShortName, vn;

	try
	{
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "OnProvisionEvent() event[%d] content[%s]"), event, identContent.name.c_str());
		content = IdentityToObjEnv(*this, UnivContent, identContent);
		state = content->getState();
		contentShortName = content->getName();
		vn = identContent.name.substr(0, identContent.name.length() - contentShortName.length() -1);
	}
	catch (const Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "OnProvisionEvent() event[%d] content[%s] access DB caught exception[%s]"), event, identContent.name.c_str(), ex.ice_name().c_str());
		return;
	}

	if (!content)
	{
		MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "OnProvisionEvent() event[%d] content[%s] doesn't exist, ignore the event"), event, identContent.name.c_str());
		return;
	}

	try
	{
		::TianShanIce::Properties::const_iterator itParam;
		switch(event)
		{
		case ::TianShanIce::Storage::peProvisionStarted:
			{
				if (params.size()>0)
				{
					::TianShanIce::Storage::UnivContentPrx uniContent = ::TianShanIce::Storage::UnivContentPrx::uncheckedCast(content);
					uniContent->setMetaData(params);
				}
				
				content->enterState(::TianShanIce::Storage::csProvisioning);
				
	#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
				MOEVENT(EventFMT(_netId.c_str(), Content, ProvisionStarted, 20, "Content[%s] vol[%s] name[%s]"), identContent.name.c_str(), vn.c_str(), contentShortName.c_str());
				OnContentProvisionStarted(identContent);

				break;
			}
		case ::TianShanIce::Storage::peProvisionStopped:
			{
				uint32 lastError = 0;
				std::string lastErrMsg;

				itParam = params.find("sys.LastError");
				if (params.end() != itParam)
					lastError = (uint32) atoi(itParam->second.c_str());

				itParam = params.find("sys.LastErrMsg");
				if (params.end() != itParam)
					lastErrMsg = itParam->second;

#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
				MOEVENT(EventFMT(_netId.c_str(), Content, ProvisionStopped, 21, "Content[%s] vol[%s] name[%s] LastError[%d] LastErrMsg[%s]"), identContent.name.c_str(), vn.c_str(), contentShortName.c_str(), lastError, lastErrMsg.c_str());
				OnContentProvisionStopped(identContent);

				if (lastError < 300)
				{
					::TianShanIce::Storage::UnivContentPrx uniContent = ::TianShanIce::Storage::UnivContentPrx::uncheckedCast(content);
					uniContent->setMetaData(params);

					content->enterState(::TianShanIce::Storage::csInService);					
				}
				else
				{
					content->enterState(::TianShanIce::Storage::csOutService);
				}
			}
			break;

		case ::TianShanIce::Storage::peProvisionProgress:
			if (::TianShanIce::Storage::csNotProvisioned == state)
				state = content->enterState(::TianShanIce::Storage::csProvisioning);

			if (::TianShanIce::Storage::csProvisioning == state || ::TianShanIce::Storage::csProvisioningStreamable == state)
			{
				::TianShanIce::Storage::UnivContentPrx uniContent = ::TianShanIce::Storage::UnivContentPrx::uncheckedCast(content);
				uniContent->setMetaData(params);

				::Ice::Long processed=0, total=0;
				itParam = params.find("sys.ProgressProcessed");
				if (params.end() != itParam)
					processed = (::Ice::Long) _atoi64(itParam->second.c_str());

				itParam = params.find("sys.ProgressTotal");
				if (params.end() != itParam)
					total = (::Ice::Long) _atoi64(itParam->second.c_str());

				if (::TianShanIce::Storage::csProvisioning == state)
					uniContent->OnFileModified();


#ifdef PROCESSED_BY_TIME
				::Ice::Long proctime  = 0;
				::Ice::Long totaltime = 0;
				itParam = params.find("sys.realplaytime");
				if (itParam != params.end())
					proctime = (::Ice::Long) _atoi64(itParam->second.c_str());

				itParam = params.find("sys.allplaytime");
				if (itParam != params.end())
					totaltime = (::Ice::Long) _atoi64(itParam->second.c_str());

				float  c = 0; 
				double a = 0;
				double b = 0;
				a = (double)proctime;
				b = (double)totaltime;
				if (0 != totaltime)
					c = 100 * a / b;

#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event, add the percent of play time")
				MOEVENT(EventFMT(_netId.c_str(), Content, ProvisionProgress, 22, 
                        "Content[%s] vol[%s] name[%s] processed["FMT64"] total["FMT64"] process_percent[%.0f%%] "), //"FMT64"
                        identContent.name.c_str(), vn.c_str(), contentShortName.c_str(), processed, total, c);
#else


#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event====")
				MOEVENT(EventFMT(_netId.c_str(), Content, ProvisionProgress, 22, 
                        "Content[%s] vol[%s] name[%s] processed["FMT64"] total["FMT64"]"), 
                        identContent.name.c_str(), vn.c_str(), contentShortName.c_str(), processed, total);
#endif//PROCESSED_BY_TIME
				OnContentProvisionProgress(identContent, processed, total);
			}

			break;


		case ::TianShanIce::Storage::peProvisionStreamable: ///< the event when the provision think its output can be streamable

			content->enterState(::TianShanIce::Storage::csProvisioningStreamable);
			content->populateAttrsFromFilesystem(); // force to populating the content attributes here

#pragma message ( __MSGLOC__ "WARNING: Sentry should parse this message to publish event")
			MOEVENT(EventFMT(_netId.c_str(), Content, ProvisionStreamable, 23, "Content[%s] vol[%s] name[%s] streamable"), identContent.name.c_str(), vn.c_str(), contentShortName.c_str());
			OnContentProvisionStreamable(identContent);
			break;

		default:
			MOLOG(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "OnProvisionEvent() event[%d] content[%s] unkown provision event"), event, identContent.name.c_str());
		}
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "OnProvisionEvent() event[%d] content[%s] processing caught exception[%s] %s"), event, identContent.name.c_str(), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "OnProvisionEvent() event[%d] content[%s] processing caught exception[%s]"), event, identContent.name.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "OnProvisionEvent() event[%d] content[%s] processing caught unknown exception"), event, identContent.name.c_str());
	}
}

void ContentStoreImpl::updateStoreReplicas(const ::TianShanIce::Replicas& replicas)
{
	char buf[512];
	std::string selfId = std::string(CATEGORY_ContentStore "::") + _replicaGroupId + "::" + _replicaId;

#ifdef _DEBUG
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "updateStoreReplicas() update %d store replicas"), replicas.size());
#endif //_DEBUG

	int cNew =0, cUpdate=0;
	NodeReplica nr;
	nr.stampUpdated = now();

	ZQ::common::MutexGuard g(_lockStoreReplicas);
	for (::TianShanIce::Replicas::const_iterator it = replicas.begin(); it < replicas.end(); it ++)
	{
		snprintf(buf, sizeof(buf)-2, "%s::%s::%s", it->category.c_str(), it->groupId.c_str(), it->replicaId.c_str());
		if (0 != it->category.compare(CATEGORY_ContentStore) || 0 != it->groupId.compare(_replicaGroupId))
		{
			MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "updateStoreReplicas() ignore replica[%s]"), buf);
			continue;
		}

		//if (0 == selfId.compare(buf)) // skip the replica info about self but reported by others  //rid it because of that cdncs and cdnss installed on the same machine
		//	continue;

		ReplicaMap::iterator itl = _storeReplicas.find(buf);
		if (itl == _storeReplicas.end())
		{
			nr.replicaData = *it;
			_storeReplicas.insert(ContentStoreImpl::ReplicaMap::value_type(buf, nr));
//			_storeReplicas[buf].stampKnew = _storeReplicas[buf].stampUpdated = ZQTianShan::now();
			cNew ++;

			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ContentStore, "updateStoreReplicas() replica[%s|%s|%s|%d] discovered"),
				it->category.c_str(), it->groupId.c_str(), it->replicaId.c_str(), it->priority);	
		}
		else
		{
			itl->second.stampUpdated = nr.stampUpdated;

			if (itl->second.replicaData.stampBorn < it->stampBorn)
			{
				MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ContentStore, "updateStoreReplicas() replica[%s|%s|%s|%d] restarted"),
					it->category.c_str(), it->groupId.c_str(), it->replicaId.c_str(), it->priority);	
			}
			
			MOLOG(ZQ::common::Log::L_INFO, CLOGFMT(ContentStore, "updateStoreReplicas() replica[%s|%s|%s|%d] updated"),
				it->category.c_str(), it->groupId.c_str(), it->replicaId.c_str(), it->priority);	

			itl->second.replicaData = *it;
			cUpdate ++;
		}
	}

	static int cMutedUpdate =0;
	cMutedUpdate = (cMutedUpdate+1) % 20; // reduce some noise if this update is too frequently

	if (cNew >0  || 0 == cMutedUpdate)
		MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "updateStoreReplicas() found %d new store replicas, updated %d known store replicas t(%d). %d in the known list"),
					cNew, cUpdate, cMutedUpdate, _storeReplicas.size());

}

void ContentStoreImpl::clearStoreReplicas()
{	
	ZQ::common::MutexGuard g(_lockStoreReplicas);
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "clearStoreReplicas() exist %d store replicas"), _storeReplicas.size());
	_storeReplicas.clear();
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "clearStoreReplicas() store replicas cleared"));
}

void ContentStoreImpl::OnIdle()
{
	static int _cIdle =0;
	static ::Ice::Long _lastIdleExec = 0;
	if (_thpool.pendingRequestSize() >0)
	{
		// skip if the service is busy
		_cIdle = 0;
		return;
	}

	// test for continous 5 idle enties
	if (++_cIdle < 5)
		return;

	_cIdle =0;
	::Ice::Long stampNow = now();
	if (stampNow - _lastIdleExec < UNATTENDED_TIMEOUT * 10)
		return; // no need to do the OnIdle work too frequently

	// let's start if the program can hit here

	char buf[256];
	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "OnIdle() performing maintainence procedure while idle. lastRun=[%s]"), ZQTianShan::TimeToUTC(_lastIdleExec, buf, sizeof(buf)-2));

#pragma message ( __MSGLOC__ "TODO: impl OnIdle() performing maintainence procedure")

// step 1.1. recovery any errors in the database by executing: db_recover
// step 1.2. clean up the old log in the database by executing: db_archive -d
// step 1.3. backup the database by executing: db_dump to a file
// step 2. call dumpDB() to make an xml backup under %DBDIR%\BaseCS-%stampNow%.xml, bzip2 it if necessary
// step 3. bzip2 the old log files if necessary
// step 4. is it worthy to redo a syncDBwithFS() every week?

	_lastIdleExec = stampNow;
}

/// -----------------------------
/// class XmlImporter
/// -----------------------------
class XmlImporter : public ZQ::common::ExpatBase
{
public:
	XmlImporter(ContentStoreImpl& store)
		:_store(store)
	{
	}

	virtual ~XmlImporter() {}

	virtual int parseFile(const char *szFilename, bool compressed=false, const char* volumeName=NULL)
	{
		if (NULL == szFilename)
			return -1;

#ifndef ENABLE_COMPRESS
		 compressed=false;
#endif // ENABLE_COMPRESS

		_searchForVolume = volumeName ? volumeName: "";
		if (0 == _searchForVolume.compare("*"))
			_searchForVolume = "";

		// Open the specified file
		std::ifstream file(szFilename);
		if (!file.is_open())
			return -2;

		std::istream* pin = &file;

#ifdef ENABLE_COMPRESS
		std::ifstream zfile(szFilename, ::std::ios::in | ::std::ios::binary);
		if (compressed && !zfile.is_open())
			return -3;

		ZQ::common::Bz2InStream unbz2(zfile);
		if (compressed)
			pin = &unbz2;
#endif // ENABLE_COMPRESS

		// Read to EOF
		char szBuffer[8192];
		int cBytes = 0;
		for (bool done = (*pin).eof(); !done;)
		{
			// Read data from the input file; store the bytes read
			(*pin).read(szBuffer, sizeof(szBuffer));
			done = (*pin).eof();
			ZQ::common::ExpatBase::parse(szBuffer, (*pin).gcount(), done);
			cBytes += (*pin).gcount();
		}
		return cBytes;
	}

protected:
	ContentStoreImpl& _store;
	std::string _searchForVolume;

	std::string _currentContent, _currentVolume;
	::TianShanIce::Properties _volumeMetaData, _contentMetaData;

	// overrideable callbacks, from ExpatBase
	virtual void OnStartElement(const XML_Char* name, const XML_Char** atts)
	{
		::std::string hiberarchyName = getHiberarchyName();
		::TianShanIce::Properties attrMap;
		for (int n = 0; atts[n]; n += 2)
			MAPSET(::TianShanIce::Properties, attrMap, atts[n], atts[ n + 1 ]);

		if (0 == hiberarchyName.compare("/ContentStore/Volume"))
		{
			_currentContent = _currentVolume = "";
			_contentMetaData.clear();
			_volumeMetaData.clear();

			_currentVolume = attrMap["name"];
			if (_searchForVolume.empty() || 0 == _searchForVolume.compare(_currentVolume))
				_currentVolume = "";

			return;
		}
		
		if (0 == hiberarchyName.compare("/ContentStore/Content"))
		{
			_currentContent = "";
			_contentMetaData.clear();

			if (_searchForVolume.empty() || 0 == _searchForVolume.compare(attrMap["volume"]))
			{
				_currentContent = attrMap["fullname"];
				MAPSET(::TianShanIce::Properties, _contentMetaData, SYS_PROP(FullName), _currentContent);
				MAPSET(::TianShanIce::Properties, _contentMetaData, SYS_PROP(ContentName), attrMap["name"]);
				MAPSET(::TianShanIce::Properties, _contentMetaData, SYS_PROP(StateId), attrMap["state"]);
				MAPSET(::TianShanIce::Properties, _contentMetaData, SYS_PROP(Volume), attrMap["volume"]);
			}

			return;
		}
		
		if (0 == hiberarchyName.compare("/ContentStore/Content/MetaData") && !_currentContent.empty())
		{
			std::string MDName = attrMap["name"];
			std::string MDValue = attrMap["value"];

			MAPSET(::TianShanIce::Properties, _contentMetaData, MDName, MDValue);
			return;
		}
		
		if (0 == hiberarchyName.compare("/ContentStore/Volume/MetaData") && !_currentVolume.empty())
		{
			std::string MDName = attrMap["name"];
			std::string MDValue = attrMap["value"];

			MAPSET(::TianShanIce::Properties, _volumeMetaData, MDName, MDValue);
			return;
		}
	}

	virtual void OnEndElement(const XML_Char*)
	{
		::std::string hiberarchyName = getHiberarchyName();
		if (0 == hiberarchyName.compare("/ContentStore/Volume") && !_currentVolume.empty())
		{
			::Ice::Identity identVol;
			identVol.name = _currentVolume;
			identVol.category = DBFILENAME_Volume;
			::TianShanIce::Storage::VolumeExPrx volume;

			try	{
				volume = IdentityToObjEnv(_store, VolumeEx, identVol);
			}
			catch (const ::Ice::Exception& ex)
			{
				_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "XmlImporter::OnEndElement() vol[%s] caught exception[%s]"), identVol.name.c_str(), ex.ice_name().c_str());
			}
			catch(...)
			{
				_store._log(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "XmlImporter::OnEndElement() vol[%s] caught unknown exception"), identVol.name.c_str());
			}

			if (!volume)
				_store._log(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "XmlImporter::OnEndElement() vol[%s] failed to open the specified volume"), identVol.name.c_str());
			else
			{
#pragma message ( __MSGLOC__ "TODO: update volume user metadata here")
			}

			_currentVolume = _currentContent = "";
			_volumeMetaData.clear();
			return;
		}
		
		if (0 == hiberarchyName.compare("/ContentStore/Content") && !_currentContent.empty())
		{
			::Ice::Identity identCont;
			identCont.name = _currentContent;
			identCont.category = DBFILENAME_Content;
			::TianShanIce::Storage::UnivContentPrx content;

			try	{
				content = IdentityToObjEnv(_store, UnivContent, identCont);
			}
			catch (const ::Ice::Exception& ex)
			{
				_store._log(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "XmlImporter::OnEndElement() content[%s] caught exception[%s]"), identCont.name.c_str(), ex.ice_name().c_str());
			}
			catch(...)
			{
				_store._log(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "XmlImporter::OnEndElement() content[%s] caught unknown exception"), identCont.name.c_str());
			}

			if (!content)
				_store._log(ZQ::common::Log::L_WARNING, CLOGFMT(ContentStore, "XmlImporter::OnEndElement() content[%s] failed to open the specified volume"), identCont.name.c_str());
			else
				content->setUserMetaData2(_contentMetaData);

			_currentContent = "";
			_contentMetaData.clear();
		}
	}

	virtual void OnCharData(const XML_Char*, int len) {}
	virtual void OnLogicalClose() {}
};


int ContentStoreImpl::importXml(const char* inputFileName, bool compressed, const char* volumeName)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "importXml(%s) compressed[%s] vol[%s]"), inputFileName?inputFileName:"null", compressed?"T":"F", volumeName?volumeName:"null");
	XmlImporter importer(*this);
	int size = 0;
	try {
		size = importer.parseFile(inputFileName, compressed, volumeName);
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "importXml(%s) processed, %d bytes"), inputFileName?inputFileName:"null", size);
	}
	catch(const ZQ::common::ExpatException& ex)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "importXml(%s) caught XML exception: %s"), inputFileName?inputFileName:"null", ex.getString());
	}
	catch(...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "importXml(%s) caught unknown exception"), inputFileName?inputFileName:"null");
	}

	return size;
}

void ContentStoreImpl::wakeupContent(const ::std::string& contentFullname, const ::Ice::Current& ic)
{
	::Ice::Identity identCont;
	identCont.name = contentFullname;
	identCont.category = DBFILENAME_Content;

	_log(ZQ::common::Log::L_INFO, CLOGFMT(ContentStore, "request to wakeup content (%s)"), contentFullname.c_str());
	_watchDog.watch(identCont, 0);
}

void ContentStoreImpl::updateContentOnSlave(const std::string& contentFullName)
{	
	if (!_storeAggregate)
	{
		return;
	}

	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "updateContentOnSlave() with content[%s]"), contentFullName.c_str());
	
	try 
	{
		(new UpdateContentOnSlaveCmd(*(const_cast<ContentStoreImpl*> (this)), contentFullName))->execute();
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "updateContentOnSlave() with content[%s] caught unknown exception"), contentFullName.c_str());
	}
}


void ContentStoreImpl::OnContentProvisionStarted( const ::Ice::Identity& identContent )
{
	updateContentOnSlave(identContent.name);
}

void ContentStoreImpl::OnContentProvisionStopped( const ::Ice::Identity& identContent )
{
	updateContentOnSlave(identContent.name);
}

void ContentStoreImpl::OnContentProvisionProgress( const ::Ice::Identity& identContent, ::Ice::Long processed, ::Ice::Long total )
{
	updateContentOnSlave(identContent.name);
}

void ContentStoreImpl::OnContentProvisionStreamable( const ::Ice::Identity& identContent )
{
	updateContentOnSlave(identContent.name);
}

TianShanIce::Storage::FolderInfos ContentStoreImpl::listSubFolders(const ::Ice::Current& c ) const
	//throw (::TianShanIce::NotImplemented)
{
	ZQTianShan::_IceThrow<TianShanIce::NotImplemented> (MOLOG, EXPFMT(ContentStore, 9999, "listSubFolders() not implemented yet"));
	TianShanIce::Storage::FolderInfos folderV;
	return folderV;
}

void ContentStoreImpl::initVolumeSubFolder(::TianShanIce::Storage::VolumeExPrx volPrx)
{
	TianShanIce::Storage::FolderInfos folderV;
	try
	{
		folderV = volPrx->listSubFolders();
		for (TianShanIce::Storage::FolderInfos::iterator itV = folderV.begin(); itV < folderV.end(); itV ++)
		{
			::TianShanIce::Storage::VolumeExPrx subF = 0;
			::Ice::Identity identF;
			identF.name = itV->name;;
			identF.category = DBFILENAME_Volume;

			try
			{
				subF = IdentityToObjEnv(*this, VolumeEx, identF);
			}
			catch (const ::Ice::Exception& ex)
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "initVolumeSubFolder() open folder proxy name[%s] caught exception[%s]"), itV->name.c_str(), ex.ice_name().c_str());
			}
			catch(...)
			{
				MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "initVolumeSubFolder() open folder proxy name[%s] caught unknown exception"), itV->name.c_str());
			}
			if(subF)
			{
				std::string strMP = subF->getMountPath();
				if (!ContentStoreImpl::createPathOfVolume(*this, strMP,identF.name))
				{
					MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "initVolumeSubFolder() failed to create the subfolder[%s] on the contentstore portal"), strMP.c_str());
					continue;
				}
				initVolumeSubFolder(subF);
			}
		}
	}
	catch(const ::TianShanIce::BaseException& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "initVolumeSubFolder() failed to listSubFolder, caught exception[%s] %s"), ex.ice_name().c_str(), ex.message.c_str());
	}
	catch(const ::Ice::Exception& ex)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "initVolumeSubFolder() failed to listSubFolder, caught exception[%s]"), ex.ice_name().c_str());
	}
	catch(...)
	{
		MOLOG(ZQ::common::Log::L_ERROR, CLOGFMT(ContentStore, "initVolumeSubFolder() failed to listSubFolder, caught unknown exception"));
	}

}

int ContentStoreImpl::resortNodeQueuePerContentReplica(ContentStoreImpl::NodeReplicaQueue& in, ContentStoreImpl::NodeReplicaQueue2& out, ContentImpl& content)
{
	std::string netIdRecentUpdate;
	std::string netIdMastReplica;
	std::string contentName = content.ident.name.c_str();

	::TianShanIce::Properties metaDatas = content.getMetaData(Ice::Current());

	::TianShanIce::Properties::const_iterator it = metaDatas.find(METADATA_MasterReplicaNetId);
	if ( it!=metaDatas.end())
		netIdMastReplica = it->second;

	it = metaDatas.find(METADATA_RecentUpdateNetId);
	if ( it!=metaDatas.end() )
		netIdRecentUpdate = it->second;

	MOLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentStore, "resortNodeQueuePerContentReplica() content[%s] resorting node replica queue per MastReplicaNetId[%s] and ReplicaOfRecentUpdate[%s]"), content.ident.name.c_str(), netIdMastReplica.c_str(), netIdRecentUpdate.c_str());

	NodeReplicaQueue2 norm, behind;
	out = norm; // reset out to empty queue
	for (; !in.empty(); in.pop())
	{
		const NodeReplica& replica = in.top();

		if (std::string::npos != netIdRecentUpdate.find(replica.replicaData.replicaId))
			behind.push(replica);
		else if (std::string::npos != netIdMastReplica.find(replica.replicaData.replicaId))
			out.push(replica);
		else norm.push(replica);
	}

	for (; !norm.empty(); norm.pop())
		out.push(norm.front());

	for (; !behind.empty(); behind.pop())
		out.push(behind.front());

	return out.size();
}

}} // namespace
