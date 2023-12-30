// NGODEnv.h: interface for the NGODEnv class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __ssmNGODr2c1_H__
#define __ssmNGODr2c1_H__

#include "TianShanDefines.h"

#include "ngodDefs.h"
#include "descCode.h"
#include ".\..\..\streamsmith\StreamSmithModule.h"
#include "stroprt.h"
#include "NGODr2c1.h"
#include "NGODFactory.h"
#include "StreamIdx.h"
#include "GroupIdx.h"
#include "StreamEventSinkI.h"
#include "thrdConnService.h"
#include "ContextImpl.h"
#include "SessionWatchDog.h"
#include "LAMFacade.h"

#include "TsEvents.h"
#include "TsSRM.h"
#include "StreamSmithAdmin.h"
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
#include <map>

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include <IceStorm/IceStorm.h>

#include <TianShanDefines.h>

#include "./Ngod2Config.h"
#include "SOPConfig.h"

#include "streamerQuerier.h"
#include "SnmpQueryServantImpl.h"
#include "SelectionResourceManager.h"

using namespace std;

#define DEFAULT_LOGFILE_NAME "C:\\TianShan\\log\\ssm_NGOD_r2c1\\ssm_NGOD_r2c1.log"
#define DEFAULT_SAFESTORE_PATH "C:\\TianShan\\data\\ssm_NGOD_r2c1\\"
#define DEFAULT_EVICTOR_SIZE 1000
#define MIN_EVICTOR_SIZE 0
#define MAX_EVICTOR_SIZE 10000
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

#define NGOD_TIANSHAN_ANNOUNCE_STATE_CHANGE		"8802"
#define NGOD_TIANSHAN_ANNOUNCE_STATE_CHANGE_STRING "State Changed"

#define NGOD_TIANSHAN_ANNOUNCE_SCALE_CHANGE		"8801"
#define NGOD_TIANSHAN_ANNOUNCE_SCALE_CHANGE_STRING "Scale Changed"


#define NGOD_ANNOUNCE_ERROR_READING_CONTENT	"4400"
#define NGOD_ANNOUNCE_ERROR_READING_CONTENT_STRING "Error Reading Content Data"

#define NGOD_ANNOUNCE_DOWNSTREAM_FAILURE	"5401"
#define NGOD_ANNOUNCE_DOWNSTREAM_FAILURE_STRING "Downstream Failure"

#define NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR "5502"
#define NGOD_ANNOUNCE_INTERNAL_SERVER_ERROR_STRING "Internal Server Error"

#define NGOD_ANNOUNCE_BANDWIDTH_EXCEEDED_LIMIT "5602"
#define NGOD_ANNOUNCE_BANDWIDTH_EXCEEDED_LIMIT_STRING "Bandwidth Exceeded Limit"

#define NGOD_ANNOUNCE_SERVER_RESOURCES_UNAVAILABLE "5200" 
#define NGOD_ANNOUNCE_SERVER_RESOURCES_UNAVAILABLE_STRING "Server Resources Unavailable"

#define NGOD_ANNOUNCE_STREAM_BANDWIDTH_UNAVAILABLE "6001" 
#define NGOD_ANNOUNCE_STREAM_BANDWIDTH_UNAVAILABLE_STRING "Stream Bandwidth Exceeds That Available"

#define NGOD_ANNOUNCE_DOWNSTREAM_UNREACHABLE			"6004"
#define NGOD_ANNOUNCE_DOWNSTREAM_UNREACHABLE_STRING		"Downstream Destination Unreachable"

#define NGOD_ANNOUNCE_UNABLE_ENCRPT "6005" 
#define NGOD_ANNOUNCE_UNABLE_ENCRPT_STRING "Unable to Encrypt one or more Components"

//4400 Error Reading Content Data



#define PENALTY_ENABLE_MASK_PLAY	1
#define PENALTY_ENABLE_MASK_PAUSE	2
#define PENALTY_ENABLE_MASK_GETPAR	4

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
#define MAP_KEY_SOPNAME	"in#sopName"
#define MAP_KEY_STREMAERNETID	"in#StreamerNetId"


#define MAP_KEY_WEIWOOSESSIONFULLID "in#weiwoosessionfullid"
#define MAP_KEY_WEIWOOSESSIONDESTROYREASON "in#weiwoosessiondestroyreason"
#define MAP_KEY_REMOVECONTEXTREASON "in#removecontextreason"

#define MAP_KEY_STREAMFULLID "in#streamfullid"
#define MAP_KEY_STREAMSTATEDESCRIPTION "out#streamstatedescription"
#define MAP_KEY_STREAMPOSITION "out#streamposition"
#define MAP_KEY_STREAMPOSITION_HEX "out#streampositionHex"
#define MAP_KEY_STREAMPOSITION_STOPPOINT "out#StreamPosStopPoint"
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

