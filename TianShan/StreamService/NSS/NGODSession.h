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
// Ident : $Id: NGODSession.h $
// Branch: $Name:  $
// Author: Hui Shao
// Desc  : 
//
// Revision History: 
// ---------------------------------------------------------------------------
// $Log: /ZQProjs/TianShan/StreamService/NSS/NGODSession.h $
// 
// 28    5/07/15 9:58a Hui.shao
// 
// 25    5/06/15 4:54p Hui.shao
// tested drop request due to penalties
// ===========================================================================

#ifndef __NGODSession_H__
#define __NGODSession_H__

#include "RTSPClient.h"
#include "TianShanUtils.h"
#include "TsStreamer.h"
#include "SystemUtils.h"

#define SYNC_INTERVAL	(60*1000)
#define SESSION_TIMEOUT	(600*1000)

#define SESSION_GROUP		"NSS_Session_Group"
#define	SETUP_TIMESTAMP		"NSS_SetupTimestamp"
#define VOLUME_NAME			"NSS_Volume_Name"
#define ONDEMANDNAME_NAME	"NSS_OnDemandName"
#define DESTINATION_NAME	"NSS_Destination"
#define SESSION_ID			"NSS_SessionId"
#define CONTROL_URI			"NSS_ControlUri"
#define	SPEED_LASTIDX		"NSS_MultiplySpeedLastIndex"
#define	SPEED_LASTDIR		"NSS_MultiplySpeedLastDirection"
#define DESTROY_REASON		"user.destroy.reason"

namespace ZQTianShan {
namespace NGODSS {

using namespace ZQ::common;

class NGODClient;
class NGODSessionGroup;

class NSSEnv;

// -----------------------------
// NGODSession
// -----------------------------
class NGODSession : public RTSPSession
{
	friend class NGODSessionGroup;
	friend class NGODClient;

public:

	typedef struct _StreamInfos
	{
		int64        timeoffset;   ///< stream timeoffset in milliseconds
		int64        duration;	   ///< stream play duration in milliseconds
		float		 scale;        ///< stream scale
		std::string	 state;        ///< state
		_StreamInfos()
			:timeoffset(0),
			duration(0),
			scale(0.0f)
		{
		}
	} StreamInfos;

	typedef Pointer < NGODSession > Ptr;
	typedef std::vector < Ptr > List;

public:

	virtual ~NGODSession();

	virtual void	destroy();
	StreamInfos		getInfos() { return  _streamInfos; };
	std::string		getBaseURL();
	std::string		getSessionGroupName() const;
	void			setControlUri(std::string controlUri) { _controlUri = controlUri; };
	std::string     getSessionId() { return _sessionId; };
	void			setSessionId(std::string sessionId) { _sessionId = sessionId; };
	std::string     getOnDemandSessionId() { return _sessGuid; };
	int32			getTimeout() { return _sessTimeout; };
	NGODClient*		getR2Client();
	NGODClient*		getC1Client();


protected:

	NGODSession(NGODSessionGroup& group, const std::string& ODSessId, const std::string& streamDestUrl, const char* filePath=NULL);

	// overwrites of RTSPSession
	virtual void OnResponse_SETUP(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, ZQ::common::RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString);
	virtual void OnResponse_PLAY(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString);
	virtual void OnResponse_TEARDOWN(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString);
	virtual void OnResponse_PAUSE(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString);
	virtual void OnResponse_GET_PARAMETER(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString);
	virtual void OnResponse_SET_PARAMETER(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString);
	virtual void OnANNOUNCE(RTSPClient& rtspClient, RTSPMessage::Ptr& pInMessage);
	virtual void OnSessionTimer();

	// overwrite OnResponse() and OnRequestError() to signal the event
	virtual void OnResponse(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString);
	virtual void OnRequestError(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RequestError errCode, const char* errDesc=NULL);

	bool	parseStreamInfo(const RTSPMessage::Ptr& pResp);

public:

