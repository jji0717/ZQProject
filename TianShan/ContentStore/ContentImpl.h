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
// Ident : $Id: ContentStoreImpl.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/ContentStore/ContentImpl.h $
// 
// 30    1/11/16 5:42p Dejian.fei
// 
// 29    1/08/14 4:46p Hui.shao
// 
// 28    1/02/14 9:28a Zonghuan.xiao
// 
// 27    12/30/13 1:42p Zonghuan.xiao
// 
// 26    12/30/13 1:29p Hui.shao
// 
// 24    12/11/13 7:43p Hui.shao
// added hashed folder calculation
// 
// 23    3/08/13 2:44p Hui.shao
// corrected KB at cache size
// 
// 22    3/06/13 4:47p Hui.shao
// export the indexing from mountpath to volname thru ICE
// 
// 21    3/06/13 4:32p Hui.shao
// added an index from mounted path back to volumename
// 
// 20    2/06/13 12:15p Hui.shao
// 
// 19    4/27/12 5:57p Hui.shao
// added Content::getProvisionSession()
// 
// 18    3/26/12 4:01p Hui.shao
// added openFolderEx() to create subfolder recursively
// 
// 17    12/30/11 2:28p Hui.shao
// 
// 16    12/29/11 8:26p Hui.shao
// 
// 15    11/30/11 9:28p Hui.shao
// merged playtime interval limitation from v1.15
// 
// 15    11/29/11 4:36p Hui.shao
// ticket#9981, JIRA ACE-9345, do not call Vstrm
// VstrmClassLoadFullAssetInfoEx() too frequently
// 
// 14    6/23/11 10:26a Hongquan.zhang
// 
// 13    6/22/11 7:33p Build
// 
// 12    5/26/11 6:09p Hui.shao
// 
// 11    5/26/11 5:51p Hongquan.zhang
// 
// 10    5/26/11 5:42p Hui.shao
// 
// 9     5/26/11 5:36p Hui.shao
// comment the "throw" declarations
// 
// 8     5/04/11 10:25a Fei.huang
// 
// 7     3/09/11 5:10p Hui.shao
// added populate subfolder per file event
// 
// 6     2/09/11 4:04p Hui.shao
// added a flag STOREFLAG_checkFSonOpenContentByFullName to borrow
// Folder::openContent() to populate content if exist on FS
// 
// 5     10-12-08 12:23 Hui.shao
// enable timer() persistently for volume, and added volume::setMetaData()
// 
// 4     10-11-25 18:45 Hui.shao
// added free-space monitoring configuration vars
// 
// 3     10-11-17 18:32 Hui.shao
// 
// 2     10-11-17 18:08 Hui.shao
// include Volume under the watchdog to yield fs-sync at the begining of
// service start
// 
// 72    10-04-28 13:10 Haoyuan.lu
// 
// 71    10-04-26 15:17 Hui.shao
// added cached content list
// 
// 70    10-04-23 11:58 Hongquan.zhang
// add edge mode
// 
// 69    10-03-05 15:40 Fei.huang
// * added NoResource exception to exception specifications of provision
// 
// 68    10-03-03 15:54 Xia.chen
// add volume parameter for createpathofvolume function
// 
// 67    10-02-05 15:59 Yixin.tian
// 
// 66    10-01-26 14:46 Yixin.tian
// add ContentStoreImpl interface listSubFolders()
// 
// 65    10-01-25 17:26 Jie.zhang
// add listSubFolder to folder
// 
// 64    10-01-07 15:45 Hui.shao
// set DB cache size to 16MB
// 
// 63    09-12-22 18:25 Hui.shao
// initialize DB_CONFIG.dat if it is not available
// 
// 62    09-12-22 17:46 Hui.shao
// abstracted folder from volume
// 
// 61    09-12-22 14:06 Jie.zhang
// merge from TianShan1.10
// 
// 60    09-09-11 11:17 Hui.shao
// made the auto sync with filesystem optional
// 
// 59    09-09-02 14:42 Hui.shao
// added flag to mask file events to handle
// 
// 58    09-07-29 13:49 Jie.zhang
// merge from TianShan1.10
// 
// 57    09-07-24 15:01 Xia.chen
// change the third parameter from contentName to contentKey for
// getExportUrl
// 
// 56    09-06-26 16:48 Fei.huang
// 
// 55    09-06-12 11:18 Hui.shao
// 
// 54    09-06-12 10:39 Hui.shao
// adjusted some default timeout values
// 
// 53    09-06-11 18:54 Hui.shao
// merged back _checkResidentialInFileDeleteEvent from 1.10
// 
// 50    09-06-05 10:54 Jie.zhang
// merge from 1.10
// 
// 50    09-05-13 21:19 Jie.zhang
// adjustProvisionSchedule() implementation added
// 
// 49    09-05-08 16:00 Jie.zhang
// add SaveSizeTrigger & SavePeriod to contentstore property
// 
// 48    09-03-09 15:19 Hui.shao
// 
// 47    09-02-26 11:05 Hongquan.zhang
// 
// 46    09-02-20 18:40 Hongquan.zhang
// 
// 45    09-02-20 17:43 Hongquan.zhang
// 
// 44    09-02-05 13:53 Hui.shao
// added the event entires from the ContentStoreLib to the impls derived
// from the base ContentStoreImpl
// 
// 43    09-01-23 15:44 Hongquan.zhang
// 
// 42    09-01-20 16:11 Hongquan.zhang
// 
// 41    09-01-14 15:10 Hongquan.zhang
// 
// 40    08-12-26 18:08 Fei.huang
// 
// 39    08-12-25 18:36 Jie.zhang
// 
// 38    08-12-25 17:32 Hui.shao
// 
// 37    08-12-24 11:07 Jie.zhang
// add a define for default volume naming
// 
// 36    08-12-23 15:20 Hui.shao
// added importDB()
// 
// 35    08-12-18 16:38 Jie.zhang
// add volume info cache
// 
// 34    08-12-17 16:48 Hui.shao
// quit the program more quickly from sync procedure
//
// 33    08-12-17 15:19 Jie.zhang
// 
// 32    08-12-16 11:57 Hui.shao
// 
// 31    08-12-16 11:25 Hui.shao
// 
// 30    08-12-11 12:28 Hui.shao
// report replica to the master
// 
// 29    08-12-10 12:57 Yixin.tian
// 
// 28    08-11-24 13:56 Jie.zhang
// add evictor size configuration
// 
// 27    08-11-24 12:45 Hui.shao
// force to populate attr if streamable and in provisioning
// 
// 26    08-11-24 12:29 Jie.zhang
// add a parameter on checkResidencialStatus
// 
// 25    08-11-18 17:38 Yixin.tian
// 
// 24    08-11-18 14:00 Hui.shao
// fix the setMetaData()s
// 
// 23    08-11-18 11:39 Yixin.tian
// modify can compile for Linux OS
// 
// 22    08-11-15 16:31 Hui.shao
// 
// 21    08-11-15 15:20 Hui.shao
// 
// 20    08-11-15 14:01 Hui.shao
// added check file residential status checking
// 
// 19    08-11-13 11:09 Jie.zhang
// add notifyReplicasChanged
// 
// 17    08-11-10 15:06 Jie.zhang
// 
// 16    08-11-10 11:48 Hui.shao
// pre-fill the some attributes when query for metadata, added OnIdle()
// entry
// 
// 15    08-11-10 11:37 Jie.zhang
// add clearReplica for test
// 
// 14    08-11-07 13:52 Hui.shao
// added _volume() for portal to access without locking the content obj
// 
// 13    08-11-07 11:01 Jie.zhang
// add common log define to unify log using style
// 
// 12    08-11-06 18:19 Hui.shao
// added a Content::isInUse() query for clusterCS to query node
// 
// 11    08-11-06 13:23 Hui.shao
// added provision event handling
// 
// 10    08-11-03 11:41 Jie.zhang
// 
// 9     08-11-03 11:35 Hui.shao
// splitted CS impl to ContentStoreImpl.cpp
// 
// 8     08-11-03 11:16 Jie.zhang
// add contentName to some interface
// 
// 7     08-10-28 17:57 Hui.shao
// test the last write if it is worthy to populate attributes
// 
// 6     08-10-23 18:31 Hui.shao
// moved watchdog out to common
// 
// 5     08-10-14 11:33 Hui.shao
// 
// 4     08-10-07 19:55 Hui.shao
// added volume layer
// 
// 2     08-08-14 18:01 Hui.shao
// 
// 1     08-08-14 15:13 Hui.shao
// merged from 1.7.10
// 
// 12    08-08-13 12:42 Hui.shao
// 
// 11    08-08-11 18:42 Hui.shao
// added store replica handling
// 
// 10    08-08-04 18:30 Hui.shao
// 
// 9     08-07-31 18:43 Hui.shao
// restrict on state for provision-related operation
// 
// 8     08-07-31 17:20 Hui.shao
// added the portail enties for provisioning
// 
// 7     08-07-29 12:16 Hui.shao
// added event log as sentry's input
// 
// 6     08-07-21 14:01 Hui.shao
// 
// 5     08-07-21 11:51 Hui.shao
// check in the works of last weekend
// 
// 4     08-07-18 18:52 Hui.shao
//
// 1     08-07-10 19:29 Hui.shao
// ===========================================================================

