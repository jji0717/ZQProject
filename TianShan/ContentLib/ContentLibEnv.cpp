#include "ContentLibEnv.h"
#include "ContentLibConfig.h"
#include "ContentLibRequest.h"
#include "FileSystemOp.h"

#define LOG_MODULE_NAME			"ContentLibEnv"
#define CONTENTLIB_APPNAME		"ContentLibApp"


ContentLibEnv::ContentLibEnv(Ice::CommunicatorPtr& communicator)
	: _pThreadPool(NULL), _pThreadPoolForContents(NULL), _communicator(communicator), _evtAdap(NULL),  _adapter(NULL)
{
}

ContentLibEnv::~ContentLibEnv()
{
}

bool ContentLibEnv::ConnectEventChannel()
{
	glog(ZQ::common::Log::L_NOTICE, CLOGFMT(ContentLibEnv, "do connectEventChannel()"));

	try
	{
		if(_contentlib)
		{
			_eventChannel = new TianShanIce::Events::EventChannelImpl(_evtAdap, _config.TopicMgrEndPoint.c_str());
			TianShanIce::Properties qos;
			_eventChannel->sink(_contentlib, qos);
		}
	}
	catch (TianShanIce::BaseException& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibEnv, "connectEventChannel(%s) caught(%s: %s)")
			, _config.TopicMgrEndPoint.c_str(), ex.ice_name().c_str(), ex.message.c_str());
		return false;
	}
	catch (Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibEnv, "connectEventChannel(%s) caught(%s)")
			, _config.TopicMgrEndPoint.c_str(), ex.ice_name().c_str());
		return false;
	}
	catch (...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibEnv, "connectEventChannel(%s) caught unexpect exception")
			, _config.TopicMgrEndPoint.c_str());
		return false;
	}

	glog(ZQ::common::Log::L_NOTICE, CLOGFMT(ContentLibEnv, "connectEventChannel() successfully"));
	return true;
}

bool ContentLibEnv::init()
{
	if (_config.ContentLibEndPoint.size() == 0)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibEnv, "Endpoint not configurated"));
		return false;
	}

	// create native thread pool
	_pThreadPool = new ZQ::common::NativeThreadPool(_config.ThreadPoolSize * 2);
	_pThreadPoolForContents = new ZQ::common::NativeThreadPool(_config.ThreadPoolSize);

	_endpoint = _config.ContentLibEndPoint;	

	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentLibEnv, "open adapter %s at %s"), CONTENTLIB_APPNAME, _endpoint.c_str());

	_adapter = ZQADAPTER_CREATE(_communicator, "ContentLib", _endpoint.c_str(), glog);

	// create listen event adapter
	try 
	{
		_evtAdap = _communicator->createObjectAdapterWithEndpoints("ContentLibEventAdapter", _config.ListenEventEndPoint);
		_evtAdap->activate();
	}
	catch(Ice::Exception& ex) 
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibEnv, "create adapter for listen event caught %s, endpoint is %s"), 
			ex.ice_name().c_str(), _config.ListenEventEndPoint.c_str());
		return false;
	}

	// check path
	{
		if (!_config.safeStorePath.size())
		{
			std::string strPath = FS::getImagePath();
			if ( strPath.size() > 0 )
			{
				char path[MAX_PATH] = {0};
				strcpy(path, strPath.c_str());
				char* p = strrchr(path, FNSEPC);
				if (NULL !=p)
				{
					*p='\0';
					p = strrchr(path, FNSEPC);
					if (NULL !=p && (0==stricmp(FNSEPS "bin", p) || 0==stricmp(FNSEPS "exe", p)))
						*p='\0';
				}

				strcat(path, FNSEPS "data" FNSEPS);
				_dbPath = path;
			}
		}
		else _dbPath = _config.safeStorePath;

		if (FNSEPC != _dbPath[_dbPath.length()-1])
			_dbPath += FNSEPS;
	}

	try {

		glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibEnv, "opening database at path: %s"), _dbPath.c_str());

		// open dictionary
		FS::createDirectory(_dbPath);

		// initial MetaLib
		::TianShanIce::StrValues nonIndices;
		ContentLibConfig::MetadataCategories::iterator iter = _config.categories.begin();
		for (; iter != _config.categories.end(); ++iter) 
		{
			if(iter->indexFlag == 0)
				nonIndices.push_back(iter->category);
		}
		_lib = new ::ZQTianShan::MetaLib::MetaLibImpl(glog, *_pThreadPool, _communicator, nonIndices, _dbPath.c_str());

		// initialize database
		::TianShanIce::Repository::MetaDataMap schema;
		::TianShanIce::Repository::MetaDataValue metaData;
		metaData.value = METADATA_INDEX;
		metaData.hintedType = ::TianShanIce::vtStrings;

		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(NETID, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(VOLUMENAME, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(CONTENTNAME, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(CONTENTSTATE, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(OBJECTTYPE, metaData));
//		metaData.value = METADATA_NONINDEX;
//		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(LASTUPDATED, metaData));
		for (iter = _config.categories.begin(); iter != _config.categories.end(); ++iter) 
		{
			if(iter->indexFlag == 0)
				metaData.value = METADATA_NONINDEX;
			else
				metaData.value = METADATA_INDEX;
			schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(iter->category, metaData));
			glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibEnv, "Meta Data [%s] configured, indexFlag [%d]"), (iter->category).c_str(), iter->indexFlag);
		}