	std::string		_volumeName;
	TianShanIce::Properties		_props;	// store metadata if need
//	SessionEvent	_eventHandle;
	uint			_resultCode;
	std::string     _sessionHistory;
	std::string		_tianShanNotice;
	std::string     _primartItemNPT;

protected:
	NGODSessionGroup&	_group;
	StreamInfos			_streamInfos;

};

// -----------------------------
// class NGODClient
// -----------------------------
class NGODClient : public RTSPClient
{
	friend class NGODSessionGroup;
	friend class NGODSession;

public:

	class Event : public SharedObject // for the stupid naming of SingleObject
	{
	public: 
		typedef Pointer < Event > Ptr;

		Event(): _bSuccess(false) {}

		SYS::SingleObject::STATE wait(timeout_t timeout=TIMEOUT_INF) { return _so.wait(timeout); }
		void signal(bool success=true) { if (!_bSuccess) _bSuccess = success; _so.signal(); }
		bool isSuccess() const { return _bSuccess; }

	protected:
		SYS::SingleObject _so;
		bool _bSuccess;
	};

	typedef struct Session_Pair
	{
		std::string _sessionId;         ///< rtsp-session-id
		std::string _onDemandSessionId; ///< on-demand-session-id
	} Session_Pair;

	typedef enum
	{
		NC_R2, NC_C1
	} ClientType;

	class FindByOnDemandSessionID
	{
	public:
		FindByOnDemandSessionID(std::string& strOnDemandSessionID):m_strOnDemandSessionID(strOnDemandSessionID){}

		bool operator() (const Session_Pair& pSession_list)
		{
			return (0 == m_strOnDemandSessionID.compare(pSession_list._onDemandSessionId));
		}

	private:

		const std::string& m_strOnDemandSessionID;
	};

public:

	NGODClient(NGODSessionGroup& group, ClientType type, Log& log, NativeThreadPool& thrdpool, InetHostAddress& bindAddress, const std::string& baseURL, const char* userAgent =NULL, Log::loglevel_t verbosityLevel =Log::L_WARNING, tpport_t bindPort =0);
	virtual ~NGODClient();

public:

	int		sendTousyMsg(const std::string& startLine, RTSPMessage::AttrMap& headers, const std::string& body=std::string(), const int cseq =-1);
	bool    waitForResponse(uint32 cseq);

	static  bool _bTryDecimalNpt;
	const char*  requireString();
	static long  parseNptValue(const char* nptFieldStr);
	std::string  formatNptValue(long msec, const char* ngodRequire =NULL);

	uint32       getPenalty();

protected:

	virtual void OnResponse(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RTSPMessage::Ptr& pResp, uint resultCode, const char* resultString);
	virtual void OnServerRequest(RTSPClient& rtspClient, const char* cmdName, const char* url, RTSPMessage::Ptr& pInMessage);
	virtual void OnRequestError(RTSPClient& rtspClient, RTSPRequest::Ptr& pReq, RequestError errCode, const char* errDesc=NULL);
	virtual void OnConnected();
	virtual void OnError();

	virtual int  OnRequestPrepare(RTSPRequest::Ptr& pReq);
	virtual void OnRequestClean(RTSPRequest& req);

	void    wakeupByCSeq(uint32 cseq, bool success=true);

	ClientType        _type;
	NGODSessionGroup& _group;
	uint			  _cContinuousTimeoutInConn;
	uint			  _cContinuousTimeout;
	int64             _stampLastRespInTime;

	bool              _bDecimalNpt, _bNptHandshaked;

	typedef std::map<uint32, Event::Ptr> EventMap;
	EventMap          _eventMap;
	ZQ::common::Mutex _lkEventMap;
};

// -----------------------------
// class NGODSessionGroup
// -----------------------------
class NGODSessionGroup : virtual public TianShanUtils::TimeoutObj
{
	friend class NGODClient;
	friend class NGODSession;

public:

