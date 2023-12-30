// File Name : StreamEventSinkI.cpp

#include "StreamEventSinkI.h"
#include "OpenVBOConfig.h"
#include "Environment.h"
#include "CRGSessionManager.h"
#include "RtspRelevant.h"

#define ANNOUNCELOG _fileLog

typedef struct _ErrorDescArray 
{
	char*	errorCodeStr;
	char*	errorDescStr;
}ErrorDescArray;

static ErrorDescArray errors[]=
{
	{"",""},
	{OPENVBO_ANNOUNCE_INTERNAL_SERVER_ERROR,		OPENVBO_ANNOUNCE_INTERNAL_SERVER_ERROR_STRING },
	{OPENVBO_ANNOUNCE_ERROR_READING_CONTENT,		OPENVBO_ANNOUNCE_ERROR_READING_CONTENT_STRING },
	{OPENVBO_ANNOUNCE_DOWNSTREAM_FAILURE,			OPENVBO_ANNOUNCE_DOWNSTREAM_FAILURE_STRING },
	{OPENVBO_ANNOUNCE_BANDWIDTH_EXCEEDED_LIMIT,		OPENVBO_ANNOUNCE_BANDWIDTH_EXCEEDED_LIMIT_STRING	},
	{OPENVBO_ANNOUNCE_DOWNSTREAM_UNREACHABLE,		OPENVBO_ANNOUNCE_DOWNSTREAM_UNREACHABLE_STRING }
};

static std::string generatorNoticeString(const std::string& strNoticeCode, 
										 const std::string& strNoticeString, 
										 const std::string& strEventDate)
{
	return strNoticeCode + " \"" + strNoticeString + "\" " + "Event-date=" + strEventDate;
}

