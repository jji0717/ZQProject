#include "PhoEdgeRMEnv.h"
#include "ZQResource.h"
#include <sys/stat.h> 
#include <sys/types.h> 
#include "MD5CheckSumUtil.h"
extern ZQ::common::Config::Loader<PHOConfig>  _cfg;

namespace ZQTianShan {
namespace EdgeRM {

#define ADAPTER_NAME_EdgeRMPHO  "EdgeRMPho"
#define PHO_ERM_ICELOGFILENAME  "pho_ERMICE.log"

	// -----------------------------
	// class WatchDog
	// -----------------------------
	WatchDog::WatchDog(PhoEdgeRMEnv& env)
		: _env(env),_bQuit(false)
	{
	}

	WatchDog::~WatchDog()
	{	
		//exit thread
		terminate(0);

		{
			ZQ::common::MutexGuard gd(_lockGroups);
			_groupsToSync.clear();
		}	
	}

	int WatchDog::terminate(int code)
	{
		_event.signal();
		//wait until the run function exit
		waitHandle(100000);
		return 1;
	}
	void  WatchDog::quit()
	{
		_bQuit = true;
		_event.signal();
		waitHandle(10000);
	}
	int WatchDog::run()
	{
		while (!_bQuit)
		{
			//		Ice::Long stampNow = now();
				Ice::Long _nextWakeup = 0;
			{
				ZQ::common::MutexGuard gd(_lockGroups);

				for (SyncMap::iterator iter = _groupsToSync.begin(); !_bQuit && iter != _groupsToSync.end(); iter++)
				{
					S6SessionGroup::Ptr group = iter->first;
					if (group)
					{
						group->OnTimer();
						if (0 == _nextWakeup )
							_nextWakeup = group->getLastSync() + group->getSyncInterval();
						else
							_nextWakeup = (_nextWakeup > (group->getLastSync() + group->getSyncInterval())) ? (group->getLastSync() + group->getSyncInterval()) : _nextWakeup;
					}
				}
			}

			long sleepTime = (long) (_nextWakeup - now());

			if (sleepTime < 100)
				sleepTime = 100;

			if (_bQuit)
				continue;

			_event.wait(sleepTime);
		}

		return 1;
	}