#ifndef __ZQTianShan_ContentImpl_H__
#define __ZQTianShan_ContentImpl_H__

#include "../common/TianShanDefines.h"
#include "ZQ_common_conf.h"
#include "Locks.h"
#include "Log.h"
#include "NativeThreadPool.h"

#include "VolumeInfoCache.h"

#include "ContentStore.h"
#include "ContentFactory.h"
#include "FileOfVol.h"
#include "ChildVolume.h"

#include <TsContentProv.h>

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>

#include "ContentSysMD.h"

#ifndef _DEBUG
#  define MAX_NOT_PROVISIONED_TIMEOUT   (4*60*60*1000) // 4 hours
#  define MAX_IDLE_PROVISIONING_TIMEOUT (4*24*60*60*1000) // 4 days
#  define MIN_UPDATE_INTERVAL           (30*1000) // 30 sec
#  define UNATTENDED_TIMEOUT			(10*60*1000) // 10 min
#  define OUTSERVICE_TIMEOUT            (1*1000) // 1 sec
#else
#  define MAX_NOT_PROVISIONED_TIMEOUT   (10*60*1000) // 10 min
#  define MAX_IDLE_PROVISIONING_TIMEOUT (2*24*60*60*1000) // 2 days
#  define MIN_UPDATE_INTERVAL           (20*1000) // 20 sec
#  define UNATTENDED_TIMEOUT			(MIN_UPDATE_INTERVAL *4) // 80 sec
#  define OUTSERVICE_TIMEOUT            (1*1000) // 1 sec
#endif //! _DEBUG

#ifndef MAX_CONTENTS
#  define MAX_CONTENTS		             (50000) //50K
#endif // MAX_CONTENTS

#ifndef ContentDB_CACHESIZE_KB
#  define ContentDB_CACHESIZE_KB	     (160*1024) //160MB
#endif // ContentDB_CACHESIZE_KB

//
// define the default volume string here
//
#define DEFAULT_VOLUME_STRING			"$"

namespace ZQTianShan {
namespace ContentStore {

class ContentStoreImpl;

#define ILLEGAL_MOUNTNAME_CHARS " ~@^&*?<>[]{}()|`:'\",+\\\t\n\r" // except LOGIC_FNSEPS "$#!"
#define ILLEGAL_NAME_CHARS ILLEGAL_MOUNTNAME_CHARS "#$!" LOGIC_FNSEPS
#define THREDHOLD_FORCESYNC 50000 // count of file events ever received to kick off a volume sync

#define DECLARE_CONTAINER(_OBJ)	Freeze::EvictorPtr _e##_OBJ
#define DECLARE_INDEX(_IDX)	TianShanIce::Storage::_IDX##Ptr _idx##_IDX

#define CATEGORY_ContentStore "ContentStore"						///< the replica category for contentstore

#define METADATA_RecentUpdateNetId        SYS_PROP(RecentUpdateNetId)
#define METADATA_MasterReplicaNetId       SYS_PROP(MasterReplicaNetId)
#define METADATA_MasterReplicaEndpoint    SYS_PROP(MasterReplicaEndpoint)

// -----------------------------
// module VolumeImpl
// -----------------------------
class VolumeImpl : public ::TianShanIce::Storage::VolumeEx, virtual public ICEAbstractMutexRLock
{
public:

	typedef ::IceInternal::Handle< VolumeImpl > Ptr;

	VolumeImpl(ContentStoreImpl& store);
	virtual ~VolumeImpl();

protected: // impl of VolumeEx