namespace EventISVODI5
{

class SmartServerRequest
{
public: 
	SmartServerRequest(IServerRequest*& pServerRequest);
	virtual ~SmartServerRequest();

private:
	IServerRequest*& _pServerRequest;
};

EventSinkI::EventSinkI( ZQ::common::Log& fileLog, Environment& env, StreamEventDispatcher& eventDispatcher  )
:_fileLog(fileLog),
_env(env),
mEventDispatcher(eventDispatcher)
{

}

EventSinkI::~EventSinkI()
{

}

void EventSinkI::sendANNOUNCE(const ::std::string& proxy, const ::std::string& uid, 
							  OpenVBOStreamEventType openVBOEventType, 
							  TianShanIce::Properties extendProps, const ::Ice::Current& ic)const
{
	//Environment& env, ZQ::common::NativeThreadPool& pool, const ::std::string& proxy, const ::std::string& uid, OpenVBOStreamEventType openVBOEventType, TianShanIce::Properties& extendProps)
	mEventDispatcher.pushEvent(proxy, uid, openVBOEventType, extendProps);
}


StreamEventSinkI::StreamEventSinkI(ZQ::common::Log& fileLog, Environment& env, StreamEventDispatcher& eventDispatcher)
:EventSinkI(fileLog, env, eventDispatcher)
{
}

StreamEventSinkI::~StreamEventSinkI()
{
	
}

void StreamEventSinkI::ping(::Ice::Long lv, const ::Ice::Current& ic)
{
}

//bool StreamEventSinkI::getSessionContext(const std::string& sessId, SsmOpenVBO::CRGSessionPrx& sessionProxy, TianShanIce::Properties& sessionContext) const
//{
//	// find session proxy 
//	int statusCode;
//	std::string strLastError;
//	sessionProxy = _env.getSessionManager().findSession(sessId, strLastError, statusCode);
//	if (NULL == sessionProxy)
//	{
//		return false;
//	}
//
//	try
//	{
//		sessionContext = sessionProxy->getMetaData();
//		_fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkI, "successed to get session[%s] context"), sessId.c_str());
//	}
//	catch (const Ice::Exception& ex)
//	{
//		_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(RequestHandler, "get session[%s] context caught[%s]"), sessId.c_str(), ex.ice_name().c_str());
//		return false;
//	}
//	return true;
//}

void StreamEventSinkI::OnBeginningOfStream(const ::std::string& proxy, const ::std::string& uid, const TianShanIce::Properties& props, const ::Ice::Current& ic)const
{
	//ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkI, "Event(Beginning-of-Stream): [%s]"), proxy.c_str());
	sendANNOUNCE(proxy, uid, ON_BEGINGNING_OF_STREAM, props, ic);
}

void StreamEventSinkI::OnSpeedChanged(const ::std::string& proxy, const ::std::string& uid, ::Ice::Float prevSpeed, ::Ice::Float currentSpeed, const TianShanIce::Properties& props, const ::Ice::Current& ic)const
{
	//ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkI, "Event(Scale Changed): [%s] from [%f] to[%f]"), proxy.c_str() , prevSpeed , currentSpeed );
	sendANNOUNCE(proxy, uid, ON_SPEED_CHANGED, props, ic);
}

void StreamEventSinkI::OnStateChanged(const ::std::string& proxy, const ::std::string& uid, ::TianShanIce::Streamer::StreamState prevState, ::TianShanIce::Streamer::StreamState curState, const TianShanIce::Properties& props, const ::Ice::Current& ic/* = ::Ice::Current */)const
{
	//ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkI, "Event(State Changed): [%s]"), proxy.c_str());
	sendANNOUNCE(proxy, uid, ON_STATE_CHANGED, props, ic);
}

void StreamEventSinkI::OnEndOfStream(const ::std::string& proxy, const ::std::string& uid, const TianShanIce::Properties& props, const ::Ice::Current& ic)const
{
	//ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkI, "Event(End-of-Stream): [%s]"), proxy.c_str());
	sendANNOUNCE(proxy, uid, ON_END_OF_STREAM, props, ic);
}

void StreamEventSinkI::OnExit(const ::std::string& proxy, const ::std::string& uid, ::Ice::Int nExitCode, const ::std::string& sReason,  const ::Ice::Current& ic /*= ::Ice::Current()*/)const
{
	//ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkI, "Event(Exit-of-Stream): [%s]"), proxy.c_str());
}

void StreamEventSinkI::OnExit2(const ::std::string& proxy, const ::std::string&, ::Ice::Int nExitCode, const ::std::string& sReason, const TianShanIce::Properties&, const ::Ice::Current&)const
{
}

void StreamEventSinkI::sendANNOUNCE_SessionInProgress(SsmOpenVBO::CRGSession &sessionContext)
{
	// only send to STB
	_fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkI, "sendANNOUNCE_SessionInProgress() Session[%s]"), sessionContext.ident.name.c_str());
	if (0 == _openVBOConfig._announce._SRMEnabled)
		return;

	// rtsp announce header
	std::string responseHead = "ANNOUNCE " + sessionContext.requestURL + " RTSP/1.0";

	// CSeq header
	std::string sequence;
	try
	{
		sequence = (_openVBOConfig._announce._useGlobalCSeq > 0) ? _env.getAnnounceSequence() : sessionContext.getAnnounceSeq();
	}
	catch (const Ice::Exception& ex)
	{
		ANNOUNCELOG(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEventSinkI, "sendANNOUNCE_SessionInProgress() session[%s] assign cseq caught exception[%s]"), sessionContext.ident.name.c_str(), ex.ice_name().c_str());
		return;
	}
	catch (...)
	{
		ANNOUNCELOG(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEventSinkI, "sendANNOUNCE_SessionInProgress() session[%s] assign cseq caught exception"), sessionContext.ident.name.c_str());
		return;
	}

	std::string currentDate = _env.getUTCTime();
	std::string strSRMNotice = generatorNoticeString(OPENVBO_ANNOUNCE_SESSION_IN_PROCESS, OPENVBO_ANNOUNCE_SESSION_IN_PROCESS_STRING, currentDate);

	std::vector<std::string> srmConnections;
	std::string srmId = sessionContext.metadata[SESSION_META_DATA_SRM_ID];
