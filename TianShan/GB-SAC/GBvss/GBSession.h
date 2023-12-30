#ifndef __GBSession_H__
#define __GBSession_H__

#include "RTSPClient.h"
#include "TianShanUtils.h"
#include "TsStreamer.h"
#include "SystemUtils.h"


#define SYNC_INTERVAL	(60*1000)
#define SESSION_TIMEOUT	(600*1000)

#define SESSION_GROUP		"GBVSS_Session_Group"
#define	SETUP_TIMESTAMP		"GBVSS_SetupTimestamp"
#define VOLUME_NAME			"GBVSS_Volume_Name"
#define ONDEMANDNAME_NAME	"GBVSS_OnDemandName"
#define DESTINATION_NAME	"GBVSS_Destination"
#define SESSION_ID			"GBVSS_SessionId"
#define CONTROL_URI			"GBVSS_ControlUri"
#define	SPEED_LASTIDX		"GBVSS_MultiplySpeedLastIndex"
#define	SPEED_LASTDIR		"GBVSS_MultiplySpeedLastDirection"

namespace ZQ{
namespace StreamService{

class GBClient;
class GBVSSSessionGroup;

class GBVSSEnv;

using namespace ZQ::common;

struct SessionEvent
	{
		SessionEvent()
		{
		}
		~SessionEvent()
		{
			for (std::map<uint32, SYS::SingleObject* >::iterator iter = _pHandle.begin(); iter != _pHandle.end(); iter++)
				m_CloseEvent((*iter).first);
		}
		bool m_Init(uint32 CSeq)
		{
			ZQ::common::MutexGuard g(_mutex);
			if (_pHandle.find(CSeq) != _pHandle.end())
			{
				return false;
			}
			_pHandle[CSeq] = new SYS::SingleObject();
			return true;
		}
		bool m_SetEvent(uint32 CSeq)
		{
			ZQ::common::MutexGuard g(_mutex);
			bool b  = false;
			if (_pHandle.find(CSeq) == _pHandle.end())
			{
				return false;
			}
			_pHandle[CSeq]->signal();
			return true;
		}
// 		bool m_ResetEvent(uint32 CSeq)
// 		{
// 			ZQ::common::MutexGuard g(_mutex);
// 			bool b  = false;
// 			if (_pHandle.find(CSeq) == _pHandle.end())
// 			{
// 				return false;
// 			}
// 			b = _pHandle[CSeq]->reset();
// 			return b;
// 		}
		bool m_CloseEvent(uint32 CSeq)
		{
			ZQ::common::MutexGuard g(_mutex);
		//	bool b  = false;
			std::map<uint32, SYS::SingleObject* >::iterator iter = _pHandle.find(CSeq);
			if (iter == _pHandle.end())
			{
				return false;
			}
		//	b = CloseHandle(_pHandle[CSeq]);
			SYS::SingleObject* _pSingleObject = iter->second;
			_pHandle.erase(iter);
			if(_pSingleObject)
				delete _pSingleObject;	
		//	return b;
			return true;
		}

		ZQ::common::Mutex   _mutex;
		std::map<uint32, SYS::SingleObject* >	_pHandle;
	};

// -----------------------------
// GBSession
// -----------------------------
class GBSession : public RTSPSession
{
	friend class GBVSSSessionGroup;
	friend class GBClient;
public:

	struct StreamInfos
	{
		///< stream timeoffset in milliseconds
		int64								timeoffset;

		///< stream play duration in milliseconds
		int64								duration;

		///< stream scale
		float								scale;

		///< state
		std::string							state;

        ///< stream source
        std::string							source;

		StreamInfos()
			:timeoffset(0),
			duration(0),
			scale(0.0f)
		{
		}
	};
	typedef Pointer < GBSession > Ptr;
	typedef std::vector < Ptr > List;

public:
	GBSession(Log& log, NativeThreadPool& thrdpool, const char* streamDestUrl, const char* filePath=NULL, Log::loglevel_t verbosityLevel=Log::L_WARNING, int timeout=60000, const char* onDemandSessionId=NULL);
	~GBSession();