	typedef ::IceInternal::Handle< NGODSessionGroup > Ptr;
	typedef std::map<std::string, Ptr> SessionGroupMap; // map of groupName to SessionGroup
	typedef std::multimap<std::string, std::string> StringIndex;  
	typedef std::pair<StringIndex::iterator, StringIndex::iterator> StringIdxRange;  

	typedef std::map<std::string, NGODSession::Ptr> SessionMap; // Map of ODSess to session
	typedef std::map<std::string, NGODClient*> NGODClientMap; // controlURL to RTSPClient map
	
	typedef enum _SessionGroupStatus
	{
		Sync = 1,
		Idle = 2
	} SessionGroupStatus;

	NGODSessionGroup(NSSEnv& env, const std::string& name, const std::string& baseURL, int max =600, Ice::Long syncInterval =60000);
	virtual ~NGODSessionGroup();

	virtual int     sync(bool bOnConnected =false); // the response would be handled by OnSessionListOfSS()
	virtual void    OnSessionListOfSS(const std::vector<NGODClient::Session_Pair>& listOnSS);

	size_t			size() { return _sessMap.size(); }
	int				getMaxSize() { return _max; };
	void			setMaxSize(int max) { _max = max; }
	
	SessionGroupStatus	getStatus() { return _status; }
	void            setStatus(SessionGroupStatus st) { _status = st; }

	std::string     getName() const { return _name; }
	std::string     getBaseURL() const { return _baseURL; }
	Ice::Long		getSyncInterval() const { return _syncInterval; }
	Ice::Long		getLastSync() { MutexGuard g(_lockSyncTimeStamp); return _stampLastSync; }
	void			setLastSync(Ice::Long time, SessionGroupStatus st) { MutexGuard g(_lockSyncTimeStamp); _stampLastSync = time; _status = st; }
			
	virtual void OnTimer(const ::Ice::Current& = ::Ice::Current());

	virtual void add(NGODSession& sess);
	virtual void remove(NGODSession& sess);

	virtual NGODSession::Ptr lookupByOnDemandSessionId(const char* ODSessId);
	virtual void updateIndex(NGODSession& sess, const char* indexName="SessionId", const char* oldValue=NULL);
	virtual NGODSession::List lookupByIndex(const char* sessionId, const char* indexName="SessionId");

	void eraseSessionId(const std::string& sessionId, NGODSession& sess);

	NGODClient* getC1Client(const std::string& controlURL); // to select one RTSPClient
	NGODClient* getR2Client();

	static NGODSessionGroup::Ptr allocateSessionGroup(const std::string& baseURL); // to select one session group
	static NGODSessionGroup::Ptr findSessionGroup(const std::string& name); // to find session group
	static std::vector<std::string> getAllSessionGroup(); // return all group name

	static void clearSessionGroup();
	static Ice::Long checkAll();
	
	static NGODSession::Ptr createSession(const std::string& ODSessId, const std::string& strmDestUrl, const char* baseURL=NULL);
	static NGODSession::Ptr openSession(const std::string& ODSessId, const std::string& groupName, bool bRestore =false);

protected:

	void clearNGODClient();

protected:

	NSSEnv&      _env;

	std::string  _name;
	std::string  _baseURL;

	int                 _max;
	SessionGroupStatus  _status;
	::Ice::Long         _syncInterval;
	::Ice::Long         _stampLastSync;

private:
	
	NGODClient				_R2Client;
	NGODClientMap			_C1ClientMap;
	Mutex					_lockClients;

	friend class NSSEnv;

	static SessionGroupMap	_groupMap;
	static StringIndex   	_idxGroupBaseUrl;
	static Mutex			_lockGroups;

	StringIndex  			_sessIdIndex; // C1 sessionId to OnDemandSessionId
	SessionMap				_sessMap;     // store all sessions of group
	Mutex					_lockSessMap; // map of OnDemandSessionId to NGODSession of the group

	Mutex					_lockSyncTimeStamp;
};

}}//end of namespace

#endif //__NGODSession_H__