#if 1 // to stay with the existing behavior though it was a bug
	_fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkI, "sendANNOUNCE_SessionInProgress() sess[%s] resetting SRM[%s] to associate conns due to broadcast is enabled"), sessionContext.ident.name.c_str(), srmId.c_str());
	srmId = "";
#endif

	_env.listSRMConns(srmConnections, srmId); // _env.getSRMConnectionIDs(srmConnections);
	_fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkI, "sendANNOUNCE_SessionInProgress() sess[%s] associated %d conns by SRM[%s]"), sessionContext.ident.name.c_str(), srmConnections.size(), srmId.c_str());

	std::vector<std::string>::iterator iter = srmConnections.begin();
	for (; iter != srmConnections.end(); iter++)
	{
		std::string& SRMConnId = *iter;
		IServerRequest* SRMAnnounce = _env.getStreamSmithSite().newServerRequest(sessionContext.ident.name.c_str(), SRMConnId);
		if (!SRMAnnounce)
		{
			_fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEventSinkI, "sendANNOUNCE_SessionInProgress() session[%s] failed to create SRM ANNOUNCE Request on conn[%s]"), sessionContext.ident.name.c_str(), SRMConnId.c_str());
			continue;
		}

		SRMAnnounce->printCmdLine((char*)responseHead.c_str());
		SRMAnnounce->printHeader(HeaderSequence, (char*)sequence.c_str());
		SRMAnnounce->printHeader(HeaderSession, (char*)sessionContext.ident.name.c_str());
		SRMAnnounce->printHeader(HeaderNotice, (char*)strSRMNotice.c_str());
		SRMAnnounce->printHeader(HeaderServer, ZQ_COMPONENT_NAME);
		SRMAnnounce->printHeader("Date", (char*)currentDate.c_str());
		SRMAnnounce->post();
		ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkI, "session[%s] Event(Session-In-Progress) has been posted to SRM[%s] thru conn[%s]"), sessionContext.ident.name.c_str(), srmId.c_str(), SRMConnId.c_str());
	}
}

void StreamEventSinkI::sendANNOUNCE_Terminated(SsmOpenVBO::CRGSession& sessionContext)
{
	if (0 == _openVBOConfig._announce._SRMEnabled && 0 == _openVBOConfig._announce._STBEnabled)
	{
		ANNOUNCELOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkI, "sendANNOUNCE_Terminated() skipped per configuration"));
		return;
	}

	_fileLog(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkI, "sendANNOUNCE_Terminated() Session[%s]"), sessionContext.ident.name.c_str());

	// rtsp announce header
	std::string responseHead = "ANNOUNCE " + sessionContext.requestURL + " RTSP/1.0";

	// CSeq header
	std::string sequence;
	try
	{
		sequence = (_openVBOConfig._announce._useGlobalCSeq > 0) ? _env.getAnnounceSequence() : sessionContext.getAnnounceSeq();
	}
	catch (const Ice::Exception& ex)
	{
		ANNOUNCELOG(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEventSinkI, "sendANNOUNCE_Terminated() session[%s] assign cseq caught exception[%s]"), sessionContext.ident.name.c_str(), ex.ice_name().c_str());
		return;
	}
	catch (...)
	{
		ANNOUNCELOG(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEventSinkI, "sendANNOUNCE_Terminated() session[%s] assign cseq caught exception"), sessionContext.ident.name.c_str());
		return;
	}

	// Date 
	std::string currentDate = _env.getUTCTime();

	// Notice header
	std::string strNotice = generatorNoticeString(OPENVBO_ANNOUNCE_TERMINATE_SESSION, OPENVBO_ANNOUNCE_TERMINATE_SESSION_STRING, currentDate);
	// std::string strSTBNotice = generatorNoticeString(OPENVBO_ANNOUNCE_STB_TERMINATE_SESSION, OPENVBO_ANNOUNCE_STB_TERMINATE_SESSION_STRING, currentDate);

	if (_openVBOConfig._announce._SRMEnabled > 0)
	{
		/// send SRM Announce
		std::vector<std::string> srmConnections;
		std::string srmId = sessionContext.metadata[SESSION_META_DATA_SRM_ID];
#if 1 // to stay with the existing behavior though it was a bug
		_fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventSinkI, "sendANNOUNCE_Terminated() sess[%s] resetting SRM[%s] to associate conns due to broadcast is enabled"), sessionContext.ident.name.c_str(), srmId.c_str());
		srmId = "";