    virtual ::Ice::Identity getIdent(const ::Ice::Current& c) const;
    virtual ::std::string getMountPath(const ::Ice::Current& c) const;
    virtual ::TianShanIce::Storage::VolumeInfo getInfo(const ::Ice::Current& c) const;

	virtual ::TianShanIce::Storage::UnivContentPrx createContent(const ::Ice::Identity& identContent, bool fromFsOrphan, const ::std::string& stampLastFileWrite, const ::Ice::Current& c);
    virtual bool deleteContent(const ::Ice::Identity& identContent, const ::Ice::Current& c);
    virtual void syncWithFileSystem(const ::Ice::Current& c);
    virtual void setMetaData(const ::TianShanIce::Properties& metaData, const ::Ice::Current& c);

    virtual void cachedListContents_async(const ::TianShanIce::Storage::AMD_VolumeEx_cachedListContentsPtr& amdCb, ::Ice::Int timeout, const ::Ice::Current& c) const;

protected: // impl of Volume

    virtual ::std::string getName(const ::Ice::Current& c) const;
	virtual ::std::string getVolumeName(const ::Ice::Current& c) const;
    virtual void getCapacity(::Ice::Long& freeMB, ::Ice::Long& totalMB, const ::Ice::Current& c);
//		throw (::TianShanIce::InvalidStateOfArt, ::TianShanIce::ServerError);

    virtual ::TianShanIce::StrValues listContent(const ::std::string& condition, const ::Ice::Current& c) const;
//		throw (::TianShanIce::InvalidStateOfArt);

    virtual ::TianShanIce::Storage::ContentPrx openContent(const ::std::string& name, const ::std::string& destinationContentType, bool createIfNotExist, const ::Ice::Current& c);
//		throw (::TianShanIce::InvalidParameter, ::TianShanIce::InvalidStateOfArt, ::TianShanIce::ServerError);

	virtual void listContents_async(const ::TianShanIce::Storage::AMD_Folder_listContentsPtr& amdCB, const ::TianShanIce::StrValues& metaDataNames, const ::std::string& startName, ::Ice::Int maxCount, const ::Ice::Current& c) const;
//		throw (::TianShanIce::ServerError);

    virtual ::TianShanIce::Storage::FolderPrx openSubFolder(const ::std::string& subname, bool createIfNotExist, ::Ice::Long quotaMB, const ::Ice::Current& c);
    virtual ::TianShanIce::Storage::FolderPrx parent(const ::Ice::Current& c) const;

    virtual void destroy(const ::Ice::Current& c);
//		throw (::TianShanIce::NotSupported, ::TianShanIce::InvalidStateOfArt);

	virtual ::TianShanIce::Storage::FolderInfos listSubFolders(const ::Ice::Current& c = ::Ice::Current()) const;

protected: // impl of TimeoutObj
	virtual void OnTimer(const ::Ice::Current& c);


protected:

	friend class ContentStoreImpl;
	ContentStoreImpl& _store;
//	bool       _bSyncWithFSOnPending;
};

// -----------------------------
// class ContentImpl
// -----------------------------
class ContentImpl : public TianShanIce::Storage::UnivContent, virtual public ICEAbstractMutexRLock
{
	friend class ContentStoreImpl;
	friend class ContentStateBase;

public:
	ContentImpl(ContentStoreImpl& store);
	ContentImpl(ContentImpl& old, const std::string& newName); // the copyer
	virtual ~ContentImpl() {}

	::std::string _name() const;
	::std::string _mainFilePathname() const;
	::TianShanIce::Storage::VolumeExPrx _volume() const;
	void _cancelProvision();

	typedef ::IceInternal::Handle< ContentImpl > Ptr;
    
public:	// impl of TimeoutObj
    virtual void OnTimer(const ::Ice::Current& c);

public:	// impls of UnivContent

    virtual bool isDirty(const ::Ice::Current& c) const;
    virtual ::Ice::Identity getIdent(const ::Ice::Current& c) const;
    virtual void populateAttrsFromFilesystem(const ::Ice::Current& c);
    virtual void setMetaData(const ::TianShanIce::Properties& metaData, const ::Ice::Current& c);
    virtual void OnRestore(const ::std::string& stampLastFileWrite, const ::Ice::Current& c);
    virtual void OnFileModified(const ::Ice::Current& c);
	virtual void OnFileCreated(const ::std::string&, const ::Ice::Current& = ::Ice::Current());
    virtual void OnFileRenamed(const ::std::string& newName, const ::Ice::Current& c);
    virtual ::TianShanIce::Storage::ContentState enterState(::TianShanIce::Storage::ContentState targetState, const ::Ice::Current& c);
    virtual ::Ice::Long checkResidentialStatus(::Ice::Long flagsToTest, const ::Ice::Current& c) const;
//  virtual bool isInUse(const ::Ice::Current& c);
	virtual bool isCorrupt(const ::Ice::Current& c) const;

    virtual ::std::string getMainFilePathname(const ::Ice::Current& c) const;

protected:	// impls of Content

    virtual ::TianShanIce::Storage::ContentStorePrx getStore(const ::Ice::Current& c) const;
    virtual ::std::string getName(const ::Ice::Current& c) const;
    virtual ::TianShanIce::Storage::ContentState getState(const ::Ice::Current& c) const;

	virtual ::TianShanIce::Properties getMetaData(const ::Ice::Current& c) const;
    virtual void setUserMetaData(const ::std::string& key, const ::std::string& value, const ::Ice::Current& c);
    virtual void setUserMetaData2(const ::TianShanIce::Properties& metadata, const ::Ice::Current& c);
    
	virtual bool isProvisioned(const ::Ice::Current& c) const;
	virtual ::std::string getProvisionTime(const ::Ice::Current& c) const;

    virtual void destroy(const ::Ice::Current& c);
		//throw (::TianShanIce::NotImplemented, ::TianShanIce::InvalidStateOfArt);
    
	virtual void destroy2(bool mandatory, const ::Ice::Current& c);
		//throw (::TianShanIce::NotImplemented, ::TianShanIce::InvalidStateOfArt);

    virtual ::std::string getLocaltype(const ::Ice::Current& c) const;

	virtual ::std::string getSubtype(const ::Ice::Current& c) ;

	virtual ::Ice::Float getFramerate(const ::Ice::Current& c) const;
		//throw (::TianShanIce::NotImplemented, ::TianShanIce::InvalidStateOfArt);