//add by lxm for ngod new spec
#define C1CONNID "c1.connId"

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

using namespace com::izq::am::facade::servicesForIce;

class PenaltyManager;

// add by zjm
enum NgodVerCode
{
	NgodVerCode_R2 = 0, 
	NgodVerCode_R2_DecNpt, 
	NgodVerCode_C1, 
	NgodVerCode_C1_DecNpt, 
	NgodVerCode_UNKNOWN
};

class NGODEnv
{	
	friend class TeardownHandler;
	friend class RequestHandler;
	friend class thrdCleanupSession;
	friend class thrdConnService;
	friend class StreamEventSinkI;
	friend class InServiceRequest;
	friend class OutServiceRequest;
	friend class NGODr2c1::SessionViewImpl;

	// add by zjm
	friend class PlayListEventSinkI;

public:

	NGODEnv();
	virtual ~NGODEnv();

	// init and uninit plug-in
	int setConfigPath(const char* pConfigPath);
	int setLogDir(const char* pLogDir);
	// return 0, if initialize successfully
	// return 1, if initialize failed
	int doInit(IStreamSmithSite* pSite);
	int doUninit();

	RequestProcessResult doFixupRequest(IStreamSmithSite* pSite, IClientRequestWriter* pReq);
	RequestProcessResult doContentHandler(IStreamSmithSite* pSite, IClientRequestWriter* pReq);
	RequestProcessResult doFixupResponse(IStreamSmithSite* pSite, IClientRequest* pReq);

	bool sessionInProgressAnnounce(NGODr2c1::ctxData& context);

	bool terminatAndAnnounce(NGODr2c1::ctxData& context);

	bool AddBackupLAM(IN const std::string& volumnName ,IN const std::string& lamEndpoint ,bool bWarmUp);

	bool GetAeList(IN const std::string& volumnName , 
										IN const std::string& strProviderID,IN const std::string& strAssetID,
										IN int cueIn , IN int cueOut,
										OUT AssetElementCollection& aeInfo,OUT long& maxBW,
										RequestHandler* pRequestHandler);

	void getCoreInfo( int32 type , ZQ::common::Variant& var);

	void addPenaltyToStreamer( const std::string& sopName , const std::string& streamerNetId );

protected:
	// init and uninit ice run-time
	bool initIceRunTime();
	void uninitIceRunTime();

	bool openSessionWatchDog();
	void closeSessionWatchDog();

	bool initThreadPool();
	void uninitThreadPool();

	// Connect IceStorm and session manager services
	bool ConnectIceStorm();
	//no session Manager
	//bool ConnectSessionManager();

	// open and close safe store
	bool openSafeStore();
	void closeSafeStore();

	// load configuration, parse configuration and show configuration
	bool loadConfig();

	bool flushPrivateData(const TianShanIce::SRM::SessionPrx& sessPrx, const TianShanIce::ValueMap& varMap, INOUTMAP& inoutMap);

	bool openContext(const std::string& sessId, NGODr2c1::ctxData& context, NGODr2c1::ContextPrx& pContextPrx, INOUTMAP& inoutMap);
	bool removeContext(const std::string& sessId, INOUTMAP& inoutMap);

	// add session's context to evictor, and it can also be used to update session's last access time
	bool addContext(const NGODr2c1::ctxData&  context, INOUTMAP& inoutMap);

	// cleanup all the resource relevant to the session
	void cleanupSession(INOUTMAP& inoutMap);

	std::string getCurrentConnID(NGODr2c1::ctxData& context);
	
	// in, inoutMap[MAP_KEY_STREAMFULLID]
	bool getStream(::TianShanIce::Streamer::StreamPrx& streamPrx, INOUTMAP& inoutMap);

	// in, inoutMap[MAP_KEY_STREAMFULLID]
	// out, inoutMap[MAP_KEY_STREAMSTATEDESCRIPTION]
	bool getStreamState(TianShanIce::Streamer::StreamState& strmState
		, const TianShanIce::Streamer::StreamPrx& strmPrx, INOUTMAP& inoutMap);

	// in, inoutMap[MAP_KEY_STREAMFULLID]
	// out, inoutMap[MAP_KEY_STREAMPOSITION]
	// out, inoutMap[MAP_KEY_STREAMSCALE]
	///@param ngodVersion, add by zjm to support protocol versioning
	bool getPositionAndScale(const TianShanIce::Streamer::StreamPrx& strmPrx, INOUTMAP& inoutMap, int ngodVersion);

	///initialize the LAM interface
	void dummyGetAeList(OUT AssetElementCollection& aeInfos,OUT long& maxBW);
	