	virtual void	destroy();
	StreamInfos		getInfos() { return  _streamInfos; };
	std::string		getBaseURL();
	void			setControlUri(std::string controlUri) { _controlUri = controlUri; };
	std::string     getSessionId() { return _sessionId; };
	void			setSessionId(std::string sessionId) { _sessionId = sessionId; };
	std::string     getOnDemandSessionId() { return _sessGuid; };
	int32			getTimeout() { return _sessTimeout; };
	GBClient*		getR2Client();
	

protected:
	// overwrites of RTSPSession
	virtual void OnResponse_SETUP(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString);
	virtual void OnResponse_PLAY(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString);
	virtual void OnResponse_TEARDOWN(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString);
	virtual void OnResponse_PAUSE(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString);
	virtual void OnResponse_GET_PARAMETER(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString);
	virtual void OnResponse_SET_PARAMETER(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString);
	virtual void OnANNOUNCE(RTSPClient& rtspClient, RTSPMessage::Ptr& pInMessage);
	virtual void OnSessionTimer();

	bool	parseStreamInfo(const RTSPMessage::Ptr& pResp);

public:
	std::string		_volumeName;
	std::string		_groupName;
	TianShanIce::Properties		_props;	// store metadata if need
	SessionEvent	_eventHandle;
	uint			_resultCode;
	std::string     _sessionHistory;
	std::string		_tianShanNotice;
	std::string     _primartItemNPT;
    std::string     _controlSession;
    std::string     _stopNPT;
private:
	//Playlist
	GBVSSSessionGroup*	_group;
	StreamInfos			_streamInfos;
//	ZQ::common::Mutex	_mutex;	// lock for stream info
};

class GBClient : public RTSPClient
{
	friend class GBVSSSessionGroup;
public:
	typedef struct Session_Pair
	{
		std::string _sessionId;//<rtsp-session-id>
		std::string _onDemandSessionId;//<on-demand-session-id>
	}Session_Pair;

	typedef enum {
		NC_R2, NC_C1
	} ClientType;

	ClientType _type;
	GBVSSSessionGroup& _group;

	class FindByOnDemandSessionID
	{
	public:
		FindByOnDemandSessionID(std::string& strOnDemandSessionID):m_strOnDemandSessionID(strOnDemandSessionID){}

		bool operator() (const Session_Pair& pSession_list)
		{
			if (m_strOnDemandSessionID.compare(pSession_list._onDemandSessionId) == 0)
				return true;
			else
				return false;
		}
	private:
		const std::string& m_strOnDemandSessionID;
	};

public:
	GBClient(GBVSSSessionGroup& group, ClientType type, Log& log, NativeThreadPool& thrdpool, InetHostAddress& bindAddress, const std::string& baseURL, const char* userAgent = NULL, Log::loglevel_t verbosityLevel =Log::L_WARNING, tpport_t bindPort=0);
	~GBClient();

public:
	void	sendTeardown(const std::string& sessionId, const std::string& onDemandSessionId = "");
	void	sendPing();
	int		sendMessage2(RTSPMessage::Ptr pMessage, const RTSPMessage::AttrMap& headerToOverride) { return sendMessage(pMessage, headerToOverride); };

protected:
	virtual void OnResponse(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString);
	virtual void OnServerRequest(RTSPClient& rtspClient, const char* cmdName, const char* url, RTSPMessage::Ptr& pInMessage);
	virtual void OnRequestError(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RequestError errCode, const char* errDesc=NULL);
	virtual void OnConnected();
	virtual void OnError();

	virtual int  OnRequestPrepare(RTSPRequest::Ptr& pReq);
};

class GBVSSSessionGroup : protected TianShanUtils::TimeoutObj, virtual public SharedObject
{
public:
	typedef std::map<std::string, GBSession::Ptr> SessionMap; // Map of ODSess to session
	typedef std::multimap <std::string, std::string> SessionIndex; // map of index to OnDemandSessionId
	typedef std::pair < SessionIndex::iterator, SessionIndex::iterator> SessionIndexRange;
	typedef std::map<std::string, GBClient*> GBClientMap; // controlURL to RTSPClient map
	typedef std::multimap<std::string, GBVSSSessionGroup*> SessionGroups; // baseURL to GBVSSSessionGroup map
	typedef std::pair <  SessionGroups::iterator,  SessionGroups::iterator> SessionGroupsRange;
	typedef enum SessionGroupStatus
	{
		Sync = 1,
		Idle = 2
	}SessionGroupStatus;