	virtual void getResolution(::Ice::Int& pixelHorizontal, ::Ice::Int& pixelVertical, const ::Ice::Current& c) const;
		//throw (::TianShanIce::NotImplemented, ::TianShanIce::InvalidStateOfArt);

	virtual ::Ice::Long getFilesize(const ::Ice::Current& c) const;
    virtual ::Ice::Long getSupportFileSize(const ::Ice::Current& c) const;
    virtual ::Ice::Long getPlayTime(const ::Ice::Current& c) const;
    virtual ::Ice::Long getPlayTimeEx(const ::Ice::Current& c) const;
    virtual ::Ice::Int getBitRate(const ::Ice::Current& c) const;
    virtual ::std::string getMD5Checksum(const ::Ice::Current& c) const;
    virtual ::TianShanIce::Storage::TrickSpeedCollection getTrickSpeedCollection(const ::Ice::Current& c) const;

    virtual ::std::string getSourceUrl(const ::Ice::Current& c) const;
    
	virtual ::std::string getExportURL(const ::std::string& transferProtocol, ::Ice::Int transferBitrate, ::Ice::Int& ttl, ::TianShanIce::Properties& exppro, const ::Ice::Current& c = ::Ice::Current());

    virtual void provision(const ::std::string& sourceUrl,
						const ::std::string& sourceContentType,
						bool overwrite,
						const ::std::string& startTimeUTC,
						const ::std::string& stopTimeUTC,
						::Ice::Int maxTransferBitrate,
						const ::Ice::Current& c);
		//				throw (::TianShanIce::InvalidParameter, TianShanIce::Storage::NoResourceException, 
        //                       ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt);

    virtual ::std::string provisionPassive(const ::std::string& sourceContentType,
						bool overwrite,
						const ::std::string& startTimeUTC,
						const ::std::string& stopTimeUTC,
						::Ice::Int maxTransferBitrate,
						const ::Ice::Current& c);
		//throw (::TianShanIce::InvalidParameter, TianShanIce::Storage::NoResourceException, 
        //       ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt);

	virtual void adjustProvisionSchedule(const ::std::string& startTimeUTC, const ::std::string& stopTimeUTC, const ::Ice::Current&);
	//	throw (::TianShanIce::InvalidStateOfArt);

    virtual void cancelProvision(const ::Ice::Current& c);
	//	throw (::TianShanIce::InvalidStateOfArt, ::TianShanIce::NotImplemented);

	::TianShanIce::ContentProvision::ProvisionSessionPrx getProvisionSession(const ::Ice::Current& c) const;

    virtual ::TianShanIce::Storage::VolumePrx theVolume(const ::Ice::Current& c) const;

	bool populateAttrDirect();

	friend class ClusterCS; // the impl of ClusterCS may need to query metadata directly
	friend class NativeCS;// the impl of NativeCS may need to query metadata directly
protected:

	ContentStoreImpl& _store;
};

#ifndef FLAG
#define FLAG(_FLAG) (1 << _FLAG)
#endif

// -----------------------------
// module ContentStoreImpl
// -----------------------------
class ContentStoreImpl : public ::TianShanIce::Storage::ContentStoreEx
{
	friend class VolumeImpl;
	friend class VolumeFactory;

	friend class ContentImpl;
	friend class ContentFactory;
	friend class ContentStateBase;

	friend class CacheStoreImpl;

	friend class BaseCmd;
	friend class FileEventCmd;
	friend class ListVolumesCmd;
	friend class UpdateReplicaCmd;
	friend class ContentWatchDog;
	friend class ListContentsCmd;
	friend class ReportReplicaCmd;
	friend class ForwardEventToMasterCmd;
	friend class UpdateContentOnSlaveCmd;
	friend class CachedListContentsCmd;

public:

	typedef ::IceInternal::Handle< ContentStoreImpl > Ptr;

	//NOTE: a later _adapter->activate() must be called after this base constructor
	ContentStoreImpl(ZQ::common::Log& log, ZQ::common::Log& eventlog, 
				ZQ::common::NativeThreadPool& threadPool,
				ZQADAPTER_DECLTYPE& adapter,
				const char* databasePath =NULL);
	
	virtual ~ContentStoreImpl();

	static ::Ice::Identity toContentIdent(const std::string& name);
	static ::Ice::Identity toVolumeIdent(const std::string& name);
	static int chopPathname(const std::string& fullname, std::string& volumeName, std::string& folderName, std::string& contentName);

	void setFlags(uint32 flags=0);

	::TianShanIce::Storage::ContentStorePrx proxy(bool collocationOptim =false);

	virtual ::TianShanIce::Replicas exportStoreReplicas();	///< include the self replica information & would check expiration

	virtual ::TianShanIce::Storage::ContentPrx openContentByFullname(const ::std::string& fullname, const ::Ice::Current&) const;

	virtual void getStoreReplicas(::TianShanIce::Replicas& replicas);	///< just get the store replica, no self replica
	virtual void updateStoreReplicas(const ::TianShanIce::Replicas& replicas);
	virtual void clearStoreReplicas();
	virtual bool subscribeStoreReplica(const std::string proxyStrToSubscriber, bool masterStoreReplica=false);

	virtual ::TianShanIce::Storage::VolumePrx mountStoreVolume(const ::std::string& name, const ::std::string& mountPath, const bool defaultVolume=false);

	virtual int dumpDB(const char* dumpFilename, bool compress=false, const char* volumeName=NULL);
	int importXml(const char* inputFileName, bool compress=false, const char* volumeName=NULL);

    virtual void OnIdle();

	void enableInterface(bool enable=true);

	bool initializeContentStore();
	void unInitializeContentStore();

	virtual void wakeupContent(const ::std::string&, const ::Ice::Current& = ::Ice::Current());
protected:

	void openDB(const char* databasePath=NULL);
	void closeDB();

	void updateContentOnSlave(const std::string& contentFullName);

public:
	ZQ::common::Log&		_log;
	ZQ::common::Log&		_eventlog;
	ZQADAPTER_DECLTYPE&		_adapter;

	VolumeInfoCache			_volumeInfoCache;
	ZQTianShan::TimerWatchDog	_watchDog;

protected:

	ZQ::common::NativeThreadPool& _thpool;

	Ice::Identity			_localId;
	::TianShanIce::Storage::ContentStorePrx _thisPrx;
	::TianShanIce::Storage::VolumeExPrx _defaultVol;
	ContentFactory::Ptr		_factory;