	void WatchDog::watch(S6SessionGroup::Ptr group, ::Ice::Long syncInterval)
	{
		{
			if (syncInterval < 0)
				syncInterval = 0;
			::Ice::Long newSync = now() + syncInterval;

			ZQ::common::MutexGuard gd(_lockGroups);
			_groupsToSync.insert(std::make_pair(group, newSync));
		}

		_event.signal();
	}
	void WatchDog::unwatch(S6SessionGroup::Ptr group)
	{
		{
			ZQ::common::MutexGuard gd(_lockGroups);
			SyncMap::iterator iter = _groupsToSync.find(group);
			if(iter != _groupsToSync.end())
				_groupsToSync.erase(group);
		}
	}

// -----------------------------
// class PhoEdgeRMEnv
// -----------------------------
	PhoEdgeRMEnv::PhoEdgeRMEnv(ZQ::common::Log& log, std::string strLogFolder, int32 maxCandidates, ::Ice::Int allocationEvictorSize, ::Ice::Int allocationLeaseMs, int ThreadPoolSize , int interval, const char* databasePath)
: _log(log), _strLogFolder(strLogFolder),_dbPath(databasePath),_maxCandidates(maxCandidates),_ThreadPoolSize(ThreadPoolSize) {
	_programRootPath = ZQTianShan::getProgramRoot();
	_programRootPath += FNSEPS;
	_allocationEvictorSize = allocationEvictorSize;
	_allocationLeaseMs   = allocationLeaseMs;
	_interval = interval;

	if(_interval < 60)
		_interval = 60;

	//init 
	_rtspTraceLevel = ZQ::common::Log::L_DEBUG;
	_sessTimeout    = _cfg.sessionTimeOut;
	_userAgent      = "PHOEdgeRM";
    
	_bindAddr.setAddress(_cfg.sessionGroup.bindAddress.c_str());

	if(_cfg.sessionGroup.defaultSG < 1)
		_cfg.sessionGroup.defaultSG = 1;

	_bQuit = false;
	_pIceLog = NULL;
	_iceLog = NULL;
	_communicator= NULL;
	_adapter = NULL;
}

PhoEdgeRMEnv::~PhoEdgeRMEnv()
{
	uninitialize();
}

bool PhoEdgeRMEnv::initialize()
{

	/// initializa ice Communicator  and adapter
	try
	{
		_pIceLog = new ZQ::common::FileLog((_strLogFolder + PHO_ERM_ICELOGFILENAME).c_str(),
			_cfg.lPHOLogLevel,
			_cfg.lPHOlogFileCount,
			_cfg.lPHOLogFileSize,
			_cfg.lPHOLogBufferSize,
			_cfg.lPHOLogWriteTimteout);

		int i = 0;
		_iceLog = new ::TianShanIce::common::IceLogI(_pIceLog);
#if ICE_INT_VERSION / 100 >= 303

		::Ice::InitializationData initData;
		//initData.properties = Ice::createProperties(i, NULL);
		//initWithConfig(initData.properties);
		Ice::PropertiesPtr proper = Ice::createProperties(i,NULL);
		proper->setProperty("Ice.ThreadPool.Client.Size","5");
		proper->setProperty("Ice.ThreadPool.Client.SizeMax","10");
		proper->setProperty("Ice.ThreadPool.Server.Size","5");
		proper->setProperty("Ice.ThreadPool.Server.SizeMax","10");

		std::map<std::string, std::string>::iterator iter = _cfg.icePropMap.begin();
		for (; iter != _cfg.icePropMap.end(); ++iter) 
		{
			proper->setProperty(iter->first, iter->second);
		}

		//initData.logger = iceLog;

		initData.logger = _iceLog;
		_communicator = Ice::initialize(initData);
#else
		Ice::PropertiesPtr proper = Ice::createProperties(i,NULL);

		proper->setProperty("Ice.ThreadPool.Client.Size","5");
		proper->setProperty("Ice.ThreadPool.Client.SizeMax","10");
		proper->setProperty("Ice.ThreadPool.Server.Size","5");
		proper->setProperty("Ice.ThreadPool.Server.SizeMax","10");

		std::map<std::string, std::string>::iterator iter = _cfg.icePropMap.begin();
		for (; iter != _cfg.icePropMap.end(); ++iter) 
		{
			proper->setProperty(iter->first, iter->second);
		}

		_communicator = Ice::initializeWithPropertiesAndLogger(i,NULL,proper, _iceLog);
#endif // ICE_INT_VERSION

		_log(ZQ::common::Log::L_INFO, CLOGFMT(PhoEdgeRMEnv, "open adapter %s at %s"), ADAPTER_NAME_EdgeRMPHO, _cfg.endpoint.c_str());

		//_adapter = ZQADAPTER_CREATE(_communicator, ADAPTER_NAME_EdgeRMPHO, _cfg.endpoint.c_str(), _log);
		_adapter = _communicator->createObjectAdapterWithEndpoints(ADAPTER_NAME_EdgeRMPHO,  _cfg.endpoint.c_str());
	}
	catch(::Ice::Exception &ex)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(PhoEdgeRMEnv, "create local adapter failed, exception(%s)"), ex.ice_name().c_str());
		return false;
	}
	catch(...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(PhoEdgeRMEnv, "create local adapter caught unknown exception(%d)"), SYS::getLastErr());
		return false;
	}

	if(!_adapter)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(PhoEdgeRMEnv, "failed to open adapter %s at %s"), ADAPTER_NAME_EdgeRMPHO, _cfg.endpoint.c_str());
		return false;
	}


	try
	{
		// create native thread pool
		_pThreadPool = new ZQ::common::NativeThreadPool(_ThreadPoolSize);

		_allocOwnerPtr = new PhoAllocationOwnerImpl(_allocationLeaseMs,*this);

	//	_adapter->ZQADAPTER_ADD(_adapter->getCommunicator(), _allocOwnerPtr, ICE_PhoAllocOwner);
         
//		Ice::Identity identPhoAllocOwner;
		_identPhoAllocOwner.name="";
		_identPhoAllocOwner.category ="";
		_identPhoAllocOwner = _adapter->getCommunicator()->stringToIdentity(ICE_PhoAllocOwner);
		_adapter->add(_allocOwnerPtr, _identPhoAllocOwner);
		_allocOwnerPrx  = ::TianShanIce::EdgeResource::AllocationOwnerPrx::checkedCast(_adapter->createProxy(_identPhoAllocOwner));

		_adapter->activate();

		ZQ::common::RTSPSession::startDefaultSessionManager();

		_watchDog = new WatchDog(*this);
		if (_watchDog)
			_watchDog->start();
	}
	catch(const Ice::Exception& ex)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(PhoEdgeRMEnv,"failed to initialize PhoEdgeRMEnv caught ice exception '%s'"), ex.ice_name().c_str());
		return false;
	}
	catch(...)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(PhoEdgeRMEnv,"failed to initialize PhoEdgeRMEnv"));
		return false;
	}

	return true;
}

