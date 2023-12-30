// ssmNGODr2c1.h: interface for the ssmNGODr2c1 class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __ssmNGODr2c1_H__
#define __ssmNGODr2c1_H__

#include "TianShanDefines.h"

#include "ngodDefs.h"
#include "descCode.h"
#include "StreamSmithModule.h"
#include "stroprt.h"
#include "NGODr2c1.h"
#include "NGODFactory.h"
#include "StreamIdx.h"
#include "GroupIdx.h"
#include "StreamEventSinkI.h"
#include "thrdConnService.h"
#include "ContextImpl.h"
#include "SessionWatchDog.h"

#include "TsEvents.h"
#include "TsSRM.h"
#include "EventChannel.h"
#include "XMLPreferenceEx.h"

#include "FileLog.h"
#include "Log.h"
#include "Locks.h"
#include "NativeThreadPool.h"

#include "./ZQResource.h"

#include <time.h>
#include <string>
#include <list>

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include <IceStorm/IceStorm.h>

using namespace std;

#define DEFAULT_LOGFILE_NAME "C:\\TianShan\\log\\ssm_NGOD_r2c1\\ssm_NGOD_r2c1.log"
#define DEFAULT_SAFESTORE_PATH "C:\\TianShan\\data\\ssm_NGOD_r2c1\\"
#define DEFAULT_EVICTOR_SIZE 1000
#define MIN_EVICTOR_SIZE 200
#define MAX_EVICTOR_SIZE 5000
#define DEFAULT_LOGFILE_NUM 10
#define DEFAULT_LOGFILE_SIZE 1024*1024*20
#define DEFAULT_LOGFILE_LEVEL 6
#define DEFAULT_RECONNECT_INTERVAL 10

#define MIN_SESSION_TIMEOUT 60
#define DEFAULT_SESSION_TIMEOUT 600
// no max value just according to configuration

#define MIN_ScanSession_INTERVAL MIN_SESSION_TIMEOUT
// default value is equal to 1/10 * session timeout value
#define MAX_ScanSession_INTERVAL 600

#define MIN_ScanThreadPool_SIZE 5
#define DEFAULT_ScanThreadPool_SIZE 10
#define MAX_ScanThreadPool_SIZE 50

// Announce code
#define NGOD_ANNOUNCE_ENDOFSTREAM "2101"
#define NGOD_ANNOUNCE_ENDOFSTREAM_STRING "End-of-Stream Reached"
#define NGOD_ANNOUNCE_BEGINOFSTREAM "2104"
#define NGOD_ANNOUNCE_BEGINOFSTREAM_STRING "Start-of-Stream Reached"
#define NGOD_ANNOUNCE_CLIENTSESSIONTERMINATED "5402"
#define NGOD_ANNOUNCE_CLIENTSESSIONTERMINATED_STRING "Client Session Terminated"
#define NGOD_ANNOUNCE_SESSIONINPROGRESS "5700"
#define NGOD_ANNOUNCE_SESSIONINPROGRESS_STRING "Session In Progress"

// buffer size
#define MY_BUFFER_SIZE 2048

#define SERVANT_TYPE "Context"
#define ORIGINAL_URI "Original-Uri"
#define RESOURCE_URI "Resource-Uri"

#define MAP_KEY_SESSION "in#session"
#define MAP_KEY_METHOD "in#method"
#define MAP_KEY_SEQUENCE "in#sequence"
#define MAP_KEY_USERAGENT "in#useragent"
#define MAP_KEY_XREASON "in#xreason"

#define MAP_KEY_WEIWOOSESSIONFULLID "in#weiwoosessionfullid"
#define MAP_KEY_WEIWOOSESSIONDESTROYREASON "in#weiwoosessiondestroyreason"
#define MAP_KEY_REMOVECONTEXTREASON "in#removecontextreason"

#define MAP_KEY_STREAMFULLID "in#streamfullid"
#define MAP_KEY_STREAMSTATEDESCRIPTION "out#streamstatedescription"
#define MAP_KEY_STREAMPOSITION "out#streamposition"
#define MAP_KEY_STREAMSCALE "out#streamscale"