	std::string				_programRootPath;
	std::string				_dbPath;
	::TianShanIce::State	_serviceState;

	::TianShanIce::Properties _idxMountPathToVolumeName;

	DECLARE_INDEX(FileOfVol);
	DECLARE_CONTAINER(Content);

	DECLARE_INDEX(ChildVolume);
	DECLARE_CONTAINER(Volume);

	typedef struct _NodeReplica
	{
		::TianShanIce::Replica replicaData;
		::Ice::Long stampUpdated;  ///< stamp that this subscriber has been recently notified about the replica

		struct greater : std::binary_function<_NodeReplica, _NodeReplica, bool>
		{
			bool operator()(const _NodeReplica _X, const _NodeReplica _Y) const
			{
				int diff = _X.replicaData.priority - _Y.replicaData.priority;

				if (0 != diff)
					return (diff >0);

				return (_X.stampUpdated > _Y.stampUpdated);
			}
		};

	} NodeReplica;

	typedef std::map <std::string, NodeReplica > ReplicaMap;
	ReplicaMap _storeReplicas;
	ZQ::common::Mutex _lockStoreReplicas;

	typedef struct _ReplicaSubscriberInfo
	{
		::std::string proxyStr;
		::TianShanIce::ReplicaSubscriberPrx subscriber;
		::Ice::Long lastUpdated;
		long timeout;
	} ReplicaSubscriberInfo;

	typedef ::std::map <std::string, ReplicaSubscriberInfo > ReplicaSubscriberMap;
	ReplicaSubscriberMap _replicaSubscriberMap;
	std::string _prxstrMasterReplica;
	ZQ::common::Mutex _lockReplicaSubscriberMap;

	uint32 _cFileEvent;

public:
	Ice::Long               _stampStarted;

	// configuration, should be moved to store later
	std::string				_netId;
	std::string				_storeType;
	long					_streamableLength; // streamable playtime in msec
//	std::string				_homeDir; // the logical home directory of the ContentStore

	// configurable timeouts
	uint32                  _timeoutNotProvisioned;
	uint32                  _timeoutIdleProvisioning;
	uint32					_timeoutOfPlaytimeAtProvisioning;

	uint32					_fileEventFlags;
	bool					_autoFileSystemSync;
	uint32					_enableInServiceCheck;		//set to 0 to disable "check missing file" when in "InService" state, other to enable

	// ContentStore wide flags
#define   STOREFLAG_checkFSonOpenContentByFullName   FLAG(0)
#define   STOREFLAG_populateSubfolders               FLAG(1)
	uint32				    _storeFlags;

	uint32					_storeAggregate;		//default is 0, set to 1 if it's cluster|contentstore aggregate		
	// configuration should include a line
	// <ContentStore ...>
	//     ...
	//		<StoreReplica groupId="" replicaId="" replicaPriority="" timeout="" />
	//			<MasterReplica endpoint="" />
	//			<Subscriber endpoint="" />
	//		</StoreReplia>
	//      ...
	// </ContentStore>
	bool					_checkResidentialInFileDeleteEvent;
	std::string				_replicaGroupId;
	std::string             _replicaId;
	uint8					_replicaPriority;
	int32					_replicaTimeout; // in msec
	int32					_contentEvictorSize;
	int32					_volumeEvictorSize;

	// configuration may include optional attributes to enable monitoring of the free space
	// <ContentStore ... warningFreeSpacePercent="10" stepFreeSpacePercent="1" >
	//     ...
	// </ContentStore>
	uint8                   _warningFreeSpacePercent;
	uint8                   _stepFreeSpacePercent;

	int32					_contentSavePeriod;
	int32					_contentSaveSizeTrigger;

	// configuration may include an optional line
	// <ContentStore ...>
	//     ...
	//		<CacheStore level="{0 ~ 255}" exposeStreamService="{1|0}" />
	// </ContentStore>
	uint8					_cacheLevel;
	bool					_exposeStreamService;
	bool					_cacheable;  // should be true if the store has configuration <CacheStore>
	bool					_edgeMode;	 // content will be cached from CDN if this is true

	// <ContentStore ...>
	//     ...
	//		<HashedFolder expression="$CUT(${ContentName}, 0, 10)/$CUT(${ContentName}, 10, 4)" />
	// </ContentStore>
	std::string             _hashFolderExpsn; // default empty means not effective

public: // impls of ContentStoreEx

	virtual void OnFileEvent(::TianShanIce::Storage::FileEvent event, const ::std::string& name, const ::TianShanIce::Properties& params, const ::Ice::Current& c);
    virtual void OnProvisionEvent(::TianShanIce::Storage::ProvisionEvent event, const ::std::string& storeNetId, const ::std::string& volumeName, const ::std::string& contentName, const ::TianShanIce::Properties& params, const ::Ice::Current& c);

	// impls of ReplicaQuery and ReplicaSubscriber
    virtual void queryReplicas_async(const ::TianShanIce::AMD_ReplicaQuery_queryReplicasPtr& amdCB, const ::std::string& category, const ::std::string& groupId, bool localOnly, const ::Ice::Current& c);
    virtual void updateReplica_async(const ::TianShanIce::AMD_ReplicaSubscriber_updateReplicaPtr& amdCB, const ::TianShanIce::Replicas& stores, const ::Ice::Current&  c);
    virtual void OnTimer(const ::Ice::Current& c);
    virtual void enableAutoFileSystemSync(bool enable, const ::Ice::Current& c);

    virtual ::TianShanIce::Storage::FolderPrx openFolderEx(const ::std::string& fullFolderName, bool createIfNotExist, ::Ice::Long quotaMB, const ::Ice::Current& c);
    virtual ::std::string mountPathToVolName(const ::std::string& mountPath, const ::Ice::Current& c);
    virtual ::std::string hashFolderNameByContentName(const ::std::string& contentName, const ::Ice::Current& c);

protected: // impls of ContentStore

    virtual ::std::string getNetId(const ::Ice::Current& c) const;
    virtual ::std::string type(const ::Ice::Current& c) const;
    virtual bool isValid(const ::Ice::Current& c) const;

	virtual ::TianShanIce::Storage::VolumePrx openVolume(const ::std::string& name, const ::Ice::Current& c) const;
	virtual void listVolumes_async(const ::TianShanIce::Storage::AMD_ContentStore_listVolumesPtr& amdCB, const ::std::string& listFrom, bool includingVirtual, const ::Ice::Current& c) const;
    