	void warmUp();


	void initSelectionManager( );

public:

	SelectionEnv							mSelectionEnv;

	NgodResourceManager						mResManager;

	PenaltyManager*							_penaltyManager;

	ZQ::common::NativeThreadPool*			_pThreadPool;
	SessionWatchDog*						_pSessionWatchDog;
	
	TianShanIce::SRM::SessionManagerPrx		_pSessionManager;	

	::Freeze::ConnectionPtr					_pSafeStoreConn;
	::Freeze::EvictorPtr					_pContextEvtr;
	NGODr2c1::StreamIdxPtr					_pStreamIdx;
	NGODr2c1::GroupIdxPtr					_pGroupIdx;

	NGODFactory::Ptr						_pFactory;

	IStreamSmithSite*						_pSite;
	std::string								_globalSession;
	std::string								_serverHeader;

	ZQ::common::SysLog						_sysLog;
	ZQ::common::FileLog						_fileLog;
	ZQ::common::FileLog						_iceLog;
	ZQ::common::FileLog                     _sentryLog;

	std::string								_configPath;
	
	::Ice::CommunicatorPtr					_pCommunicator;
	ZQADAPTER_DECLTYPE						_pEventAdapter;

	TianShanIce::Events::EventChannelImpl::Ptr _pEventChannal;

	thrdConnService							_thrdConnService;

	struct ConnIDGroupPair
	{
		std::string		_connectionID;
		std::string		_sessionGroup;
	};
	ZQ::common::Mutex						_connIDGroupsLock;
	std::vector<ConnIDGroupPair>			_connIDGroups;
	
	ZQ::common::Mutex						_contextEvtrLock;

	uint32									_globalSequence;

	streamerQuerier*						_streamerQuerier; // important for me 
	streamerReplicaSink*					_streamerReplciaSink;

	ZQ::common::Mutex						_lockSopMap;

	ZQ::common::Mutex						_lockLAMServer;

	ZQ::common::Mutex						_importChannelMapLocker;

	//PlaylistControl.enableEOT
	//bool									_playlistControlEnableEOT;	

// 	typedef struct _SopStreamerInfo
// 	{
// 		// the target streamer is healthy or not
// 		//this value depend on query replica information and replica report		
// 		bool								_bEnabled;
// 		long								_penaltyValue;
// 		std::string							_strNetID;
// 		std::string							_strEndpoint;
// 		std::string							_volumeName;
// 		std::string							_importChannel;
// 		Ice::Long							_totalBandwidth;
// 		Ice::Long							_usedBandwidth;
// 		Ice::Int							_totalStreamCount;
// 		Ice::Int							_usedStreamCount;
// 		TianShanIce::Streamer::StreamSmithAdminPrx	_streamServicePrx;
// 	}SopStreamerInfo;
// 	typedef struct _SOPData
// 	{
// 		int								serviceGroup;
// 		//::TianShanIce::SRM::Resource streamers;
// 		std::vector<SopStreamerInfo>	vecStreamerInfo;
// 	} SOPData;
// 
// 	typedef std::map <std::string, SOPData > SopMap;
// 	SopMap								_sopRestriction;

// 	typedef struct _LogSytanxMonitorInfo
// 	{
// 		std::string		logFile;
// 		std::string		syntaxFile;
// 	}LogSytanxMonitorInfo;
// 
// 	std::map<std::string,std::string>	_logSyntaxs;

	typedef struct _LAMInfo
	{
		int				iSupportNasStream;
		int				iLibPriority;
		int				iReferenceOnly;
		std::string		lamEndpoint;
		LAMFacadePrx	lamPrx;
	}LAMInfo;
	typedef std::map<std::string,LAMInfo>	LAMMAP;
	LAMMAP				_lamMap;

	ZQ::common::Mutex	_lockLammap;

	SnmpQueryServantImpl	mSnmpServant;
// 	typedef struct _tagImportChannelInfo 
// 	{
// 		Ice::Long			maxBW;
// 		long				maxSessions;		
// 		Ice::Long			usedBW;
// 		long				usedSessions;	
// 	}ImportChannelInfo;
// 	typedef std::map< std::string , ImportChannelInfo >	ImportChannelInfoMap;
// 	//				channel Name , Channel Info
// 	ImportChannelInfoMap			_importChannelMap;
	
	
	
	
	

//	long							_streamerQueryInterval;

public:

	static	void	getNgodUsage( std::map< std::string , NGOD2::SOPRestriction::SopHolder >& sopRestriction , std::string& measuredSince );
	static	void	getImportChannelUsage( NGODr2c1::ImportChannelUsageS& usage );

	static	void	resetCounters();

	static	Ice::Long	counterMeasuredSince;

private:

	bool			bQuited;
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