#define MAP_KEY_LASTERROR "out#lasterror"

#define MAX_SESSION_CONTEXT 20000

#define DEFAULT_TheadPoolSize 5
#define MIN_TheadPoolSize 3

#define  DEFAULT_TTL_IDLE_SESSION				(10 *60 *1000)	// 10 min
#ifndef _DEBUG
#  define  MAX_TTL_IDLE_SESSION					(60 *60 *1000)	// 60 min
#  define  MIN_TTL_IDLE_SESSION					(30 *1000)		// 30 sec
#else // _DEBUG
#  define  MAX_TTL_IDLE_SESSION					(60 *1000)		// 60 sec
#  define  MIN_TTL_IDLE_SESSION					(5 *1000)		// 5  sec
#endif // _DEBUG


struct CONFIG
{
	std::string _sessionManager;
	std::string _iceStorm;
	std::string _eventAdapter;
	std::string _contextAdapter;
	int _timeoutInterval;
	int _threadPoolSize;
	std::string _logFileName;
	int _logFileSize;
	int _logFileNum;
	int _logFileLevel;
	std::string _safeStorePath;
	int _evictorSize;
	bool _useGlobalSeq;
	bool _bSetParamHeartBeat;
	std::string _nodeGroupID;
	std::map<std::string, std::string> _loadConfigMap;
	std::map<std::string, std::string> _iceConfigMap;
};
	
typedef std::map<std::string, std::string> INOUTMAP;
typedef INOUTMAP::iterator INOUTMAP_ITOR;
typedef INOUTMAP::const_iterator INOUTMAP_CITOR;

typedef std::map<std::string, std::string> STRINGMAP;
typedef STRINGMAP::iterator STRINGMAP_ITOR;
typedef STRINGMAP::const_iterator STRINGMAP_CITOR;

class RequestHandler;
class thrdCleanupSession;
class thrdConnService;
class StreamEventSinkI;

class ssmNGODr2c1
{
	friend class RequestHandler;
	friend class thrdCleanupSession;
	friend class thrdConnService;
	friend class StreamEventSinkI;
	friend class InServiceRequest;
	friend class OutServiceRequest;

public:

	ssmNGODr2c1();
	virtual ~ssmNGODr2c1();

	// init and uninit plug-in
	int setConfigPath(const char* pConfigPath);
	int doInit(IStreamSmithSite* pSite);
	int doUninit();

	RequestProcessResult doFixupRequest(IStreamSmithSite* pSite, IClientRequestWriter* pReq);
	RequestProcessResult doContentHandler(IStreamSmithSite* pSite, IClientRequestWriter* pReq);
	RequestProcessResult doFixupResponse(IStreamSmithSite* pSite, IClientRequest* pReq);

	bool sessionInProgressAnnounce(NGODr2c1::ContextImplPtr pContext);

	bool terminatAndAnnounce(NGODr2c1::ContextImplPtr pContext);

private:

	// init and uninit ice run-time
	bool initIceRunTime();
	void uninitIceRunTime();

	bool openSessionWatchDog();
	void closeSessionWatchDog();

	bool initThreadPool();
	void uninitThreadPool();

	// Connect IceStorm and session manager services
	bool ConnectIceStorm();
	bool ConnectSessionManager();

	// open and close safe store
	bool openSafeStore();
	void closeSafeStore();

	// load configuration, parse configuration and show configuration
	bool loadConfig();
	bool parseConfig();
	void showConfig();

	bool flushPrivateData(const TianShanIce::SRM::SessionPrx& sessPrx, const TianShanIce::ValueMap& varMap, INOUTMAP& inoutMap);

	bool openContext(const std::string& sessId, NGODr2c1::ContextImplPtr& pContext, NGODr2c1::ContextPrx& pContextPrx, INOUTMAP& inoutMap);
	bool removeContext(const std::string& sessId, INOUTMAP& inoutMap);

	// add session's context to evictor, and it can also be used to update session's last access time
	bool addContext(const NGODr2c1::ContextImplPtr pContext, INOUTMAP& inoutMap);