    virtual ::Ice::Byte getCacheLevel(const ::Ice::Current& c) const;
    virtual ::TianShanIce::Replicas getStreamServices(const ::Ice::Current& c) const;

/*
protected: // impl of ProvisionSessionBind
	virtual void OnProvisionStateChanged(const ::TianShanIce::ContentProvision::ProvisionContentKey& content, ::Ice::Long timeStamp, ::TianShanIce::ContentProvision::ProvisionState prevState, ::TianShanIce::ContentProvision::ProvisionState currentState, const ::TianShanIce::Properties& params, const ::Ice::Current& c);
    virtual void OnProvisionProgress(const ::TianShanIce::ContentProvision::ProvisionContentKey& content, ::Ice::Long timeStamp, ::Ice::Long processed, ::Ice::Long total, const ::TianShanIce::Properties& params, const ::Ice::Current& c);
    virtual void OnProvisionStarted(const ::TianShanIce::ContentProvision::ProvisionContentKey& content, ::Ice::Long timeStamp, const ::TianShanIce::Properties& params, const ::Ice::Current& c);
    virtual void OnProvisionStopped(const ::TianShanIce::ContentProvision::ProvisionContentKey& content, ::Ice::Long timeStamp, bool errorOccurred, const ::TianShanIce::Properties& params, const ::Ice::Current& c);
    virtual void OnProvisionStreamable(const ::TianShanIce::ContentProvision::ProvisionContentKey& content, ::Ice::Long timeStamp, bool streamable, const ::TianShanIce::Properties& params, const ::Ice::Current& c);
    virtual void OnProvisionDestroyed(const ::TianShanIce::ContentProvision::ProvisionContentKey& content, ::Ice::Long timeStamp, const ::TianShanIce::Properties& params, const ::Ice::Current& c);
*/

protected: // impl of Volume

    virtual ::std::string getName(const ::Ice::Current& c) const;
	virtual ::std::string getVolumeName(const ::Ice::Current& c) const;
    virtual void getCapacity(::Ice::Long& freeMB, ::Ice::Long& totalMB, const ::Ice::Current& c);
//		throw (::TianShanIce::InvalidStateOfArt, ::TianShanIce::ServerError);

    virtual ::TianShanIce::StrValues listContent(const ::std::string& condition, const ::Ice::Current& c) const;
//		throw (::TianShanIce::InvalidStateOfArt);

    virtual ::TianShanIce::Storage::ContentPrx openContent(const ::std::string& name, const ::std::string& destinationContentType, bool createIfNotExist, const ::Ice::Current& c);
//		throw (::TianShanIce::InvalidParameter, ::TianShanIce::InvalidStateOfArt, ::TianShanIce::ServerError);

	virtual void listContents_async(const ::TianShanIce::Storage::AMD_Folder_listContentsPtr& amdCB, const ::TianShanIce::StrValues& metaDataNames, const ::std::string& startName, ::Ice::Int maxCount, const ::Ice::Current& c) const;
//		throw (::TianShanIce::ServerError);

    virtual ::TianShanIce::Storage::FolderPrx openSubFolder(const ::std::string& subname, bool createIfNotExist, ::Ice::Long quotaMB, const ::Ice::Current& c);
    virtual ::TianShanIce::Storage::FolderPrx parent(const ::Ice::Current& c) const;

    virtual void destroy(const ::Ice::Current& c);
//		throw (::TianShanIce::NotSupported, ::TianShanIce::InvalidStateOfArt);
	
	virtual ::TianShanIce::Storage::FolderInfos listSubFolders(const ::Ice::Current& c = ::Ice::Current()) const;
//		throw (::TianShanIce::NotImplemented);

	virtual void initVolumeSubFolder(::TianShanIce::Storage::VolumeExPrx volPrx);
protected: // impls of BaseService

	virtual ::std::string getAdminUri(const ::Ice::Current& c);
    virtual ::TianShanIce::State getState(const ::Ice::Current& c);

#ifndef _DEBUG
private:
#else
public:
#endif // _DEBUG
	friend class ClusterCS; // the impl of ClusterCS may need to access the NodeReplica data structures directly
	typedef std::priority_queue <NodeReplica, std::vector<NodeReplica>, NodeReplica::greater > NodeReplicaQueue;
	typedef std::queue <NodeReplica> NodeReplicaQueue2;

	int buildNodeReplicaQueue(NodeReplicaQueue& nodequeue);
	int resortNodeQueuePerContentReplica(ContentStoreImpl::NodeReplicaQueue& in, ContentStoreImpl::NodeReplicaQueue2& out, ContentImpl& content);

	::TianShanIce::Storage::UnivContentPrx openContentByProvisionKey(const ::TianShanIce::ContentProvision::ProvisionContentKey& provisionKey);


protected: // defines the portal entires

	// ContentStore Portal Entries
	// -----------------------------

	void* _ctxPortal;  ///< the context for the portal layer to customize

	/// Portal entry, called during ContentStore initialization, for the portal implementation to initialize.
	/// If necessary, the portal should initialize the portal context _ctxPortal
	///@param[in] store reference to the ContentStore
	static void initializePortal(ContentStoreImpl& store);

	/// Portal entry, called during ContentStore uninitialization, for the portal implementation to cleanup its context
	/// if necessary.
	///@param[in] store reference to the ContentStore
	static void uninitializePortal(ContentStoreImpl& store);

public:

	/// Portal entry for the portal implementation to fixup the path name of a file. For those case-insensitive file system
	/// such as NTFS, the portal implementation should covert the name to the same cases
	///@param[in] store reference to the ContentStore
	///@param[in] the pathname needs to fix up
	static std::string fixupPathname(ContentStoreImpl& store, const std::string& pathname);

	/// Portal entry for the portal implementation to create or validate if a path-of-volume is valid
	/// For example on NTFS, if this pathOfVolume refer to a folder, the portal must create the directory and return true if
	/// the directory exists and allow to create files under it
	///@param[in] store reference to the ContentStore
	///@param[in] the pathOfVolume the full pathname of volume to create or validate
	///@param[in] the volname
	///@true if the given pathOfVolume is valid and the portal can create files under pathOfVolume
	static bool createPathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume, const std::string& volname=NULL);

	/// Portal entry for the portal implementation to delete a path-of-volume
	/// For example on NTFS, if this pathOfVolume refer to a folder, the portal must unlink the directory and any files under it before
	/// returning true
	///@param[in] store reference to the ContentStore
	///@param[in] the pathOfVolume the full pathname of volume to delete
	///@true if the given pathOfVolume has been cleaned up and deleted
	static bool deletePathOfVolume(ContentStoreImpl& store, const std::string& pathOfVolume);