#endif

		_env.listSRMConns(srmConnections, srmId); // _env.getSRMConnectionIDs(srmConnections);
		std::vector<std::string>::iterator iter = srmConnections.begin();
		for (; iter != srmConnections.end(); iter++)
		{
			std::string& SRMConnId = *iter;
			IServerRequest* SRMAnnounce = _env.getStreamSmithSite().newServerRequest(sessionContext.ident.name.c_str(), SRMConnId);
			SmartServerRequest smartSRMAnnounce(SRMAnnounce);
			if (!SRMAnnounce)
			{
				ANNOUNCELOG(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEventSinkI, "session[%s] failed to create SRM ANNOUNCE Request"), sessionContext.ident.name.c_str());
			}
			else
			{
				SRMAnnounce->printCmdLine((char*)responseHead.c_str());
				SRMAnnounce->printHeader(HeaderSequence, (char*)sequence.c_str());
				SRMAnnounce->printHeader(HeaderSession, (char*)sessionContext.ident.name.c_str());
				SRMAnnounce->printHeader(HeaderNotice, (char*)strNotice.c_str());
				SRMAnnounce->printHeader(HeaderServer, ZQ_COMPONENT_NAME);
				SRMAnnounce->printHeader("Date", (char*)currentDate.c_str());
				SRMAnnounce->post();
				ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkI, "session[%s] Event(Terminated) has been posted to SRM"), sessionContext.ident.name.c_str());
			}
		}
	}

	if (_openVBOConfig._announce._STBEnabled > 0)
	{
		/// send STB Annnounce
		std::string STBConnId = sessionContext.metadata["STBConnectionID"];
		IServerRequest* STBAnnounce = _env.getStreamSmithSite().newServerRequest(sessionContext.ident.name.c_str(), STBConnId);
		SmartServerRequest smartSTBAnnounce(STBAnnounce);
		if (!STBAnnounce)
		{
			ANNOUNCELOG(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEventSinkI, "session[%s] failed to create STB ANNOUNCE Request"), sessionContext.ident.name.c_str());
		}
		else
		{
			STBAnnounce->printCmdLine((char*)responseHead.c_str());
			STBAnnounce->printHeader(HeaderSequence, (char*)sequence.c_str());
			STBAnnounce->printHeader(HeaderSession, (char*)sessionContext.ident.name.c_str());
			STBAnnounce->printHeader(HeaderNotice, (char*)strNotice.c_str());
			STBAnnounce->printHeader(HeaderServer, ZQ_COMPONENT_NAME);
			STBAnnounce->printHeader("Date", (char*)currentDate.c_str());
			STBAnnounce->post();
			ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventSinkI, "session[%s] Event(Terminated) has been posted to STB"), sessionContext.ident.name.c_str());
		}
	}
}


PlayListEventSinkI::PlayListEventSinkI(ZQ::common::Log& fileLog, Environment& env, StreamEventDispatcher& eventDispatcher)
:EventSinkI(fileLog, env, eventDispatcher)
{

}

PlayListEventSinkI::~PlayListEventSinkI()
{

}