void PhoEdgeRMEnv::uninitialize()
{
	_bQuit = true;
	ZQ::common::RTSPSession::stopDefaultSessionManager();

	try
	{
		if (_adapter)
		{
			_adapter->deactivate();
		}
	}
	catch(...){}
 
	try
	{
		if (_watchDog)
		{
			_watchDog->quit();
			delete _watchDog;
		}
		_watchDog = NULL;

		S6SessionGroup::clearSessionGroup();
	}
	catch (...)
	{
	}
 
	try
	{
		if (_communicator)
		{
			_communicator->destroy();
		}
	}
	catch(...){}

	_communicator = NULL;
	_adapter = NULL;
	_allocOwnerPtr = NULL;
   _iceLog = NULL;


	if (_pIceLog) 
	{
		try
		{
			delete _pIceLog;
			_pIceLog = NULL;
		}
		catch(...)
		{
		}
	}
	// destroy native thread pool
	if (_pThreadPool)
		try {delete _pThreadPool;} catch (...){}
	_pThreadPool = NULL;


}
bool PhoEdgeRMEnv::createSessionGroups(const std::string& baseURL, int maxSessionsPerGroup, int maxSessionGroups, const std::string& streamLinkId)
{  
	//get all sessionGroups with streamLinkId
//	std::vector<std::string> sessionGroups = S6SessionGroup::getSessionGroupNames(streamLinkId);

	//get all sessionGroups with baseURL
	std::vector<std::string> sessionGroups = S6SessionGroup::getSessionGroupNames(baseURL);

/*
	///判断StreamLink上的Ip 或者 Port是否更新, 如果更新, 则需要删除之前创建的Conection, recreate Connection with new BaseURL
	{
		MutexGuard g(_lockStrLikToBaseURL);
		std::map<std::string, std::string>::iterator itorStreamLIDtoBase = _streamLinkIdToBaseURLs.find(streamLinkId);
		if(itorStreamLIDtoBase != _streamLinkIdToBaseURLs.end() && baseURL != itorStreamLIDtoBase->second)
		{
			for(std::vector<std::string>::iterator itorSG = sessionGroups.begin(); itorSG != sessionGroups.end(); itorSG++)
			{
				S6SessionGroup::Ptr s6SGPtr = S6SessionGroup::findSessionGroup(*itorSG);
				if(s6SGPtr)
				{
					if (_watchDog)
						_watchDog->unwatch(s6SGPtr);

					MutexGuard g(S6SessionGroup::_lockGroups);
					S6SessionGroup::_groupMap.erase(*itorSG);

					s6SGPtr = NULL;
				}
		
			}
			_streamLinkIdToBaseURLs.erase(itorStreamLIDtoBase);
			
			sessionGroups.clear();
		}
	}
*/
	if(!sessionGroups.empty())
		return true;

	// create the session groups
    // std::string sessionGroupPrxfix = getSessionGroupPrefix(streamLinkId);

	//create the session groups with baseURL
	std::string sessionGroupPrxfix = getSessionGroupPrefix(baseURL);
	MutexGuard g(S6SessionGroup::_lockGroups);
	for (int i = 0; i < maxSessionGroups; i++)
	{
		char groupName[100] = "";
		snprintf(groupName, sizeof(groupName)-2, "%s.%02d", sessionGroupPrxfix.c_str(), i+1);
		S6SessionGroup::Ptr group = new S6SessionGroup(*this, groupName, baseURL, maxSessionsPerGroup, _cfg.sessionGroup.groupSyncInterval);
		S6SessionGroup::_groupMap.insert(S6SessionGroup::SessionGroupMap::value_type(groupName, group));	

		if (_watchDog)
			_watchDog->watch(group, _cfg.sessionGroup.groupSyncInterval);

		_log(::ZQ::common::Log::L_DEBUG, CLOGFMT(PhoEdgeRMEnv, "StreamLinkId[%s] SessionGroup[%s] created to SS[%s]"),streamLinkId.c_str(), groupName, baseURL.c_str());	
	}

	{
		MutexGuard g(_lockStrLikToBaseURL);
		_streamLinkIdToBaseURLs[streamLinkId] =  baseURL;
	}
//	return S6SessionGroup::getSessionGroupNames(streamLinkId).size();
	return S6SessionGroup::getSessionGroupNames(baseURL).size();
}