	/// Portal entry for the portal implementation to return the storage size.
	///@param[in] store reference to the ContentStore
	///@param[out] freeMB the free space left on the storage
	///@param[out] totalMB the total space of the storage
	///@param[in] rootPath the root path of the (virtual) volume, the portal should test the spaces from the given path
	static void getStorageSpace(ContentStoreImpl& store, uint32& freeMB, uint32& totalMB, const char* rootPath=NULL);

	/// Portal entry for the portal implementation to validate the name of the main filename that would be used as the
	/// content name.
	///@param[in] store reference to the ContentStore
	///@param[in] fileName the filename to validate
	///@param[in] contentType the content type for reference; "" if use the default contentType of the ContentStore
	///@return true if the filename is valid
	static bool validateMainFileName(ContentStoreImpl& store, std::string& fileName, const std::string& contentType);

	typedef struct _FileInfo
	{
		std::string filename;
		std::string stampLastWrite; ///< ISO8601 time-stamp of the last write of the file
	} FileInfo;
	typedef ::std::vector <FileInfo> FileInfos;

	/// Portal entry for the portal implementation to list all the main files. Non-recursive to the sub directories
	/// For those ContentStore that support supplemental files for a main file, supplemental file names would be excluded in
	/// the result
	///@param[in] store reference to the ContentStore
	///@param[in] rootPath the root path to list
	///@return a FileInfo collection of main file, the filename should NOT includes the part of rootPath
	///@note the portal MUST throw exceptions if it is not ready to respond this query at the moment
	static FileInfos listMainFiles(ContentStoreImpl& store, const char* rootPath=NULL);
//			throw (::TianShanIce::InvalidParameter, ::TianShanIce::InvalidStateOfArt);

	/// convert the member file name to the content name
	///@param[in] store reference to the ContentStore
	///@param[in] memberFilename the short name of member file of a content, the file name has been cut off the volume relatvie
	///           pathname
	static std::string memberFileNameToContentName(ContentStoreImpl& store, const std::string& memberFilename);

/*
	/// Portal entry for the portal implementation to test if the content exists
	/// For those ContentStore that support supplemental files for a main file, this entry also validate the file set of the 
	/// content
	///@param[in] store reference to the ContentStore
	///@param[in] mainFilePathname the full path of main file on the filesystem
	///@return true if the content is resident with neccessary supplemental files
	static bool fileExists(ContentStoreImpl& store, const std::string& mainFilePathname);
*/

	/// Portal entry for the portal implementation to test the file residential status of the member files of a content
	///@param[in] store reference to the ContentStore
	///@param[in] content reference to the content object
	///@param[in] mainFilePathname the full path of main file on the filesystem
	///@return the flags of the file residential status
	static uint64 checkResidentialStatus(ContentStoreImpl& store, uint64 flagsToTest, ContentImpl::Ptr pContent, const ::std::string& contentFullName, const ::std::string& mainFilePathname);

	/// Portal entry for the portal implementation to delete a content with all its supplemental files
	///@param[in] store reference to the ContentStore
	///@param[in] content reference to the content object
	///@param[in] mainFilePathname the full path of main file on the filesystem
	///@return true if all the disk files of this content has been confirmed not in the storage
	static bool deleteFileByContent(ContentStoreImpl& store, const ContentImpl& content, const ::std::string& mainFilePathname);

	/// Portal entry for the portal implementation to complete the renaming procedure
	///@param[in] store reference to the ContentStore
	///@param[in] mainFilePathname the full path name of main file on the filesystem that is about to rename
	///@param[in] newPathname the full path name of the new main file on the filesystem to rename to
	//@return false if renaming is not allowed or has not completed
	static bool completeRenaming(ContentStoreImpl& store, const ::std::string& mainFilePathname, const ::std::string& newPathname);

	/// Portal entry for the portal implementation to delete a content with all its supplemental files
	///@param[in] store reference to the ContentStore
	///@param[in, out] content reference to the content object that this entry should fill attributes into
	///           the following are some neccessary attributes that should be updated:
	///				- METADATA_FileSize                 file size in bytes
	///				- METADATA_SupportFileSize          file size subtotal, in bytes, of supplemental files excluding the main file
	///				- METADATA_PixelHorizontal          picture resoultion
	///				- METADATA_PixelVertical            picture resoultion 
	///				- METADATA_BitRate                  the encoded bitrate in bps
	///				- METADATA_PlayTime                 play time in msec
	///				- METADATA_FrameRate                frame rate in fps
	///				- METADATA_LocalType                local format type after save the content to local storage
	///				- METADATA_MD5CheckSum              the MD5 checksum of the main file (optional)
	///@param[in] mainFilePathname the full path of main file on the filesystem
	///@return true if some of the attributes are succesfully populated and updated into content metadata
	static bool populateAttrsFromFile(ContentStoreImpl& store, ContentImpl& content, const ::std::string& mainFilePathname);

	/// Portal entry for the portal implementation to setup and commit the active provision task
	///@param[in] store reference to the ContentStore
	///@param[in] content reference to the content object that this entry should fill attributes into
	///@param[in] sourceUrl - the source URL to provision this Content with
	///@param[in] sourceContentType - the enum type code to specify the source content type, coversion may happen to 
	///                               meet the type of this destination type
	///@param[in] overwrite         - true if need to overwrite a existing content
	///@param[in] startTimeUTC      - the scheduled start time, values are in UTC format, NULL if need to start immediately
	///@param[in] stopTimeUTC       - the scheduled stop time, values are in UTC format, NULL if no time-based cut off needed
	///@param[in] maxTransferBitrate - the limitation on transfer bitrate in Kbps, the transfering
	///                       should not exceed this given max bitrate, 0 if no limitation
	///@return the proxy to the Provision session
	static TianShanIce::ContentProvision::ProvisionSessionPrx submitProvision(
                            ContentStoreImpl& store, 
                            ContentImpl& content, 
                            const ::std::string& contentName, 
                            const ::std::string& sourceUrl, 
                            const ::std::string& sourceType, 
                            const ::std::string& startTimeUTC, 
                            const ::std::string& stopTimeUTC, 
                            const int maxTransferBitrate);
//			throw (::TianShanIce::InvalidParameter, TianShanIce::Storage::NoResourceException, 
//                   ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt);