void PlayListEventSinkI::OnItemStepped(const ::std::string& proxy, const ::std::string& playlistId, ::Ice::Int currentUserCtrlNum, ::Ice::Int prevUserCtrlNum, const ::TianShanIce::Properties& ItemProps, const ::Ice::Current& ic /* = ::Ice::Current */) const
{
/*
	MLOG(ZQ::common::Log::L_DEBUG, CLOGFMT(PlaylistEventI, "OnItemStepped() [%s]"), playlistId.c_str());
	if (ngodConfig.sessionHistory.enableHistory <= 0 && ngodConfig.announce.includeTransition <= 0
		&& ngodConfig.announce.useTianShanAnnounceCodeScaleChanged <= 0) // because "lastScale" updated per ItemStepped maybe referenced in scaleChanged announce
		return; 

	StreamEventAttr a;	initStreamEventAttr(a);
	a.proxyString		= proxy;
	a.playlistString	= playlistId;
	a.props				= ItemProps;
	a.prevCtrlNum		= prevUserCtrlNum;
	a.currentCtrlNum	= currentUserCtrlNum;

	ZQTianShan::Util::getPropertyDataWithDefault(ic.ctx,"EventSeq",-1,a.eventIndex);
	bool bPerUserRequest = false;
	{
		std::string tmpstr;
		ZQTianShan::Util::getPropertyDataWithDefault(ItemProps, "ItemExitReason", "", tmpstr);
		std::transform(tmpstr.begin(), tmpstr.end(), tmpstr.begin(), tolower);
		if (std::string::npos != tmpstr.find("userreq"))
			bPerUserRequest = true;
	}
	if(bPerUserRequest)
		a.props["reason"] = "USER";
	else
		a.props["reason"] = "SERVER";

	sendEvent( streamEventITEMSTEP , a );
*/

	::TianShanIce::Properties props = ItemProps;
	bool bPerUserRequest = false;
	{
		std::string tmpstr;
		ZQTianShan::Util::getPropertyDataWithDefault(ItemProps, "ItemExitReason", "", tmpstr);
		std::transform(tmpstr.begin(), tmpstr.end(), tmpstr.begin(), tolower);
		if (std::string::npos != tmpstr.find("userreq"))
			bPerUserRequest = true;
	}
	if(bPerUserRequest)
		props["reason"] = "USER";
	else
		props["reason"] = "SERVER";

	char buf[10];
	itoa(prevUserCtrlNum, buf, 10);
	std::string prevUserCtrlNumStr = buf;

	itoa(currentUserCtrlNum, buf, 10);
	std::string currentUserCtrlNumStr = buf;

	props["currentUserCtrlNum"] = currentUserCtrlNumStr;
	props["prevUserCtrlNum"]	= prevUserCtrlNumStr;

	sendANNOUNCE(proxy, playlistId, ON_ITEM_STEP, props, ic);
}

void PlayListEventSinkI::ping(::Ice::Long lv, const ::Ice::Current& ic /* = ::Ice::Current */)
{

}

SmartServerRequest::SmartServerRequest(IServerRequest*& pServerRequest) : _pServerRequest(pServerRequest)
{
}

SmartServerRequest::~SmartServerRequest()
{
	if (NULL != _pServerRequest)
	{
		_pServerRequest->release();
	}
	_pServerRequest = NULL;
}

StreamEventDispatchRequest::StreamEventDispatchRequest(Environment& env, ZQ::common::NativeThreadPool& pool, const std::string& proxy, const std::string& uid, OpenVBOStreamEventType openVBOEventType, TianShanIce::Properties extendProps, ZQ::common::Log& fileLog)
:_env(env),
_proxy(proxy),
_uid(uid),
_openVBOEventType(openVBOEventType),
_props(extendProps),
_fileLog(fileLog),
ZQ::common::ThreadRequest(pool)
{
}