//		glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibEnv, "Max content count [%d] configured"), _config.maxContent);

		_lib->proxy()->registerMetaClass(CONTENTREPLICA, schema); // CONTENTREPLICA 

		schema.clear();
		metaData.value = METADATA_INDEX;
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(ENDPOINT, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(NETID, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(OBJECTTYPE, metaData));

		_lib->proxy()->registerMetaClass(CONTENTSTOREREPLICA, schema); // CONTENTSTOREREPLICA

		schema.clear();
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(NETID, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(VOLUMENAME, metaData));
		schema.insert(::TianShanIce::Repository::MetaDataMap::value_type(OBJECTTYPE, metaData));

		_lib->proxy()->registerMetaClass(METAVOLUME, schema); // METAVOLUME

		// init ContentLib
		_contentlib = new ContentLibImpl(*_lib, *this);
		if (!_contentlib)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibEnv, "Fail to create ContentLibImpl"));
			return false;
		}

		_adapter->ZQADAPTER_ADD(_communicator, _contentlib, CONTENTLIB_APPNAME);
		glog(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentLibEnv, "ContentLib interface added to adatper"));

	}
	catch(const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibEnv, "Caught ice exception: %s"), ex.ice_name().c_str());
		return false;
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibEnv, "Caught unknown exception"));
		return false;
	}
	glog.flush();

	try
	{
		_adapter->activate();
	}
	catch(const Ice::Exception& ex)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibEnv, "Caught ice exception while active adapter: %s"), ex.ice_name().c_str());
		return false;
	}
	catch(...)
	{
		glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibEnv, "Caught unknown exception while active adapter"));
		return false;
	}
	glog.flush();

	ConnectEventChannelRequest* pConnEventChannel = new ConnectEventChannelRequest(*this);
	pConnEventChannel->start();

	if(_contentlib)
	{
		try
		{
			ContentLibImpl::Ptr _contentLibPtr = ContentLibImpl::Ptr::dynamicCast(_contentlib);
			_pSyncThread = new SyncThread(_contentLibPtr);
			_pSyncThread->start();
			_pSyncThread->notify();

			_pContentCacheThread = new ContentCacheThread(_contentLibPtr, *_lib);
			_pContentCacheThread->start();
			_pContentCacheThread->notify();
		}
		catch (Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibEnv, "SyncThread() caught(%s)")
				, ex.ice_name().c_str());
			return false;
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibEnv, "SyncThread() caught unknown error"));
			return false;
		}
	}

	glog(ZQ::common::Log::L_INFO, CLOGFMT(ContentLibEnv, "ContentLibEnv initialized successfully"));

	_bInited = true;

//	initContentStore();

	return true;
}

void ContentLibEnv::unInit()
{
	glog(ZQ::common::Log::L_DEBUG, CLOGFMT(ContentLibEnv, "unInit ContentLibEnv object"));

	// destroy native thread pool
// 	if (NULL != _pSyncThread)
// 		try {delete _pSyncThread;} catch (...){}
// 	_pSyncThread = NULL;

	if (NULL != _pThreadPool)
		try {delete _pThreadPool;} catch (...){}
	_pThreadPool = NULL;

	if (NULL != _pThreadPoolForContents)
		try {delete _pThreadPoolForContents;} catch (...){}
	_pThreadPoolForContents = NULL;

	_bInited = false;
}

bool ContentLibEnv::sync(std::string netId)
{
/*
	if(_contentlib)
	{
		try
		{
			ContentLibImpl::Ptr _contentLibPtr = ContentLibImpl::Ptr::dynamicCast(_contentlib);
			return _contentLibPtr->syncContentStore(netId);
		}
		catch (Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibEnv, "sync(%s) caught(%s)"), netId.c_str()
				, ex.ice_name().c_str());
			return false;
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibEnv, "sync(%s) caught unknown error"), netId.c_str());
			return false;
		}
	}
*/
	return false;
}

void ContentLibEnv::initContentStore()
{
	if(_contentlib)
	{
		ContentLibImpl::Ptr _contentLibPtr;
		try
		{
			_contentLibPtr = ContentLibImpl::Ptr::dynamicCast(_contentlib);
		}
		catch (Ice::Exception& ex)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibEnv, "initContentStore() caught(%s)")
				, ex.ice_name().c_str());
			return;
		}
		catch (...)
		{
			glog(ZQ::common::Log::L_ERROR, CLOGFMT(ContentLibEnv, "initContentStore() caught unknown error"));
			return;
		}
/*
		ContentLibConfig::CSParams::iterator iter = _config.csParams.begin();
		for (; iter != _config.csParams.end(); ++iter) 
		{
			std::string strNetId;
			if(_contentLibPtr->connectContentStore(iter->endpoint, strNetId))
				_contentLibPtr->addContentStoreReplica(strNetId, iter->endpoint);
		}
*/
	}
	return;
}