	GBVSSSessionGroup(GBVSSEnv& env, Log& log, const std::string& name, const std::string& baseURL, int max = 600, Ice::Long syncInterval = 60000);
	~GBVSSSessionGroup();

	virtual int	sync(bool bOnConnected = false);
	void			syncSessionList(const std::vector<GBClient::Session_Pair>& list);
	size_t			size() { return _sessMap.size(); };
	int				getMaxSize() { return _max; };
	void			setMaxSize(int max) { _max = max; };
	void            setStatus(SessionGroupStatus st) { _status = st; };
	SessionGroupStatus			getStatus() { return _status; };
	std::string     getName() { return _name; }; 
	std::string     getBaseURL() { return _baseURL; }; 
	Ice::Long		getSyncInterval() { return _syncInterval; };
	Ice::Long		getLastSync() { MutexGuard g(_lockSyncTimeStamp); return _stampLastSync; };
	void			setLastSync(Ice::Long time, SessionGroupStatus st) { MutexGuard g(_lockSyncTimeStamp); _stampLastSync = time; _status = st;};
			
	virtual void OnTimer(const ::Ice::Current& = ::Ice::Current()); // look up session timeout

	virtual void add(GBSession& sess);
	virtual void remove(GBSession& sess);

	virtual GBSession::Ptr& lookupByOnDemandSessionId(const char* usrSessId);
	virtual void updateIndex(GBSession& sess, const char* indexName="SessionId", const char* oldValue=NULL);
	virtual GBSession::List lookupByIndex(const char* sessionId, const char* indexName="SessionId");

	void eraseSessionId(const std::string& sessionId, GBSession& sess);

	GBClient* getC1Client(const std::string& controlURL); // to select one RTSPClient
	GBClient* getR2Client();
	void addC1Client(const std::string& controlURL, GBClient* client); // to add GBClient
	void reomoveC1Client(const std::string& controlURL); // to remove GBClient

	static GBVSSSessionGroup* allocateSessionGroup(const std::string& baseURL); // to select one session group
	static GBVSSSessionGroup* findSessionGroup(const std::string& name); // to find session group
	static std::vector<std::string> getAllSessionGroup(); // return all group name

	static void addSessionGroup(const std::string& name, GBVSSSessionGroup* group); // to add session group
	static void removeSessionGroup(const std::string& name); // to remove session group

	static void clearSessionGroup();
	void clearGBClient();

	static Ice::Long checkAll();

protected:
	Log&		  _log;
	std::string  _name;
	std::string  _baseURL;
	int          _max;
	SessionGroupStatus _status;
	::Ice::Long _syncInterval;
	::Ice::Long _stampLastSync;

private:
	GBVSSEnv&				_env;
	GBClient*				_R2Clinet;
	GBClientMap			_C1ClientMap;
	static SessionGroups	_groups;
	static Mutex			_lockGroups;
	Mutex					_lockClients;
	SessionIndex			_sessIdIndex; // C1 sessionId to OnDemandSessionId
	SessionMap				_sessMap;  // store all session of group
	Mutex					_lockSessMap; // map of OnDemandSessionId to GBSession of the group
	Mutex					_lockSyncTimeStamp;
};

class SyncWatchDog : public ZQ::common::NativeThread
{
	friend class GBVSSSessionGroup;
public:
	SyncWatchDog(ZQ::common::Log& log);
	virtual ~SyncWatchDog();

	///@param[in] contentIdent identity of the object
	///@param[in] timeout the timeout to wake up timer to check the specified object
	void watch(GBVSSSessionGroup* group, ::Ice::Long syncInterval = 0);

	//quit watching
	void quit();

protected:
	int		run();
	//used for third party to stop this thread
	int		terminate(int code /* = 0 */);

private:
	ZQ::common::Log& _log;
	typedef std::multimap <GBVSSSessionGroup*, Ice::Long > SyncMap; // sessId to expiration map
	ZQ::common::Mutex   _lockGroups;
	SyncMap				_groupsToSync;
	bool	_bRun;
	SYS::SingleObject	m_Event;
};


}}//end of namespace

#endif //__GBSession_H__