std::string  PhoEdgeRMEnv::getSessionGroupPrefix(const std::string& streamLinkId)
{
	char UserMd5Index[12] = {1,3,5,7,9,11,13,15,17,18,19,20};

	std::string sessionGroup;
	ZQ::common::MD5ChecksumUtil sgMd5;
	sgMd5.checksum(streamLinkId.c_str(), streamLinkId.size());
	const char* pMD5 = sgMd5.lastChecksum();

	for(int i = 0; i < sizeof(UserMd5Index); i++)
	{
		sessionGroup.push_back(*(pMD5 + UserMd5Index[i]));
	}
	sessionGroup = _cfg.sessionGroup.netId + "." + sessionGroup;
	return sessionGroup;
}
int  PhoEdgeRMEnv::getMaxSessGroups(const std::string& ip, int port)
{
	int maxSessionGroups;
	char buf[512];
	snprintf(buf, sizeof(buf), "%s:%d", ip.c_str(), port);
	if(_cfg.sessionGroup.s6ServerInfos.find(buf) != _cfg.sessionGroup.s6ServerInfos.end())
	{
		maxSessionGroups =  _cfg.sessionGroup.s6ServerInfos[buf].maxSessionGroups;
	}
	else
	{
		maxSessionGroups = _cfg.sessionGroup.defaultSG;
	}
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(PhoEdgeRMEnv, "getMaxSessGroups[%s] count[%d]"), buf, maxSessionGroups);
	return maxSessionGroups;
}

bool  PhoEdgeRMEnv::syncS6Session(const std::string& sessionId, const std::string& OnDemandSessionId, const std::string& sessGroup)
{
	if(_bQuit)
	{
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(PhoEdgeRMEnv, "ignore sync S6Session[%s] OnDemandSessionId[%s]SessionGroup[%s], PHO_ERM is exiting"), sessionId.c_str(), OnDemandSessionId.c_str(), sessGroup.c_str());
		return true;
	}
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(PhoEdgeRMEnv, "sync S6Session[%s] OnDemandSessionId[%s]SessionGroup[%s]object in DB"), sessionId.c_str(), OnDemandSessionId.c_str(), sessGroup.c_str());

	bool nResult = false;
	Ice::Identity ident;

	try
	{
        if(!hasOnDemandSession(OnDemandSessionId))
		{
            return nResult;
		} 
		S6Session::Ptr s6Session = S6SessionGroup::openSession(OnDemandSessionId, sessGroup, true);
		if(s6Session)
		{
			s6Session->setSessionId(sessionId);
			_log(ZQ::common::Log::L_INFO, CLOGFMT(PhoEdgeRMEnv, "sync S6Session[%s] OnDemandSessionId[%s]SessionGroup[%s]successfully"), sessionId.c_str(), OnDemandSessionId.c_str(), sessGroup.c_str());
			nResult = true;
		}
	}
	catch(const Ice::ObjectNotExistException& ex)
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(PhoEdgeRMEnv, "sync S6Session[%s] PhoAllocation[%s] object not exist in DB"), sessionId.c_str(), ident.name.c_str());
	}
	catch(const Ice::Exception& ex)
	{
	}
	catch (...)
	{
	}
	return nResult;
}

bool PhoEdgeRMEnv::hasOnDemandSession(const std::string& onDemandSessId)
{
	bool bRet = false;
	std::map<std::string, std::string>::iterator itorOnDemandSess;

	MutexGuard g(_lockStrLikToBaseURL);
	itorOnDemandSess = _onDemandSessions.find(onDemandSessId);
	if(itorOnDemandSess != _onDemandSessions.end())
		bRet = true;
	return bRet;
}
void PhoEdgeRMEnv::addOnDemandSession(const std::string& onDemandSessId)
{
	std::map<std::string, std::string>::iterator itorOnDemandSess;

	MutexGuard g(_lockStrLikToBaseURL);

	MAPSET(OnDemandSessionMap, _onDemandSessions, onDemandSessId, onDemandSessId);
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(PhoEdgeRMEnv, "add OnDemandSession[%s]"), onDemandSessId.c_str());
}

void PhoEdgeRMEnv::removeOnDemandSession(const std::string& onDemandSessId)
{
	std::map<std::string, std::string>::iterator itorOnDemandSess;
	
	MutexGuard g(_lockStrLikToBaseURL);
	itorOnDemandSess = _onDemandSessions.find(onDemandSessId);
	if(itorOnDemandSess != _onDemandSessions.end())
	{
		_onDemandSessions.erase(itorOnDemandSess);
		_log(ZQ::common::Log::L_DEBUG, CLOGFMT(PhoEdgeRMEnv, "remove OnDemandSession[%s]"), onDemandSessId.c_str());
	}
}

}} // namespace