StreamEventDispatchRequest::~StreamEventDispatchRequest()
{

}
int	StreamEventDispatchRequest::run()
{
	if (0 == _openVBOConfig._announce._SRMEnabled && 0 == _openVBOConfig._announce._STBEnabled)
	{
		// ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventDispatch, "disable send stream event to SRM and STB"));
		return -1;
	}

	std::vector< Ice::Identity > idents = _env.getSessionManager().findStreams(_uid, 1);
	if (idents.size() == 0)
	{
		// ANNOUNCELOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventDispatch, "unknown streamID[%s], ignore the event"), uid.c_str());
		return -1;
	}

	// get session context
	TianShanIce::Properties sessionContext;
	SsmOpenVBO::CRGSessionPrx sessionProxy = _env.getSessionManager().getSessionContext(idents[0].name, sessionContext);
	if (NULL == sessionProxy)
	{
		ANNOUNCELOG(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEventDispatch, "failed to get session[%s] context for stream[%s]"), idents[0].name.c_str(), _uid.c_str());
		return -1;
	}

	// rtsp announce header
	std::string responseHead = "ANNOUNCE " + sessionContext[SESSION_META_DATA_REQUEST_URL] + " RTSP/1.0";

	// CSeq header
	std::string sequence;
	try
	{
		sequence = (_openVBOConfig._announce._useGlobalCSeq > 0) ? _env.getAnnounceSequence() : sessionProxy->getAnnounceSeq();
	}
	catch (const Ice::Exception& ex)
	{
		ANNOUNCELOG(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEventDispatch, "caught [%s] when got announce CSeq from session[%s]"), ex.ice_name().c_str(), idents[0].name.c_str());
		return -1;
	}

	// Data header
	std::string currentDate = _env.getUTCTime();

	std::string strEvent;
	bool toSRM =false, toSTB =false;

	// Notice header
	std::string strNotice;
	switch (_openVBOEventType)
	{
	case ON_END_OF_STREAM:
		toSTB = true;
		strNotice = generatorNoticeString(OPENVBO_ANNOUNCE_ENDOFSTREAM, OPENVBO_ANNOUNCE_ENDOFSTREAM_STRING, currentDate);
		strEvent = "End-of-Stream";
		break;

	case ON_BEGINGNING_OF_STREAM :
		toSTB = true;
		strNotice = generatorNoticeString(OPENVBO_ANNOUNCE_BEGINOFSTREAM, OPENVBO_ANNOUNCE_BEGINOFSTREAM_STRING, currentDate);
		strEvent = "Beginning-of-Stream";
		break;

	case ON_SPEED_CHANGED:
		toSTB = true;
		if (_openVBOConfig._announce._sendTianShanScaleChangeAnnounce <= 0)
			return 0;
		else
		{
			strNotice = generatorNoticeString(OPENVBO_ANNOUNCE_SCALE_CHANGE, OPENVBO_ANNOUNCE_SCALE_CHANGE_STRING, currentDate);
			strEvent = "On-Speed-Change";
			break;
		}

	case ON_STATE_CHANGED:
		toSTB = true;
		if (_openVBOConfig._announce._sendTianShanStateChangeAnnounce <= 0)
			return 0;

		strNotice = generatorNoticeString(OPENVBO_ANNOUNCE_STATE_CHANGE, OPENVBO_ANNOUNCE_STATE_CHANGE_STRING, currentDate);
		strEvent = "On-State-Changed";
		break;

	case ON_PAUSE_TIMEOUT:
		if (_openVBOConfig._announce._sendPauseTimeout <= 0 )
			return 0;

		strNotice = generatorNoticeString(OPENVBO_ANNOUNCE_PAUSE_TIMEOUT, OPENVBO_ANNOUNCE_PAUSE_TIMEOUT_STRING, currentDate);
		strEvent = "Pause-Timeout";
		break;

	case ON_ITEM_STEP:
		toSTB = true;
		{
			/*std::string currentUserCtrlNumStr;
			std::string prevUserCtrlNumStr;
			ZQTianShan::Util::getPropertyDataWithDefault(_props, "currentUserCtrlNum", "0", currentUserCtrlNumStr);
			ZQTianShan::Util::getPropertyDataWithDefault(_props, "prevUserCtrlNum", "0", prevUserCtrlNumStr);

			int currentUserCtrlNum	= atoi(currentUserCtrlNumStr.c_str());
			int prevUserCtrlNum		= atoi(prevUserCtrlNumStr.c_str());*/

			Ice::Int errorCode = 0;
			ZQTianShan::Util::getPropertyDataWithDefault( _props, "ItemSkipErrorCode", 0 , errorCode );
			if ( errorCode <= 0 || errorCode >= (sizeof(errors)/sizeof(errors[0])) )
				return 0;

			strNotice = generatorNoticeString(errors[errorCode].errorCodeStr, errors[errorCode].errorDescStr, currentDate);
			strEvent = "On-Item-Step";
		}

	case ON_EXIT:
		toSRM =true;
		strEvent = "On-Exit";
		break;

	case ON_EXIT2:
		toSRM =true;
		strEvent = "On-Exit";
		break;

	default:
		toSRM =true;
		strEvent = "unknown-Event";
		break;
	}

	// Date
	if (_openVBOConfig._announce._SRMEnabled > 0)
	{
		/// send SRM Announce
		std::vector<std::string> srmConnections;
		std::string srmId = sessionContext[SESSION_META_DATA_SRM_ID];
#if 1 // to stay with the existing behavior though it was a bug
		_fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventDispatch, "sess[%s] event[%s] resetting SRM[%s] to associate conns due to broadcast is enabled"), idents[0].name.c_str(), strEvent.c_str(), srmId.c_str());
		srmId = "";