	/// Portal entry for the portal implementation to book a passive provision task
	///@param[in] store reference to the ContentStore
	///@param[in] content reference to the content object that this entry should fill attributes into
	///@param[out] pushUrl the URL to where the client can import content
	///@param[in] sourceContentType - the enum type code to specify the source content type, coversion may happen to 
	///                               meet the type of this destination type
	///@param[in] overwrite         - true if need to overwrite a existing content
	///@param[in] startTimeUTC      - the scheduled start time, values are in UTC format, NULL if need to start immediately
	///@param[in] stopTimeUTC       - the scheduled stop time, values are in UTC format, NULL if no time-based cut off needed
	///@param[in] maxTransferBitrate - the limitation on transfer bitrate in Kbps, the transfering
	///                       should not exceed this given max bitrate, 0 if no limitation
	///@return the proxy to the Provision session
	static TianShanIce::ContentProvision::ProvisionSessionPrx bookPassiveProvision(ContentStoreImpl& store, const ContentImpl& content, const ::std::string& contentName,
						::std::string& pushUrl, const ::std::string& sourceType, const ::std::string& startTimeUTC, const ::std::string& stopTimeUTC, const int maxTransferBitrate);
//			throw (::TianShanIce::InvalidParameter, ::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt);

	/// Portal entry for the portal implementation to expose the URL where the content can be export from
	///@param[in] store reference to the ContentStore
	///@param[in] content reference to the content object that this entry should fill attributes into
	///@param[in] targetCSType     the target contentstore after exporting
	///@param[in] transferProtocol the expected transfer protocol to export the content, if ContentStore does not support the
	///                              expected transfer protocol, InvalidStateOfArt exception will throw. 
	///@param[out] params		     the addition information to the export content, such as protocol, file count, file list, ttl, etc
	///@return the URL where the user can download the content from the storage
	static std::string getExportURL(ContentStoreImpl& store, ContentImpl& content, const ::TianShanIce::ContentProvision::ProvisionContentKey& contentkey, const ::std::string& transferProtocol, ::Ice::Int transferBitrate, ::Ice::Int& ttl, ::TianShanIce::Properties& params);

	/// Portal entry for the portal implementation to cancel a on-going provision task attached on the given content
	///@param[in] store reference to the ContentStore
	///@param[in] content reference to the content object that this entry should fill attributes into
	///@param[in] provisionTaskPrx the provision task bound on the content
	///@note the entry implementation must return only if the canceling is confirmed, otherwise throw exceptions
	///@throw ServerError - if the canceling failed
	///@throw InvalidStateOfArt - if the canceling is not allowed in this state
	static void cancelProvision(ContentStoreImpl& store, ContentImpl& content, const ::std::string& provisionTaskPrx);
//		throw (::TianShanIce::ServerError, ::TianShanIce::InvalidStateOfArt);

	static void notifyReplicasChanged(ContentStoreImpl& store, const ::TianShanIce::Replicas& replicasOld, const ::TianShanIce::Replicas& replicasNew);

protected:
	// Events of the ContentStoreLib
	// Classes derived from this base ContentStoreImpl maybe follow with additional procedures on the given events
	// by overwritting the following virtual methods

	// Events of a content record has just been created, deleted or state changed
	virtual void OnContentCreated(const ::Ice::Identity& identContent) {}
	virtual void OnContentDestroyed(const ::Ice::Identity& identContent) {}
	virtual void OnContentStateChanged(const ::Ice::Identity& identVolume, const ::TianShanIce::Storage::ContentState previousState, const ::TianShanIce::Storage::ContentState newState) {}

	// Events of a volume has just been created, mounted
	virtual void OnSubVolumeCreated(const ::Ice::Identity& identVolume) {}
	virtual void OnVolumeMounted(const ::Ice::Identity& identVolume, const ::std::string& path) {}

	// Events of a content's provision has just been started, stopped, in-progress, or becomes streamable
	virtual void OnContentProvisionStarted(const ::Ice::Identity& identContent);
	virtual void OnContentProvisionStopped(const ::Ice::Identity& identContent);
	virtual void OnContentProvisionProgress(const ::Ice::Identity& identContent, ::Ice::Long processed, ::Ice::Long total);
	virtual void OnContentProvisionStreamable(const ::Ice::Identity& identContent);
};

#define RSDFLAG(_FLAG) FLAG(::TianShanIce::Storage::_FLAG)

#define DBFILENAME_Content "Content"
#define DBFILENAME_Volume  "Volume"

#define IdentityToObj(_CLASS, _ID) ::TianShanIce::Storage::_CLASS##Prx::uncheckedCast(_adapter->createProxy(_ID))
#define IdentityToObj2(_CLASS, _ID) ::TianShanIce::Storage::_CLASS##Prx::checkedCast(_adapter->createProxy(_ID))
#define IdentityToObjEnv(_ENV, _CLASS, _ID) ::TianShanIce::Storage::_CLASS##Prx::checkedCast((_ENV)._adapter->createProxy(_ID))

// -----------------------------
// class CachedContentListImpl
// -----------------------------
class CachedContentListImpl : public TianShanIce::Storage::CachedContentList, virtual public ICEAbstractMutexRLock
{
	friend class CachedListContentsCmd;
	friend class CachedContentListReadResultCmd;
public:
	typedef ::IceInternal::Handle< CachedContentListImpl > Ptr;
	CachedContentListImpl(ContentStoreImpl& store, ::Ice::Int timeout);
	virtual ~CachedContentListImpl() {}

public:
	//impl of CachedContentList
    virtual void next_async(const ::TianShanIce::Storage::AMD_CachedContentList_nextPtr& amdCB, const ::TianShanIce::StrValues& metaDataNames, const ::std::string& startName, ::Ice::Int maxCount, ::Ice::Int renewMs, const ::Ice::Current& c);
    virtual void destory(const ::Ice::Current& c);
	::Ice::Int size(const ::Ice::Current& c) const;
	::Ice::Int left(const ::Ice::Current& c) const;

	//impl of TimeoutObj
    virtual void OnTimer(const ::Ice::Current& c);

protected:
	::Ice::Long _expiration;
	ContentStoreImpl& _store;
	IdentCollection _identResults;
	IdentCollection::iterator _itResults;
	::Ice::Identity _ident, _identVolume;
};

std::string invokeSignature(const ::Ice::Current& c);

}} // namespace

#endif // __ZQTianShan_ContentImpl_H__