	// cleanup all the resource relevant to the session
	void cleanupSession(INOUTMAP& inoutMap);

	std::string getCurrentConnID(NGODr2c1::ContextImplPtr pContext);
	
	// in, inoutMap[MAP_KEY_WEIWOOSESSIONFULLID]
	bool getPurchase(::TianShanIce::Application::PurchasePrx& purchasePrx
		, const ::TianShanIce::SRM::SessionPrx sessionPrx, INOUTMAP& inoutMap);

	// in, inoutMap[MAP_KEY_STREAMFULLID]
	bool getStream(::TianShanIce::Streamer::StreamPrx& streamPrx, INOUTMAP& inoutMap);

	// in, inoutMap[MAP_KEY_WEIWOOSESSIONFULLID]
	bool getWeiwooSession(TianShanIce::SRM::SessionPrx& sessionPrx, INOUTMAP& inoutMap);
	// in, inoutMap[MAP_KEY_WEIWOOSESSIONFULLID]
	// in, timeout_value
	bool renewWeiwooSession(const TianShanIce::SRM::SessionPrx& sessionPrx, int timeout_value, INOUTMAP& inoutMap);
	// in, inoutMap[MAP_KEY_WEIWOOSESSIONFULLID]
	// in, inoutMap[MAP_KEY_WEIWOOSESSIONDESTROYREASON]
	bool destroyWeiwooSession(const TianShanIce::SRM::SessionPrx& sessPrx, INOUTMAP& inoutMap);

	// in, inoutMap[MAP_KEY_STREAMFULLID]
	// out, inoutMap[MAP_KEY_STREAMSTATEDESCRIPTION]
	bool getStreamState(TianShanIce::Streamer::StreamState& strmState
		, const TianShanIce::Streamer::StreamPrx& strmPrx, INOUTMAP& inoutMap);

	// in, inoutMap[MAP_KEY_STREAMFULLID]
	// out, inoutMap[MAP_KEY_STREAMPOSITION]
	// out, inoutMap[MAP_KEY_STREAMSCALE]
	bool getPositionAndScale(const TianShanIce::Streamer::StreamPrx& strmPrx, INOUTMAP& inoutMap);

public:

	ZQ::common::NativeThreadPool*			_pThreadPool;
	SessionWatchDog* _pSessionWatchDog;
	
	TianShanIce::SRM::SessionManagerPrx _pSessionManager;	
	CONFIG _config;

	::Freeze::ConnectionPtr _pSafeStoreConn;
	ZQ::common::Mutex _contextEvtrLock;
	::Freeze::EvictorPtr _pContextEvtr;
	NGODr2c1::StreamIdxPtr _pStreamIdx;
	NGODr2c1::GroupIdxPtr _pGroupIdx;

	NGODFactory::Ptr _pFactory;

	IStreamSmithSite* _pSite;
	std::string _globalSession;
	std::string _serverHeader;

	ZQ::common::SysLog _sysLog;
	ZQ::common::FileLog _fileLog;

	std::string _configPath;

	::Ice::CommunicatorPtr _pCommunicator;
	::Ice::ObjectAdapterPtr _pEventAdapter;
	::Ice::ObjectAdapterPtr _pContextAdapter;
	TianShanIce::Events::EventChannelImpl::Ptr _pEventChannal;

	thrdConnService _thrdConnService;

	struct ConnIDGroupPair
	{
		std::string _connectionID;
		std::string _sessionGroup;
	};
	ZQ::common::Mutex _connIDGroupsLock;
	std::vector<ConnIDGroupPair> _connIDGroups;

	typedef struct _SOPData
	{
		int serviceGroup;
		::TianShanIce::SRM::Resource streamers;
	} SOPData;

	typedef std::map <std::string, SOPData > SopMap;
	SopMap _sopRestriction;

	uint32 _globalSequence;
};

class SmartServerRequest
{
public: 
	SmartServerRequest(IServerRequest*& pServerRequest);
	virtual ~SmartServerRequest();

private:
	IServerRequest*& _pServerRequest;
};

#endif // __ssmNGODr2c1_H__