#endif
		_env.listSRMConns(srmConnections, srmId); // _env.getSRMConnectionIDs(srmConnections);
		ANNOUNCELOG(ZQ::common::Log::L_DEBUG, CLOGFMT(StreamEventDispatch, "sess[%s] event[%s] associated %d conns by SRM[%s]"), idents[0].name.c_str(), strEvent.c_str(), srmConnections.size(), srmId.c_str());

		std::vector<std::string>::iterator iter = srmConnections.begin();
		for (; iter != srmConnections.end(); iter++)
		{
			//std::string SRMConnId = sessionContext[SESSION_META_DATA_SRM_CONNECTION_ID];
			std::string& SRMConnId = *iter;
			IServerRequest* SRMAnnounce = _env.getStreamSmithSite().newServerRequest(idents[0].name.c_str(), SRMConnId);
			SmartServerRequest smartSRMAnnounce(SRMAnnounce);
			if (!SRMAnnounce)
			{
				ANNOUNCELOG(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEventSinkI, "sess[%s] event[%s] failed to create SRM ANNOUNCE Request on conn[%s]"), idents[0].name.c_str(), strEvent.c_str(), SRMConnId.c_str());
				continue;
			}

			SRMAnnounce->printCmdLine((char*)responseHead.c_str());
			SRMAnnounce->printHeader(HeaderSequence, (char*)sequence.c_str());
			SRMAnnounce->printHeader(HeaderSession, (char*)idents[0].name.c_str());
			SRMAnnounce->printHeader(HeaderNotice, (char*)strNotice.c_str());
			SRMAnnounce->printHeader(HeaderServer, ZQ_COMPONENT_NAME);
			SRMAnnounce->printHeader("Date", (char*)currentDate.c_str());
			SRMAnnounce->post();

			ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventDispatch, "sess[%s] event[%s] has been posted to SRM[%s] thru conn[%s]"), idents[0].name.c_str(), strEvent.c_str(), srmId.c_str(), SRMConnId.c_str());
		}
	}

	if (_openVBOConfig._announce._STBEnabled > 0)
	{
		/// send STB Annnounce
		std::string STBConnId = sessionContext[SESSION_META_DATA_STB_CONNECTION_ID];
		IServerRequest* STBAnnounce = _env.getStreamSmithSite().newServerRequest(idents[0].name.c_str(), STBConnId);
		SmartServerRequest smartSTBAnnounce(STBAnnounce);
		if (!STBAnnounce)
		{
			ANNOUNCELOG(ZQ::common::Log::L_ERROR, CLOGFMT(StreamEventDispatch, "sess[%s] event[%s] failed to create STB ANNOUNCE Request on conn[%s]"), idents[0].name.c_str(), strEvent.c_str(), STBConnId.c_str());
			return -1;
		}

		STBAnnounce->printCmdLine((char*)responseHead.c_str());
		STBAnnounce->printHeader(HeaderSequence, (char*)sequence.c_str());
		STBAnnounce->printHeader(HeaderSession, (char*)idents[0].name.c_str());
		STBAnnounce->printHeader(HeaderNotice, (char*)strNotice.c_str());
		STBAnnounce->printHeader(HeaderServer, ZQ_COMPONENT_NAME);
		STBAnnounce->printHeader("Date", (char*)currentDate.c_str());
		STBAnnounce->post();
		ANNOUNCELOG(ZQ::common::Log::L_INFO, CLOGFMT(StreamEventDispatch, "sess[%s] event[%s] has been posted to STB thru conn[%s]"), idents[0].name.c_str(), strEvent.c_str(), STBConnId.c_str());
	}

	return 0;
}

