#ifndef __ZQTianShan_PhoEdgeRMEnv_H__
#define __ZQTianShan_PhoEdgeRMEnv_H__

#include "PhoAllocationOwnerImpl.h"
#include "S6Client.h"
#include "FileLog.h"
#include <string>
#include "Log.h"
#include "Configuration.h"
#include "definition.h"
#include "InetAddr.h"
#include "SystemUtils.h"
#include "TianShanDefines.h"
#include "IceLog.h"

#ifdef _DEBUG
#  pragma comment(lib, "Iced")
#  pragma comment(lib, "IceUtild")
#  pragma comment(lib, "freezed")
#else
#  pragma comment(lib, "Ice")
#  pragma comment(lib, "IceUtil")
#  pragma comment(lib, "freeze")
#endif //_DEBUG

// #define YTD_EVICTOR

#define PHOALLOCATION_KEY_SEPARATOR  "#"
#define ICE_PhoAllocOwner	         "PhoAllocOwner"

namespace ZQTianShan {
	namespace EdgeRM {

#define COST_MAX                     (10000)
#define COST_UNAVAILABLE             (COST_MAX +1)
#define MAX_SESSION_COUNT            (5000)

class PhoEdgeRMEnv;

#define DECLARE_DICT(_DICT)	_DICT##Ptr _p##_DICT; ZQ::common::Mutex _lock##_DICT
#define DECLARE_CONTAINER(_OBJ)	Freeze::EvictorPtr _e##_OBJ;// ZQ::common::Mutex _lock##_OBJ;
#define DECLARE_INDEX(_IDX)	TianShanIce::EdgeResource::_IDX##Ptr _idx##_IDX;

// -----------------------------
// class WatchDog
// -----------------------------
class WatchDog : public ZQ::common::NativeThread
{
	friend class S6SessionGroup;
	typedef SYS::SingleObject Event; // for the stupid naming of SingleObject

public:
	WatchDog(PhoEdgeRMEnv& env);
	virtual ~WatchDog();

public:
	///@param[in] contentIdent identity of the object
	///@param[in] timeout the timeout to wake up timer to check the specified object
	void watch(S6SessionGroup::Ptr group, ::Ice::Long syncInterval =0);
   void  unwatch(S6SessionGroup::Ptr group);
	//quit watching
	void quit();
protected:

	int		run();
	//used for third party to stop this thread
	int		terminate(int code /* = 0 */);

private:

	PhoEdgeRMEnv&             _env;

	typedef std::multimap <S6SessionGroup::Ptr, Ice::Long > SyncMap; // sessGroup to expiration map
	ZQ::common::Mutex   _lockGroups;
	SyncMap				_groupsToSync;
	Event	            _event;

	bool				_bQuit;
};

// -----------------------------
// class PhoEdgeRMEnv
// -----------------------------
class PhoEdgeRMEnv
{
public:
	
	PhoEdgeRMEnv(ZQ::common::Log& log, std::string strLogFolder, int32 maxCandidates,
		::Ice::Int allocationEvictorSize, ::Ice::Int heartBeatTime, int ThreadPoolSize, 
		int interval , const char* databasePath = NULL);
	virtual ~PhoEdgeRMEnv();

	virtual bool initialize();
	void uninitialize();

public:

	ZQ::common::Log&		        _log;

	Ice::CommunicatorPtr			_communicator;
//	ZQADAPTER_DECLTYPE          	 _adapter;
	Ice::ObjectAdapterPtr            _adapter;
	ZQ::common::Log*				_pIceLog;
	TianShanIce::common::IceLogIPtr	_iceLog;
	std::string						_strLogFolder;

	std::string				    _programRootPath;
	PhoAllocationOwnerImpl::Ptr	_allocOwnerPtr;
	TianShanIce::EdgeResource::AllocationOwnerPrx  _allocOwnerPrx;
	Ice::Identity               _identPhoAllocOwner;

	// native thread pool
	ZQ::common::NativeThreadPool* _pThreadPool;

	ZQ::common::Log::loglevel_t _rtspTraceLevel;

	int							_sessTimeout;
	std::string                 _userAgent;
	ZQ::common::InetHostAddress _bindAddr;
    
public: // 
	std::string				_dbPath;
	std::string             _dbRuntimePath;
	std::string				_modulePath;
	int						_allocationEvictorSize;
	Ice::Long               _allocationLeaseMs; ///< the lease time in msec
	int32					_maxCandidates;
	int32					_interval;
	int32					_ThreadPoolSize;

	bool                    _bQuit;

protected:

	WatchDog*	   _watchDog;

	std::map<std::string, std::string> _streamLinkIdToBaseURLs; //map streamLinkId to baseURL
	Mutex							   _lockStrLikToBaseURL;

	Mutex                              _lockOndemandSession;
	typedef std::map<std::string, std::string> OnDemandSessionMap;
	OnDemandSessionMap                 _onDemandSessions;
public: 
	bool		createSessionGroups(const std::string& baseURL, int maxSessionsPerGroup, int maxSessionGroups,const std::string& streamLinkId);

	bool        syncS6Session(const std::string& sessionId, const std::string& OnDemandSessionId, const std::string& sessGroup);
	int          getMaxSessGroups(const std::string& ip, int port);

	bool        hasOnDemandSession(const std::string& onDemandSessId);
	void        addOnDemandSession(const std::string& onDemandSessId);
	void        removeOnDemandSession(const std::string& onDemandSessId);

protected:
	std::string  getSessionGroupPrefix(const std::string& streamLinkId);
};

#define INDEXFILENAME(_IDX)	#_IDX "Idx"

#define IdentityToObjEnv(_ENV, _CLASS, _ID) ::TianShanIce::EdgeResource::_CLASS##Prx::uncheckedCast((_ENV)._adapter->createProxy(_ID))
#define IdentityToObjEnv2(_ENV, _CLASS, _ID) ::TianShanIce::EdgeResource::_CLASS##Prx::checkedCast((_ENV)._adapter->createProxy(_ID))
#define envlog			(_env._log)
//#define evntlog		    (_env._eventlog)

}} // namespace

#endif // __ZQTianShan_PhoEdgeRMEnv_H__