void StreamEventDispatchRequest::final(int retcode ,bool bCancelled)
{
	delete this;
}

StreamEventDispatcher::StreamEventDispatcher( ZQ::common::Log& fileLog, Environment& env)
:_fileLog(fileLog),_env(env)
{
	start();
}
StreamEventDispatcher::~StreamEventDispatcher()
{
	stop();
}

void StreamEventDispatcher::start()
{
	mPool.resize(10);
	ANNOUNCELOG(ZQ::common::Log::L_INFO,CLOGFMT(StreamEventDispatcher,"start StreamEventDispatcher with threadcount[%d]"), mPool.size());
}
void StreamEventDispatcher::stop( )
{
	mPool.stop();
}

void StreamEventDispatcher::pushEvent(const std::string& proxy, const std::string& uid, OpenVBOStreamEventType openVBOEventType, TianShanIce::Properties& extendProps)
{
	StreamEventDispatchRequest* req = new StreamEventDispatchRequest(_env, mPool, proxy, uid, openVBOEventType, extendProps, _fileLog);
	req->start();
}

//////////////////////////////////////////////////////////////////////////
///PauseTimeoutSinkI
PauseTimeoutSinkI::PauseTimeoutSinkI(ZQ::common::Log& fileLog, Environment& env, StreamEventDispatcher& eventDispatcher)
: EventSinkI(fileLog, env, eventDispatcher)
{
}

PauseTimeoutSinkI::~PauseTimeoutSinkI()
{
}

void PauseTimeoutSinkI::post(const ::std::string& category, ::Ice::Int eventId,  const ::std::string& eventName,
							   const ::std::string& stampUTC,  const ::std::string& sourceNetId,  const ::TianShanIce::Properties& params,
							   const ::Ice::Current& ic  )
{
	std::string playlistId; 
	ZQTianShan::Util::getPropertyDataWithDefault(params, "streamSessionId", "",  playlistId);
	ANNOUNCELOG(ZQ::common::Log::L_DEBUG, CLOGFMT(PauseTimeoutSinkI, "Event(%s) [%s]"), eventName.c_str(), playlistId.c_str());
	
	sendANNOUNCE("PauseTimeoutSinkI", playlistId, ON_PAUSE_TIMEOUT, params, ic);

	/*
	StreamEventAttr a;	initStreamEventAttr(a);
	a.playlistString	= playlistId;
	a.props				= params;
	a.props["reason"]   = "USER";
	ZQTianShan::Util::getPropertyDataWithDefault(ic.ctx, "EventSeq", -1,a.eventIndex);

	sendEvent( streamEventPauseTimeout , a );
	*/
}


} // end EventISVODI5