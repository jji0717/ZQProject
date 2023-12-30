// **********************************************************************
//
//
// **********************************************************************

#include <winsock2.h>
#include "NSSImpl.h"
#include <IceUtil/UUID.h>
#include "ngod_rtsp_action.h"
#include "../NSSRtspSession/ngod_rtsp_parser/ClientSocket.h"
#include "IPathHelperObj.h"
#include <sstream>
#include "NSSCommitThrd.h"

namespace ZQTianShan{
namespace NSS{

static string strCategory = "NSS";
/***************
//NGODStreamImpl
****************/
NGODStreamImpl::
NGODStreamImpl(NSSEnv &env):
_env(env),
_nssSessionGroupList(env._sessionGroupList),
_logFile(env._logFile),
_pool(env._thpool)
{

}

NGODStreamImpl::~NGODStreamImpl()
{
	_nssSessionGroupList = NULL;
	//envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODStreamImpl,"NGODStreamImpl::De-construct() successfully destroy sesion(%s)"), ident.name.c_str());
	//_rtspClientSession = NULL;
}

void NGODStreamImpl::renewPathTicket()
{
	::Ice::Identity tmpident = _env._communicator->stringToIdentity(pathTicketStr);
	::TianShanIce::Transport::PathTicketPrx& _pathTicketPrx = ::TianShanIce::Transport::PathTicketPrx::uncheckedCast(_env._adapter->createProxy(tmpident));
	int32 iTime = GetTickCount();
	try
	{
		if(_pathTicketPrx!=NULL)
		{
			_pathTicketPrx->renew(iTime);//renew path ticket
		}
		//renewStatusReport(RENEW_OK); //if renew ok, just continue the rest job
	}
	catch (::TianShanIce::InvalidParameter&) 
	{
		//here is just a log message 
		envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(renewPathTicket,"session(%s) invalid renew time =%d"), ident.name.c_str(),iTime);
	}
	catch(Ice::ObjectNotExistException& ex)
	{
		//if the relative path ticket is not available , kill playlist
		envlog(::ZQ::common::Log::L_ERROR,CLOGFMT(renewPathTicket,"session(%s) renew fail with renew time =%d and error is %s"), ident.name.c_str(), iTime,ex.ice_name().c_str());
		::Ice::Current c;
		try
		{
			destroy(c);
			delete this;
		}
		catch(...)
		{
			envlog(::ZQ::common::Log::L_ERROR,CLOGFMT(renewPathTicket,"fail to destroy session(%s) after renew pathticker find object not exist"), ident.name.c_str());
		}
		
		//renewStatusReport(RENEW_NOOBJECT); //this call will issue a thread and kill the playlist
	}
	catch (Ice::ConnectionLostException&) 
	{
		//just do nothing
		//renewStatusReport(RENEW_NOCONNECT);
	}
	catch (Ice::ConnectionRefusedException& ) 
	{
		//just do nothing
		//renewStatusReport(RENEW_NOCONNECT);
	}
	catch (Ice::Exception& ex)
	{
		//only record a log message
		envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(renewPathTicket,"session(%s) renew failed with ice exception is %s"), ident.name.c_str(), ex.ice_name().c_str());
	}
	catch(...)
	{
		//something wrong , and of course record a log message
		envlog(::ZQ::common::Log::L_ERROR,CLOGFMT(renewPathTicket,"session(%s) renew fail with time =%d and unexpect error"), ident.name.c_str(), iTime);
		//renewStatusReport(RENEW_NOCONNECT);
	}
}

void NGODStreamImpl::
setRtspClientSession(RTSPClientSession *rtspClientSession)
{
	RTSPTransportUdpHeader pHeader = rtspClientSession->m_RTSPR2Header.m_RTSPTransportHeader.TransportUdpHeader.front();
	//set variables for db
	volume = rtspClientSession->m_RTSPR2Header.Volume.strName;

	for (RTSPTransportUdpHeaderVec::iterator iter = rtspClientSession->m_RTSPR2Header.m_RTSPTransportHeader.TransportUdpHeader.begin();
	iter != rtspClientSession->m_RTSPR2Header.m_RTSPTransportHeader.TransportUdpHeader.end();
	iter++)
	{
		::TianShanIce::Streamer::NGODStreamServer::TransHeader header;
		header.clientId = (*iter).strClient_id;
		header.destination = (*iter).strDestination;
		header.clientPort = (*iter).strClient_port;
		header.sopName = (*iter).strSop_name;
		header.bandWidth = (*iter).strBandwidth;

		tansportHeaderList.push_back(header);
	}

	WLock sync(*this);
	//IceUtil::Mutex::Lock lock(*this);
	for (SDPRequestContentVec::iterator pIter = rtspClientSession->m_RTSPR2Header.m_SDPRequestContent.begin();
	pIter != rtspClientSession->m_RTSPR2Header.m_SDPRequestContent.end();
	pIter++)
	{
		::TianShanIce::Streamer::NGODStreamServer::stItem tmpItem;
		tmpItem.assetId = (*pIter).strAsset_id;
		tmpItem.itemNum = (*pIter).iCtrlNum;
		assetId.push_back(tmpItem);
	}
	//_rtspClientSession = rtspClientSession;
}

//impl of NSS
::std::string NGODStreamImpl::
getOnDemandSessionId(const ::Ice::Current& c) const
{
	RLock sync(*this);
	//IceUtil::Mutex::Lock lock(*this);
	return ident.name;
}

::std::string NGODStreamImpl::
getNssSessionId(const ::Ice::Current& c) const
{
	RLock sync(*this);
	//IceUtil::Mutex::Lock lock(*this);
	return sessIdNss;
}

::std::string NGODStreamImpl::
getSessionGroup(const ::Ice::Current& c) const
{
	RLock sync(*this);
	//IceUtil::Mutex::Lock lock(*this);
	return sessGroup;
}

::std::string NGODStreamImpl::
getCtrlURL(const ::Ice::Current& c) const
{
	//IceUtil::Mutex::Lock lock(*this);
	RLock sync(*this);
	return controlURl;
}

//impl of NSSEx
::std::string NGODStreamImpl::
getVolume(const ::Ice::Current& c) const
{
	RLock sync(*this);
	//IceUtil::Mutex::Lock lock(*this);
	return volume;
}

::TianShanIce::Streamer::NGODStreamServer::transHeaderList NGODStreamImpl::
getTransportHeader(const ::Ice::Current& c) const
{
	RLock sync(*this);
	//IceUtil::Mutex::Lock lock(*this);
	return tansportHeaderList;
}
  
::TianShanIce::Streamer::NGODStreamServer::ItemList NGODStreamImpl::
getAssetId(const ::Ice::Current& c) const
{
	RLock sync(*this);
	////IceUtil::Mutex::Lock lock(*this);
	return assetId;
}
	
//impl of stream
void NGODStreamImpl::
allotPathTicket(const ::TianShanIce::Transport::PathTicketPrx& ticket,
				const ::Ice::Current& c)
	throw (::TianShanIce::Transport::ExpiredTicket,
		   ::TianShanIce::ServerError,
		   ::TianShanIce::NotSupported)
{
}

void NGODStreamImpl::
destroy(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError)
{
	WLock sync(*this);
	_env._eNssStream->remove(ident);

	//envlog(ZQ::common::Log::L_INFO, CLOGFMT(NSS, "NGODStream session number %d"), _env._eNssStream->getSize());

	//try to send teardown
	DWORD iTime = GetTickCount();
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "destroy: session(%s) begin send TEARDOWN"), ident.name.c_str());
	bool b = ngod_rtsp_action::TeardownAction(ident.name, *_nssSessionGroupList, _pool, _logFile);
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "destroy: session(%s) send TEARDOWN over, cost %dms"), ident.name.c_str(), GetTickCount() - iTime);
	
	RTSPClientSession *pSess = _env._daemonThrd->findSessionByOnDemandSessionId(ident.name);
	if (pSess == NULL)
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"NGODStreamImpl",143,"NGODStreamImpl::session(%s) destroy() fail to find exist on demand session", ident.name.c_str());

	NSSSessionGroupList::iterator iter = find_if(_nssSessionGroupList->begin(), _nssSessionGroupList->end(),FindBySessionGroup(pSess->m_RTSPR2Header.SessionGroup.strToken));

	if (iter == _nssSessionGroupList->end())
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"NGODStreamImpl",144,"NGODStreamImpl::session(%s) destroy() fail to find session in any session group", ident.name.c_str());
	}

	EnterCriticalSection(&(*iter)->m_CS_SessionMap);
	if (pSess != NULL)
	{
		_env._daemonThrd->removeSession(pSess);
		delete pSess;
		pSess = NULL;
		LeaveCriticalSection(&(*iter)->m_CS_SessionMap);
		envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODStreamImpl,"NGODStreamImpl::destroy() successfully destroy sesion(%s) with video server"), ident.name.c_str());
	}
	else
	{
		LeaveCriticalSection(&(*iter)->m_CS_SessionMap);
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"NGODStreamImpl",145,"NGODStreamImpl::destroy() fail to find exist session(%s)", ident.name.c_str());
	}

	if (!b)
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"NGODStreamImpl",146,"NGODStreamImpl::destroy() fail to destroy session(%s)", ident.name.c_str());
}

void NGODStreamImpl::
commit_async(const ::TianShanIce::Streamer::AMD_Stream_commitPtr& amdStream, 
			 const ::Ice::Current& c)
{	
	////query content store about this asset status
	//RTSPClientSession *pSess = _env._daemonThrd->findSessionByOnDemandSessionId(ident.name);
	//if (NULL != pSess)
	//{
	//	::std::string destinationContentType;
	//	for(SDPRequestContentVec::iterator iter = pSess->m_RTSPR2Header.m_SDPRequestContent.begin();
	//		iter != pSess->m_RTSPR2Header.m_SDPRequestContent.end(); iter++)
	//	{
	//		try
	//		{
	//			::Ice::Identity tmpIdent = _env._contentStore.toContentIdent((*iter).strAsset_id.c_str());
	//			::TianShanIce::Storage::UnivContentPrx contentPrx = ::TianShanIce::Storage::UnivContentPrx::checkedCast(_env._contentStore._adapter->createProxy(tmpIdent));
	//			
	//			if (NULL == contentPrx)
	//			{
	//				::std::string strErrMsg = ::std::string("NGODStreamImpl::commit_async(), asset(%s) not find") + (*iter).strAsset_id.c_str();
	//				::ZQ::common::Exception ex(strErrMsg);
	//				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(NGODStreamImpl,"NGODStreamImpl::commit_async(), asset(%s) not find"), (*iter).strAsset_id.c_str());
	//				amdStream->ice_exception(ex);
	//			}
	//		}
	//		catch (::TianShanIce::InvalidParameter* e)
	//		{
	//			ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"NGODStreamImpl",188,"NGODStreamImpl::commit_async(),error(%s),  fail to check content(%s) status", e->message.c_str(), (*iter).strAsset_id.c_str());
	//		}
	//		catch (::TianShanIce::InvalidStateOfArt* e)
	//		{
	//			ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"NGODStreamImpl",188,"NGODStreamImpl::commit_async(),error(%s),  fail to check content(%s) status", e->message.c_str(), (*iter).strAsset_id.c_str());
	//		}
	//		catch (::TianShanIce::ServerError*e )
	//		{
	//			ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"NGODStreamImpl",188,"NGODStreamImpl::commit_async(),error(%s),  fail to check content(%s) status", e->message.c_str(), (*iter).strAsset_id.c_str());
	//		}
	//		
	//	}

	//	NSSCommitThrd *commitThrd = new NSSCommitThrd(this, amdStream);
	//	commitThrd->start();
	//}
	//else
	//{
	//	::std::string strErrMsg = ::std::string("NGODStreamImpl::commit_async() failed to find session(") + ident.name;
	//	::ZQ::common::Exception ex(strErrMsg);
	//	envlog(ZQ::common::Log::L_ERROR, CLOGFMT(NGODStreamImpl,"NGODStreamImpl::commit_async() failed to setup session(%s) with video server"), ident.name.c_str());
	//	amdStream->ice_exception(ex);
	//}
	NSSCommitThrd *commitThrd = new NSSCommitThrd(this, amdStream);
	commitThrd->start();
}

::std::string NGODStreamImpl::
lastError(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return string("");
}
  
::Ice::Identity NGODStreamImpl::
getIdent(const ::Ice::Current& c) const
{
	RLock sync(*this);
	//IceUtil::Mutex::Lock lock(*this);
	return ident;
}

void NGODStreamImpl::
setConditionalControl(const ::TianShanIce::Streamer::ConditionalControlMask& mask, 
					  const ::TianShanIce::Streamer::ConditionalControlPrx& condCtrl, 
					  const ::Ice::Current& c)
{
}
    
bool NGODStreamImpl::play(const ::Ice::Current& c)
{
	RLock sync(*this);
	//TODO: send play message
	DWORD iTime = GetTickCount();
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "play: session(%s) begin send PLAY"), ident.name.c_str());
	bool b = ngod_rtsp_action::PlayAction(ident.name, *_nssSessionGroupList, _pool, _logFile);
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "play: session(%s) send PLAY over, cost %dms"), ident.name.c_str(), GetTickCount() - iTime);
	
	if (!b)
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"NGODStreamImpl",173,"NGODStreamImpl::session(%s) play() failed to play video server", ident.name.c_str());
	return b;
}
    
bool NGODStreamImpl::
setSpeed(::Ice::Float newSpeed,
		 const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter, 
		   ::TianShanIce::ServerError, 
		   ::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	//TODO: send play message
	RTSPClientSession *_rtspClientSession = _env._daemonThrd->findSessionByOnDemandSessionId(ident.name);
	if (_rtspClientSession != NULL)
	{
		_rtspClientSession->fScale = newSpeed;
		DWORD iTime = GetTickCount();
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "setSpeed: session(%s) begin send PLAY"), ident.name.c_str());
		bool b = ngod_rtsp_action::PlayAction(ident.name, *_nssSessionGroupList, _pool, _logFile);
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "setSpeed: session(%s) send PLAY over, cost %dms"), ident.name.c_str(), GetTickCount() - iTime);
		
		if (!b)
			ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"NGODStreamImpl",174,"NGODStreamImpl::session(%s) play() failed to set play speed", ident.name.c_str());
		else
		{
			if (_rtspClientSession->fScale != newSpeed)
			{
				::ZQTianShan::NSS::listmem params;
				params.type = ::ZQTianShan::NSS::E_PLAYLIST_SPEEDCHANGED;
				params.param[EventField_PlaylistGuid] = _rtspClientSession->m_RTSPR2Header.strOnDemandSessionID;
				params.param[EventField_EventCSEQ] = (long)(_rtspClientSession->uClientCSeq);
				_env._daemonThrd->_eventList.PushBack(params);
			}
		}
		return b;
	}
	else
	{
		ZQTianShan::_IceThrow<::TianShanIce::InvalidStateOfArt> (envlog,"NGODStreamImpl",175,"NGODStreamImpl::session(%s) setSpeed() failed to find sessionId", ident.name.c_str());
		return false;
	}
}
    
bool NGODStreamImpl::
pause(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError, 
		   ::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	RTSPClientSession *_rtspClientSession = _env._daemonThrd->findSessionByOnDemandSessionId(ident.name);
	if (_rtspClientSession != NULL)
	{
		if (_rtspClientSession->iRTSPClientState != PAUSE)
		{
			DWORD iTime = GetTickCount();
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "pause: session(%s) begin send PAUSE"), ident.name.c_str());
			bool b = ngod_rtsp_action::PauseAction(ident.name, *_nssSessionGroupList, _pool, _logFile);
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "pause: session(%s) send PAUSE over, cost %dms"), ident.name.c_str(), GetTickCount() - iTime);
			
			if (!b)
				ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"NGODStreamImpl",176,"NGODStreamImpl::session(%s) pause() failed to pause", ident.name.c_str());
			return b;
		}
		else
		{
			ZQTianShan::_IceThrow<::TianShanIce::InvalidStateOfArt> (envlog,"NGODStreamImpl",177,"NGODStreamImpl::session(%s) pause() client already in PAUSE state", ident.name.c_str());
			return false;
		}
	}
	else
	{
		ZQTianShan::_IceThrow<::TianShanIce::InvalidStateOfArt> (envlog,"NGODStreamImpl",178,"NGODStreamImpl::session(%s) pause() failed to find sessionId", ident.name.c_str());
		return false;
	}
}
    
bool NGODStreamImpl::
resume(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError, 
		   ::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	DWORD iTime = GetTickCount();
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "resume: session(%s) begin send PLAY"), ident.name.c_str());
	bool b = ngod_rtsp_action::PlayAction(ident.name, *_nssSessionGroupList, _pool, _logFile);
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "resume: session(%s) send PLAY over, cost %dms"), ident.name.c_str(), GetTickCount() - iTime);
	
	if (!b)
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"NGODStreamImpl",179,"NGODStreamImpl::session(%s) resume() failed to resume server", ident.name.c_str());
	return b;
}
    
::TianShanIce::Streamer::StreamState NGODStreamImpl::
getCurrentState(const ::Ice::Current& c) const
	throw (::TianShanIce::ServerError)
{
	RLock sync(*this);
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "session(%s) enter getCurrentState"), ident.name.c_str());
	RTSPClientSession *_rtspClientSession = _env._daemonThrd->findSessionByOnDemandSessionId(ident.name);
	if (_rtspClientSession != NULL)
	{
		switch (_rtspClientSession->iRTSPClientState)
		{
			case SETUP:
				return ::TianShanIce::Streamer::stsSetup;
			case PLAY:
				return ::TianShanIce::Streamer::stsStreaming;
			case PAUSE:
				return ::TianShanIce::Streamer::stsPause;
			case TEARDOWN:
				return ::TianShanIce::Streamer::stsStop;
			default:
				return ::TianShanIce::Streamer::stsStop;
		}	
	}
	else
		return ::TianShanIce::Streamer::stsStop;
}
    
::TianShanIce::SRM::SessionPrx NGODStreamImpl::
getSession(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError)
{
	return NULL;
	//TODO
}

void NGODStreamImpl::
setMuxRate(::Ice::Int nowRate, 
		   ::Ice::Int maxRate,
		   ::Ice::Int minRate,
		   const ::Ice::Current& c)
{
}

bool NGODStreamImpl::
allocDVBCResource(::Ice::Int serviceGroupID,
				  ::Ice::Int bandWidth,
				  const ::Ice::Current& c)
	throw (::TianShanIce::ServerError)
{
	return true;
}

::Ice::Long NGODStreamImpl::
seekStream(::Ice::Long offset,
		   ::Ice::Int startPos,
		   const ::Ice::Current& c)
	throw (::TianShanIce::ServerError,
		   ::TianShanIce::InvalidParameter)
{
	if (startPos == 1 && offset < 0)
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"NGODStreamImpl",180,"NGODStreamImpl::session(%s) seekStream() seek from begin but offset(%d) is negative", ident.name.c_str(), offset);
	if (startPos == 2 && offset > 0)
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"NGODStreamImpl",180,"NGODStreamImpl::session(%s) seekStream() seek from end but offset(%d) is positive", ident.name.c_str(), offset);
	return 1;
}

/*void NGODStreamImpl::
playEx(::Ice::Long& timeOffset,
::Ice::Float& currentSpeed,
const ::Ice::Current& c) 
throw (::TianShanIce::ServerError, 
::TianShanIce::InvalidStateOfArt)*/
::TianShanIce::Streamer::StreamInfo NGODStreamImpl::
playEx(::Ice::Float newSpeed, 
	   ::Ice::Long timeOffset,
	   ::Ice::Short from,
	   const ::TianShanIce::StrValues& expectedProps,
	   const ::Ice::Current& c)
	   throw (::TianShanIce::ServerError, 
	   ::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	DWORD iTime;
	::TianShanIce::Streamer::StreamInfo streamInfo;
	RTSPClientSession *_rtspClientSession = _env._daemonThrd->findSessionByOnDemandSessionId(ident.name);
	if (_rtspClientSession != NULL)
	{
		_rtspClientSession->strCurrentTimePoint.clear();
		bool b = false;
		vecGETPARAMETER_EXT headerList;
		//for convenient use
		GetPramameterRes_ExtHeader &sessState = _rtspClientSession->m_RTSPR2Header.m_GetPramameterRes_ExtHeader;
		long fPos = 0;
		if (from < 0)
		{
			headerList.clear();
			headerList.push_back(position);

			iTime = GetTickCount();
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "playEx: session(%s) begin send GETPARAMETER"), ident.name.c_str());
			b = ngod_rtsp_action::GetParameterActionBySessionId(ident.name, *_nssSessionGroupList, _pool, _logFile, headerList);
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "playEx: session(%s) send GETPARAMETER over, cost %dms"), ident.name.c_str(), GetTickCount() - iTime);
			
			if (!b)
				ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"NGODStreamImpl",180,"NGODStreamImpl::session(%s) playEx() fail to get the session info", ident.name.c_str());
			else
			{
				const char *pHead = strstr(_rtspClientSession->m_RTSPR2Header.m_GetPramameterRes_ExtHeader.strPosition.c_str(), "-");
				pHead += 1;
				fPos = atoi(pHead);
				fPos -= timeOffset / 1000;
			}
		}
		else if (from > 0)
			fPos = timeOffset / 1000;

		stringstream ss;
		//set time off set
		ss << fPos;

		if (from != 0)
			_rtspClientSession->strCurrentTimePoint = ss.str();
		else
			_rtspClientSession->strCurrentTimePoint = "now";
		_rtspClientSession->fScale = newSpeed;

		iTime = GetTickCount();
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "playEx: session(%s) begin send PLAY"), ident.name.c_str());
		b = ngod_rtsp_action::PlayAction(ident.name, *_nssSessionGroupList, _pool, _logFile);
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "playEx: session(%s) send PLAY over, cost %dms"), ident.name.c_str(), GetTickCount() - iTime);

		if (!b)
		{
			ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"NGODStreamImpl",180,"NGODStreamImpl::session(%s) playEx() fail to play the session", ident.name.c_str());
		
			//try to get session info
			headerList.clear();
			headerList.push_back(presentation_state);
			b = ngod_rtsp_action::GetParameterActionBySessionId(ident.name, *_nssSessionGroupList, _pool, _logFile, headerList);
			if (!b)
				ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"NGODStreamImpl",180,"NGODStreamImpl::session(%s) playEx() fail to get the session state", ident.name.c_str());
			else
			{			
				if (sessState.strPresentation_state.compare(READYSTATE) == 0)
					streamInfo.state = TianShanIce::Streamer::stsSetup;
				else if (sessState.strPresentation_state.compare(PLAYSTATE) == 0)
					streamInfo.state	= TianShanIce::Streamer::stsStreaming;
				else if (sessState.strPresentation_state.compare(PAUSESTATE) == 0)
					streamInfo.state	= TianShanIce::Streamer::stsPause;
				else
					streamInfo.state	= TianShanIce::Streamer::stsStop;			
			}
		}
		else
		{
			streamInfo.state = TianShanIce::Streamer::stsStreaming;
			float fRange = 0.0;
			if (!_rtspClientSession->strCurrentTimePoint.empty())
			{
				fRange = atof(_rtspClientSession->strCurrentTimePoint.c_str());
				stringstream ss;
				ss << fRange * 1000;
				streamInfo.props["CURRENTPOS"]	= ss.str();
			}
			stringstream sScale;
			sScale << _rtspClientSession->fScale;
			streamInfo.props["SPEED"]	=sScale.str();
		}
	}
	else
		ZQTianShan::_IceThrow<::TianShanIce::InvalidStateOfArt> (envlog,"NGODStreamImpl",181,"NGODStreamImpl::session(%s) playEx() fail to find exist session", ident.name.c_str());

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODStreamImpl,"NGODStreamImpl(OnDemandSession: %s) playEx successfully"), ident.name.c_str());
	return streamInfo;
}

/*void NGODStreamImpl::
pauseEx(::Ice::Long& timeOffset,
const ::Ice::Current& c)
throw (::TianShanIce::ServerError, 
::TianShanIce::InvalidStateOfArt)*/
::TianShanIce::Streamer::StreamInfo NGODStreamImpl::
pauseEx(const ::TianShanIce::StrValues& expectedProps, 
		const ::Ice::Current& c)
		throw (::TianShanIce::ServerError, 
		::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	::TianShanIce::Streamer::StreamInfo streamInfo;
	RTSPClientSession *_rtspClientSession = _env._daemonThrd->findSessionByOnDemandSessionId(ident.name);
	if (_rtspClientSession != NULL)
	{
		_rtspClientSession->strCurrentTimePoint.clear();
		vecGETPARAMETER_EXT headerList;
		//easy to use
		GetPramameterRes_ExtHeader &sessState = _rtspClientSession->m_RTSPR2Header.m_GetPramameterRes_ExtHeader;

		DWORD iTime = GetTickCount();
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "pauseEx: session(%s) begin send PAUSE"), ident.name.c_str());
		bool b = ngod_rtsp_action::PauseAction(ident.name, *_nssSessionGroupList, _pool, _logFile);

		envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "pauseEx: session(%s) send PAUSE over, cost %dms"), ident.name.c_str(), GetTickCount() - iTime);
		
		if (!b)
		{
			ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"NGODStreamImpl",180,"NGODStreamImpl::session(%s) pauseEx() fail to play the session", ident.name.c_str());

			//try to get session info
			headerList.clear();
			headerList.push_back(presentation_state);

			iTime = GetTickCount();
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "pauseEx: session(%s) begin send GETPARAMETER"), ident.name.c_str());
			b = ngod_rtsp_action::GetParameterActionBySessionId(ident.name, *_nssSessionGroupList, _pool, _logFile, headerList);
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "pauseEx: session(%s) send GETPARAMETER over, cost %dms"), ident.name.c_str(), GetTickCount() - iTime);
			
			if (!b)
				ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"NGODStreamImpl",180,"NGODStreamImpl::session(%s) pauseEx() fail to get the session state", ident.name.c_str());
			else
			{			
				if (sessState.strPresentation_state.compare(READYSTATE) == 0)
					streamInfo.state = TianShanIce::Streamer::stsSetup;
				else if (sessState.strPresentation_state.compare(PLAYSTATE) == 0)
					streamInfo.state = TianShanIce::Streamer::stsStreaming;
				else if (sessState.strPresentation_state.compare(PAUSESTATE) == 0)
					streamInfo.state = TianShanIce::Streamer::stsPause;
				else
					streamInfo.state = TianShanIce::Streamer::stsStop;			
			}
		}
		else
		{
			streamInfo.state = TianShanIce::Streamer::stsPause;
			float fRange = 0.0;
			if (!_rtspClientSession->strCurrentTimePoint.empty())
			{
				fRange = atof(_rtspClientSession->strCurrentTimePoint.c_str());
				stringstream ss;
				ss << fRange * 1000;
				streamInfo.props["CURRENTPOS"]	= ss.str();
			}
			stringstream sScale;
			sScale << _rtspClientSession->fScale;
			streamInfo.props["SPEED"]	=sScale.str();
		}
	}
	else
		ZQTianShan::_IceThrow<::TianShanIce::InvalidStateOfArt> (envlog,"NGODStreamImpl",181,"NGODStreamImpl::session(%s) pausEx() fail to find exist session", ident.name.c_str());
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(NGODStreamImpl,"NGODStreamImpl(OnDemandSession: %s) pauseEx successfully"), ident.name.c_str());
	return streamInfo;
}
    
void NGODStreamImpl::
setSpeedEx(::Ice::Float newSpeed,
		   ::Ice::Long& timeOffset,
		   ::Ice::Float& currentSpeed,
		   const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter,
		   ::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	RTSPClientSession *_rtspClientSession = _env._daemonThrd->findSessionByOnDemandSessionId(ident.name);
	stringstream ss;

	//set speed
	_rtspClientSession->fScale = newSpeed;

	//set time off set
	ss << timeOffset;
	_rtspClientSession->strCurrentTimePoint = ss.str();
	ss.clear();

	DWORD iTime = GetTickCount();
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "setSpeedEx: session(%s) begin send PLAY"), ident.name.c_str());
	bool b = ngod_rtsp_action::PlayAction(ident.name, *_nssSessionGroupList, _pool, _logFile);
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "setSpeedEx: session(%s) send PLAY over, cost %dms"), ident.name.c_str(), GetTickCount() - iTime);

	if (!b)
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"NGODStreamImpl",180,"NGODStreamImpl::session(%s) setSpeedEx() fail to play the session with new scale", ident.name.c_str());
	else
	{
		if (_rtspClientSession->fScale != newSpeed)
		{
			::ZQTianShan::NSS::listmem params;
			params.type = ::ZQTianShan::NSS::E_PLAYLIST_SPEEDCHANGED;
			params.param[EventField_PlaylistGuid] = _rtspClientSession->m_RTSPR2Header.strOnDemandSessionID;
			params.param[EventField_EventCSEQ] = (long)_rtspClientSession->uClientCSeq;
			_env._daemonThrd->_eventList.PushBack(params);
		}
		currentSpeed = _rtspClientSession->fScale;
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "session(%s) send setSpeedEx OK"), ident.name.c_str());
	}
}
    
//impl of playlist
::std::string NGODStreamImpl::
getId(const ::Ice::Current& c) const
{
	RLock sync(*this);
	//IceUtil::Mutex::Lock lock(*this);
	return ident.name;
}
   
bool NGODStreamImpl::
getInfo(::Ice::Int mask,
		::TianShanIce::ValueMap& varOut,
		const ::Ice::Current& c)
{
	RLock sync(*this);

	envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(NGODStreamImpl,"guid[%s]enter get info"), ident.name.c_str());

	switch(mask)
	{
		case TianShanIce::Streamer::infoSTREAMNPTPOS:
			break;
		default:
			return false;
	}

	vecGETPARAMETER_EXT headerList;
	headerList.push_back(presentation_state);
	headerList.push_back(position);
	headerList.push_back(scale);

	bool b = false;

	DWORD iTime = GetTickCount();
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "getInfo: session(%s) begin send GETPARAMETER"), ident.name.c_str());
	b = ngod_rtsp_action::GetParameterActionBySessionId(ident.name, *_nssSessionGroupList, _pool, _logFile, headerList);
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "getInfo: session(%s) send GETPARAMETER over, cost %dms"), ident.name.c_str(), GetTickCount() - iTime);
	
	if (!b)
		return false;
	else
	{
		ZQ::common::Variant var;
		if(!var.valid())
			return false;

		RTSPClientSession *_rtspClientSession = _env._daemonThrd->findSessionByOnDemandSessionId(ident.name);
		if (NULL == _rtspClientSession)
		{
			envlog(::ZQ::common::Log::L_WARNING,CLOGFMT(NGODStreamImpl,"guid[%s]enter get info fail, could not find session"), ident.name.c_str());

			return false;
		}

		TianShanIce::Variant res;
		res.ints.clear();
		res.bRange = false;
		res.ints.push_back(atoi(_rtspClientSession->m_RTSPR2Header.m_GetPramameterRes_ExtHeader.strPosition.c_str()));
		res.type=TianShanIce::vtInts;
		varOut["playposition"]=res;		

		/*res.ints.clear();
		res.bRange = false;
		res.strs.push_back(_rtspClientSession->m_RTSPR2Header.m_GetPramameterRes_ExtHeader.strPosition);
		res.type=TianShanIce::vtInts;
		varOut["totalplaytime"]=res;*/

		res.bRange = false;
		res.strs.clear();
		res.strs.push_back(_rtspClientSession->m_RTSPR2Header.m_GetPramameterRes_ExtHeader.strScale);
		res.type=TianShanIce::vtStrings;
		varOut["scale"]=res;
	}

	envlog(::ZQ::common::Log::L_WARNING,CLOGFMT(NGODStreamImpl,"guid[%s]enter get info success"), ident.name.c_str());
	return true;
}
   
::Ice::Int NGODStreamImpl::
insert(::Ice::Int userCtrlNum, 
	   const ::TianShanIce::Streamer::PlaylistItemSetupInfo& newItemInfo,
	   ::Ice::Int whereUserCtrlNum,
	   const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter, 
		   ::TianShanIce::ServerError, 
		   ::TianShanIce::InvalidStateOfArt)
{
	RTSPClientSession *_rtspClientSession = _env._daemonThrd->findSessionByOnDemandSessionId(ident.name);
	if (_rtspClientSession == NULL)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"NGODStreamImpl",188,"NGODStreamImpl::session(%s) insert() fail to find session", ident.name.c_str());
		return -1;
	}

	::TianShanIce::ValueMap pMap = newItemInfo.privateData;
	::TianShanIce::ValueMap::iterator mapIter;
	SDPRequestContent pSDP;
	
	pSDP.iCtrlNum = userCtrlNum;
	//pSDP.strProvider_id = "schange.com";
	pSDP.strRange = "0-";
	//pSDP.strAsset_id = newItemInfo.contentName;
	if ((mapIter = pMap.find("PAID")) != pMap.end())
	{
		//get client_id
		if ((*mapIter).second.type != ::TianShanIce::vtStrings)
			pSDP.strAsset_id = "";
		else			
			pSDP.strAsset_id = (*mapIter).second.strs[0];
		envlog(::ZQ::common::Log::L_WARNING,CLOGFMT(NGODStreamImpl,"insert: session(%s) get PAID %s"), ident.name.c_str(), pSDP.strAsset_id.c_str());
	}
	if ((mapIter = pMap.find("PID")) != pMap.end())
	{
		//get client_id
		if ((*mapIter).second.type != ::TianShanIce::vtStrings)
			pSDP.strProvider_id = "";
		else
			pSDP.strProvider_id = (*mapIter).second.strs[0];
		envlog(::ZQ::common::Log::L_WARNING,CLOGFMT(NGODStreamImpl,"insert: session(%s) get PID %s"), ident.name.c_str(), pSDP.strProvider_id.c_str());
	}

	SDPRequestContentVec *pSDPList = &(_rtspClientSession->m_RTSPR2Header.m_SDPRequestContent);
	SDPRequestContentVec::iterator listIter = find_if(pSDPList->begin(), pSDPList->end(), FindByCtlrNum(whereUserCtrlNum));

	if (listIter != pSDPList->end())
		//insert before
		pSDPList->insert(listIter, pSDP);
	else
		pSDPList->push_back(pSDP);

	WLock sync(*this);
	//IceUtil::Mutex::Lock lock(*this);
	::TianShanIce::Streamer::NGODStreamServer::stItem tmpItem;
	tmpItem.assetId = pSDP.strAsset_id;
	tmpItem.providerId = pSDP.strProvider_id;
	tmpItem.itemNum = whereUserCtrlNum;
	if (assetId.empty())
		assetId.push_back(tmpItem);
	else
	{
		for (::TianShanIce::Streamer::NGODStreamServer::ItemList::iterator iter = assetId.begin();
			iter != assetId.end(); iter++)
		{
			if ( whereUserCtrlNum == (*iter).itemNum)
			{
				assetId.insert(iter, tmpItem);
				return 1;
			}
		}
		assetId.push_back(tmpItem);
	}

	return 1;
}

   
::Ice::Int NGODStreamImpl::
pushBack(::Ice::Int userCtrlNum,
		 const ::TianShanIce::Streamer::PlaylistItemSetupInfo& newItemInfo,
		 const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter,
		   ::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
	WLock sync(*this);
	RTSPClientSession *_rtspClientSession = _env._daemonThrd->findSessionByOnDemandSessionId(ident.name);
	if (_rtspClientSession == NULL)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"NGODStreamImpl",189,"NGODStreamImpl::session(%s) pushBack() fail to find session %s", ident.name.c_str());
		return -1;
	}

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(NSS, "NGODStreamImpl::pushBack() successfully find session %s") , ident.name.c_str());

	const ::TianShanIce::ValueMap &pMap = newItemInfo.privateData;

	::TianShanIce::ValueMap::const_iterator mapIter;
	SDPRequestContent pSDP;
	
	pSDP.iCtrlNum = userCtrlNum;
	//pSDP.strProvider_id = "schange.com";
	pSDP.strRange = "0-";
	//pSDP.strAsset_id = newItemInfo.contentName;
	if ((mapIter = pMap.find("PAID")) != pMap.end())
	{
		//get asset_id
		if ((*mapIter).second.type != ::TianShanIce::vtStrings)
		{
			pSDP.strAsset_id = "";
			envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(NGODStreamImpl,"pushBack: session(%s) PAID type isn't string"), ident.name.c_str());
		}
		else
		{
			pSDP.strAsset_id = (*mapIter).second.strs[0];
			envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(NGODStreamImpl,"pushBack: session(%s) get PAID %s "), ident.name.c_str(), pSDP.strAsset_id.c_str());
		}
	}
	if ((mapIter = pMap.find("PID")) != pMap.end())
	{
		//get provider_id
		if ((*mapIter).second.type != ::TianShanIce::vtStrings)
		{
			pSDP.strProvider_id = "";
			envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(NGODStreamImpl,"pushBack: session(%s) PID type isn't string"), ident.name.c_str());
		}
		else
			pSDP.strProvider_id = (*mapIter).second.strs[0];
		envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(NGODStreamImpl,"pushBack: session(%s) get PID %s"), ident.name.c_str(), pSDP.strProvider_id.c_str());
	}

	_rtspClientSession->m_RTSPR2Header.m_SDPRequestContent.push_back(pSDP);

	//IceUtil::Mutex::Lock lock(*this);
	::TianShanIce::Streamer::NGODStreamServer::stItem tmpItem;
	tmpItem.assetId = pSDP.strAsset_id;
	tmpItem.providerId = pSDP.strProvider_id;
	tmpItem.itemNum = userCtrlNum;
	assetId.push_back(tmpItem);

	return 1;
}
   
::Ice::Int NGODStreamImpl::
size(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return assetId.size();
}
   
::Ice::Int NGODStreamImpl::
left(const ::Ice::Current& c) const
{
	return 1;
}
   
bool NGODStreamImpl::
empty(const ::Ice::Current& c) const
	throw (::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	return assetId.empty();
}

::Ice::Int NGODStreamImpl::
current(const ::Ice::Current& c) const
	throw (::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
	return 1;
}
   
void NGODStreamImpl::
erase(::Ice::Int whereUserCtrlNum,
	  const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter,
		   ::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
	RTSPClientSession *_rtspClientSession = _env._daemonThrd->findSessionByOnDemandSessionId(ident.name);
	SDPRequestContentVec *pSDPList = &(_rtspClientSession->m_RTSPR2Header.m_SDPRequestContent);
	SDPRequestContentVec::iterator listIter = find_if(pSDPList->begin(), pSDPList->end(), FindByCtlrNum(whereUserCtrlNum));
	if (listIter == pSDPList->end())
	{
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"NGODStreamImpl",180,"NGODStreamImpl::session(%s) erase() fail to find item from playlist", ident.name.c_str());
	}
	else
	{
		pSDPList->erase(listIter);
	}
	for (::TianShanIce::Streamer::NGODStreamServer::ItemList::iterator iter = assetId.begin();
			iter != assetId.end(); iter++)
	{
		if ( whereUserCtrlNum = (*iter).itemNum)
		{
			assetId.erase(iter);
			return;
		}
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"NGODStreamImpl",180,"NGODStreamImpl::session(%s) erase() fail to find item from playlist", ident.name.c_str());
	}
}
   
::Ice::Int NGODStreamImpl::
flushExpired(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
	return 1;
}
   
bool NGODStreamImpl::
clearPending(bool includeInitedNext,
			 const ::Ice::Current& c)
	throw (::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
	return true;
}
   
bool NGODStreamImpl::
isCompleted(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
	return true;
}
   
::TianShanIce::IValues NGODStreamImpl::
getSequence(const ::Ice::Current& c) const
{
	::TianShanIce::IValues tmpValues;
	::TianShanIce::Streamer::NGODStreamServer::ItemList::const_iterator iter = assetId.begin();
	for (; iter != assetId.end(); iter++)
		tmpValues.push_back((*iter).itemNum);
	return tmpValues;
}
   
::Ice::Int NGODStreamImpl::
findItem(::Ice::Int userCtrlNum,
		 ::Ice::Int from,
		 const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter,
		   ::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
	::TianShanIce::Streamer::NGODStreamServer::ItemList::const_iterator iter = assetId.begin();
	for (; iter != assetId.end(); iter++)
	{
		if ((*iter).itemNum == userCtrlNum)
			return (*iter).itemNum;
	}
	return 0;
}
   
bool NGODStreamImpl::
distance(::Ice::Int to,
		 ::Ice::Int from,
		 ::Ice::Int& dist,
		 const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter,
		   ::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
	return true;
}
   
bool NGODStreamImpl::
skipToItem(::Ice::Int where, 
		   bool bPlay,
		   const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter, 
		   ::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
	return true;
}
   
bool NGODStreamImpl::
seekToPosition(::Ice::Int UserCtrlNum,
			   ::Ice::Int timeOffset,
			   ::Ice::Int startPos,
			   const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter,
		   ::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
	return true;
}
   
void NGODStreamImpl::
enableEoT(bool enable,
		  const ::Ice::Current& c)
	throw (::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
}

::TianShanIce::Streamer::StreamInfo NGODStreamImpl::
playItem(::Ice::Int UserCtrlNum,
		 ::Ice::Int timeOffset,
		 ::Ice::Short from,
		 ::Ice::Float newSpeed,
		 const ::TianShanIce::StrValues& expectedProps,
		 const ::Ice::Current& c)
  throw (::TianShanIce::ServerError,
		 ::TianShanIce::InvalidStateOfArt, 
		 ::TianShanIce::InvalidParameter)
{
	return playEx(newSpeed, timeOffset, from, expectedProps, c);
}

/***********************
//NGODStreamServiceImpl
************************/
NGODStreamServiceImpl::
NGODStreamServiceImpl(ZQ::common::FileLog &LogFile,
					  ::ZQ::common::NativeThreadPool &pool,
					  ::Ice::CommunicatorPtr& communicator,
					  string &strServerPath,
					  uint16 &uServerPort,
					  SessionGroup &sessionGroups,
					  int32 &evictorSize,
					  NSSEnv &env):
_env(env),
_logFile(LogFile),
_pool(pool),
_strServerPath(strServerPath),
_uServerPort(uServerPort)
{
	_guid.create();
	_env.SetSessionList(_nssSessionGroupList);
	
	SessionGroup::Groups &groups = sessionGroups._groups;
	//initialize daemon thread
	_nssDaemodThrdPtr = new ngod_daemon_thread(&_logFile, _env._nssEventSinkI._paramsList ,_nssSessionGroupList, _pool, _env._communicator, _env._adapter);
	_env.SetDaemonThrd(_nssDaemodThrdPtr);
	_nssDaemodThrdPtr->initialize();

	//active adapter
	try
	{
		_env._adapter->ZQADAPTER_ADD(_env._communicator, this, ADAPTER_NAME_NSS);
		//_env._adapter->activate();
	} catch (const Ice::Exception& e) {
		cerr << e << endl;
	} catch (const char* msg) {
		cerr << msg << endl;
	}

	// Initialize the NSSEventSinkI class
	//::ZQTianShan::NSS::NSSEventSinkI eventSink(_logFile, iceStormEndpoint, _env._adapter);

	//initialize environment for NSS evictor
	_env._adapter->findServantLocator(DBFILENAME_NssSession);
	if (evictorSize <= 0)
		_env._eNssStream->setSize(1000);
	else
		_env._eNssStream->setSize(evictorSize);
	Freeze::EvictorIteratorPtr evicIter = _env._eNssStream->getIterator("", _env._eNssStream->getSize());
	const ::Ice::Current c;
	while (evicIter && evicIter->hasNext())
	{
		::Ice::Identity _ident = evicIter->next();
		try
		{
			//string PrxString = _env._communicator->identityToString(_ident);
			//::TianShanIce::Streamer::NGODStreamServer::NGODStreamExPrx streamPrx= ::TianShanIce::Streamer::NGODStreamServer::NGODStreamExPrx::checkedCast(_env._adapter->createProxy(_ident));
			::TianShanIce::Streamer::NGODStreamServer::NGODStreamExPrx streamPrx = NSSIdentityToObjEnv(_env, NGODStreamEx, _ident);

			//use proxy to restore data
			string strSessionGroup = streamPrx->getSessionGroup();
			string strRtspSessionId = streamPrx->getNssSessionId();
			string strOnDemandSessId = streamPrx->getOnDemandSessionId();

			NSSSessionGroupList::iterator iter = find_if(_nssSessionGroupList.begin(), _nssSessionGroupList.end(), FindBySessionGroup(strSessionGroup));
			NSSSessionGroup *pGroup = NULL;
			if (iter == _nssSessionGroupList.end())
			{
				//add new session into group
				envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(NSS, "restore group from db"));
				//try to restore the session group
				pGroup = InitSessionGroup(strSessionGroup, _strServerPath, _uServerPort);
				if (pGroup != NULL)
				{
					pGroup->uMaxSession = NSSSessionMapMaxSize;
					for (SessionGroup::Groups::iterator iter = groups.begin(); iter != groups.end(); iter++)
					{
						if ((*iter).name.compare(strSessionGroup) == 0)
							pGroup->uMaxSession = (*iter).maxSession;
					}
					_env._daemonThrd->addGroup(pGroup);
				}
			}
			else
				pGroup = *iter;

			//restore session from evictor and add to group
			RTSPClientSession *pSess = new RTSPClientSession();
			pSess->m_RTSPR2Header.Volume.strName = streamPrx->getVolume();
			RTSPTransportUdpHeader header;
			::TianShanIce::Streamer::NGODStreamServer::transHeaderList headerList = streamPrx->getTransportHeader();
			for (::TianShanIce::Streamer::NGODStreamServer::transHeaderList::iterator transIter = headerList.begin(); transIter != headerList.end(); transIter++)
			{
				header.strClient_id = (*transIter).clientId;
				header.strDestination = (*transIter).destination;
				header.strClient_port = (*transIter).clientPort;
				header.strSop_name = (*transIter).sopName;
				header.strBandwidth = (*transIter).bandWidth;
				pSess->m_RTSPR2Header.m_RTSPTransportHeader.TransportUdpHeader.push_back(header);
			}

			::TianShanIce::Streamer::NGODStreamServer::ItemList  assetList = streamPrx->getAssetId();
			SDPRequestContent pSDP;
			for (::TianShanIce::Streamer::NGODStreamServer::ItemList::iterator pIter = assetList.begin();
				pIter != assetList.end(); pIter++)
			{
				pSDP.strProvider_id = "schange.com";
				pSDP.strRange = "0-";
				pSDP.strAsset_id = (*pIter).assetId;
				pSDP.iCtrlNum = (*pIter).itemNum;
				pSess->m_RTSPR2Header.m_SDPRequestContent.push_back(pSDP);
			}

			pSess->m_RTSPR2Header.strOnDemandSessionID = strOnDemandSessId;
			pSess->m_RTSPR2Header.strSessionID = strRtspSessionId;
			pSess->m_RTSPR2Header.SessionGroup.strToken = strSessionGroup;
			_env._daemonThrd->addSession(pSess, strSessionGroup);
		}
		catch  (::Ice::UnmarshalOutOfBoundsException &ex)
		{
			envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(NSS, "catch UnmarshalOutOfBoundsException when restore group from db error(%s)"), _ident.name.c_str());
			_env._eNssStream->remove(_ident);
		}
		catch(...)
		{
			envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(NSS, "restore group from db error(%s)"), _ident.name.c_str());
			_env._eNssStream->remove(_ident);
		}
	}

	//retrieve session groups from configuration
	for (SessionGroup::Groups::iterator iter = groups.begin(); iter != groups.end(); iter++)
	{
		NSSSessionGroupList::iterator pIter = find_if(_nssSessionGroupList.begin(), _nssSessionGroupList.end(), FindBySessionGroup((*iter).name));
		if (pIter == _nssSessionGroupList.end())
		{
			//add new session into group
			envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(NSS, "create group from config"));
			//try to restore the session group
			NSSSessionGroup * pGroup = InitSessionGroup((*iter).name, _strServerPath, _uServerPort);
			if (pGroup != NULL)
			{
				pGroup->uMaxSession = (*iter).maxSession;
				_env._daemonThrd->addGroup(pGroup);
			}
			else
			{
				envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(NSS, "cannot create group %s"), (*iter).name.c_str());
				//exit(-1);
			}
		}
		else
			(*pIter)->uMaxSession = (*iter).maxSession;
	}

	//try to sync after create group
	for (NSSSessionGroupList::iterator iter = _nssSessionGroupList.begin(); iter != _nssSessionGroupList.end(); iter++)
	{
		ngod_rtsp_action::GetParameterAction((*iter)->strSessionGroup, _nssSessionGroupList, _pool, _logFile);
	}
	_nssDaemodThrdPtr->start();
}

NGODStreamServiceImpl::
~NGODStreamServiceImpl()
{
	delete _nssDaemodThrdPtr;
}

::TianShanIce::Streamer::StreamPrx NGODStreamServiceImpl::
createStream(const ::TianShanIce::Transport::PathTicketPrx& pathTicket,
			 const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter, 
		   ::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
	//get information from pathTicket
	//create a new rtspClientSession
	//add rtspClientSession to SessionGroup

	try
	{
		/// read from ticket: stream->onDemandSessId = 
		//printf("pathTicket string %s\n",_env._communicator->proxyToString(pathTicket).c_str());
		::Ice::Identity ident = pathTicket->getIdent();
			
		RTSPClientSession *rtspClientSession = InitRTSPSession(pathTicket);
		rtspClientSession->m_RTSPR2Header.strOnDemandSessionID = ident.name;

		//add session to group
		if (_env._daemonThrd->addSession(rtspClientSession) == false)//create session fail
		{
			delete rtspClientSession;
			::ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, EXPFMT(NSS, 306, "createSession(%s) fail to join session group"), ident.name.c_str());
		}

		NGODStreamImplPtr stream = new NGODStreamImpl(_env);
		if (!stream)
			return NULL;

		stream->ident.name = ident.name;
		//stream->ident.category = strCategory;
		stream->ident.category = DBFILENAME_NssSession;
		stream->pathTicketStr = _env._communicator->proxyToString(pathTicket);

		stream->sessGroup = rtspClientSession->m_RTSPR2Header.SessionGroup.strToken;
		stream->volume = rtspClientSession->m_RTSPR2Header.Volume.strName;
		stream->setRtspClientSession(rtspClientSession);

		::TianShanIce::Streamer::StreamPrx streamProxy = NSSIdentityToObjEnv(_env, NGODStreamEx, stream->ident);
		rtspClientSession->strStreamName = _env._communicator->proxyToString(streamProxy);
		
		envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(NSS, "createSession() add the new NGODStream session[%s] into DB"), stream->ident.name.c_str());

		stream->sessKey = stream->ident.name;
		_env._eNssStream->add(stream, stream->ident);
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(NSS, "created a new NGODStream session[%s]"), stream->ident.name.c_str());

		//envlog(ZQ::common::Log::L_INFO, CLOGFMT(NSS, "NGODStream session number %d"), _env._eNssStream->getSize());
		
		return streamProxy;
	}
	catch (Ice::Exception& ex)
	{
		//printf("name:%s\n", ex.ice_name().c_str());
		//ex.ice_print(cout);
		::ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, EXPFMT(NSS, 306, "createSession() ice exception[%s]"), ex.ice_name().c_str());
	}
	catch (...)
	{
		::ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, EXPFMT(NSS, 307, "createSession() unknown exception"));
		//cerr << "unknow exception" << endl;
	}

	//catch exception, return null
	return NULL;
}

::TianShanIce::Streamer::StreamerDescriptors
NGODStreamServiceImpl::listStreamers(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError)
{
	::TianShanIce::Streamer::StreamerDescriptors _Descriptors;

	//envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(NSS, "listStreamers call"));
	//Freeze::EvictorIteratorPtr iterPtr = _env._eNssStream->getIterator("", 100);
	//while (iterPtr && iterPtr->hasNext())
	//{
		::TianShanIce::Streamer::StreamerDescriptor _Descriptor;
		//::Ice::Identity _ident = iterPtr->next();
		//_Descriptor.deviceId = _ident.name;
		//_Descriptor.type = _ident.category;
		char tmpGUid[128];
		//_guid.toString(tmpGUid, 127);
		//tmpGUid[127] = 0;

		memset(tmpGUid, 0 , 128);
		gethostname(tmpGUid, 127);
		//tmpGUid[127] = 0;
		_Descriptor.deviceId = tmpGUid;
		//_Descriptor.deviceId = "Stream1";
		//_Descriptor.type = "NSSService";
		_Descriptor.type = DBFILENAME_NssSession;
		_Descriptors.push_back(_Descriptor);
	//}
	return _Descriptors;
}

::std::string NGODStreamServiceImpl::
getNetId(const ::Ice::Current& c) const
	throw (::TianShanIce::ServerError)
{
	//char tmpGuid[128];
	//_guid.toString(tmpGuid, 127);
	//tmpGuid[127] = 0;
	//return string(tmpGuid);
	return string("NSS");
}

void NGODStreamServiceImpl::
getStreamStat_async(const::TianShanIce::Streamer::NGODStreamServer::AMD_NGODStreamService_getStreamStatPtr& amdCB, 
					const ::TianShanIce::StrValues&, 
					const ::std::string&, 
					const ::Ice::Current& c)
{
	//TODO:
	::TianShanIce::Streamer::NGODStreamServer::StreamStatCollection streamStateCollection;
	amdCB->ice_response(streamStateCollection);
}

::TianShanIce::Streamer::NGODStreamServer::NGODStreamPrx NGODStreamServiceImpl::
findStreamByOnDemandSession(const ::std::string& onDemandSessionId, 
							const ::Ice::Current& c)
{
	for (NSSSessionGroupList::iterator iter = _nssSessionGroupList.begin();
	iter != _nssSessionGroupList.end(); iter++)
	{
		RTSPClientSession *pSess = _nssDaemodThrdPtr->findSessionByOnDemandSessionId(onDemandSessionId);
		if (pSess != NULL)//got the session
		{
			string strTmpSessKey;
			strTmpSessKey = pSess->m_RTSPR2Header.SessionGroup.strToken + "#" + pSess->m_RTSPR2Header.strClientSessionID;
			::std::vector<::Ice::Identity> identities = _env._idxSessionIdx->find(strTmpSessKey);

			//got stream
			if (identities.size() > 0)
				return NSSIdentityToObjEnv(_env, NGODStreamEx, identities[0]);
		}
	}
	return NULL;
}

::TianShanIce::Streamer::NGODStreamServer::NGODStreamPrx NGODStreamServiceImpl::
findStreamByNssSession(const ::std::string& sopName, 
					   const ::std::string& nssSessionId,
					   const ::Ice::Current& c)
{
	for (NSSSessionGroupList::iterator iter = _nssSessionGroupList.begin();
	iter != _nssSessionGroupList.end(); iter++)
	{
		string strTmpSessKey;
		strTmpSessKey = (*iter)->strSessionGroup + "#" + nssSessionId;
		::std::vector<::Ice::Identity> identities = _env._idxSessionIdx->find(strTmpSessKey);

		//got stream
		if (identities.size() > 0)
			return NSSIdentityToObjEnv(_env, NGODStreamEx, identities[0]);
	}

	//could not find stream
	return NULL;
}

::std::string NGODStreamServiceImpl::
getAdminUri(const ::Ice::Current& c)
{
	return string("");
}

::TianShanIce::State NGODStreamServiceImpl::
getState(const ::Ice::Current& c)
{
	return ::TianShanIce::stNotProvisioned;
}

//don't need impl
void NGODStreamServiceImpl::
queryReplicas_async(const ::TianShanIce::AMD_ReplicaQuery_queryReplicasPtr& amdReplicaQuery, const ::std::string& category, const ::std::string& groupId, bool Only, const ::Ice::Current& c)
{
	::TianShanIce::Replicas replicas;
	amdReplicaQuery->ice_response(replicas);
}

NSSSessionGroup * NGODStreamServiceImpl::
InitSessionGroup(string &strSessionGroup, string &strServerPath, uint16 uServerPort)
{
	NSSSessionGroup *pGroup = new NSSSessionGroup();
	
	pGroup->m_SessionSocket.m_Socket = CreateSocket(TCPSOCKET);
	pGroup->m_SessionSocket.m_Status = true;
	pGroup->usClientSeq = 1;
	pGroup->usServerSeq = 1;
	pGroup->uServerPort = _uServerPort;
	pGroup->strServerPath = _strServerPath;
	pGroup->strSessionGroup = strSessionGroup;
	InitializeCriticalSection(&pGroup->m_CS_SessionMap);
	InitializeCriticalSection(&pGroup->m_CS);
	InitializeCriticalSection(&pGroup->m_CS_ClientSeq);
	InitializeCriticalSection(&pGroup->m_CS_ServerSeq);
	if (bConnection(strServerPath, uServerPort, pGroup->m_SessionSocket.m_Socket, 1))
		return pGroup;
	else
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(NSS, "fail to create Session group(%s) connected %s"), strSessionGroup.c_str(), strServerPath.c_str());
		//closesocket(pGroup->m_SessionSocket.m_Socket);
		pGroup->m_SessionSocket.m_Status = false;
		//DeleteCriticalSection(&pGroup->m_CS);
		//DeleteCriticalSection(&pGroup->m_CS_ClientSeq);
		//DeleteCriticalSection(&pGroup->m_CS_ServerSeq);
		//delete pGroup;
		//return NULL;
		return pGroup;
	}
}

RTSPClientSession* NGODStreamServiceImpl::
InitRTSPSession(const ::TianShanIce::Transport::PathTicketPrx &_pathTicket)
{
	RTSPClientSession *rtspClientSession = new RTSPClientSession();

	::TianShanIce::Transport::StorageLinkPrx sLink = _pathTicket->getStorageLink();
	rtspClientSession->m_RTSPR2Header.Volume.strName = sLink->getStorageInfo().netId;
	
	::TianShanIce::ValueMap pMap = _pathTicket->getPrivateData();
	::TianShanIce::SRM::ResourceMap resMap = _pathTicket->getResources();
	::std::string strTicket = _env._communicator->proxyToString(_pathTicket);

	RTSPTransportUdpHeader pTransHeader;

	TianShanIce::Variant	valBandwidth;
	TianShanIce::Variant	valDestAddr;
	TianShanIce::Variant	valDestPort;
	TianShanIce::Variant	valDestMac;
//	TianShanIce::Variant	valServerIP;
//	TianShanIce::Variant	valServerPort;
//	TianShanIce::Variant	valNATEnable;
//	TianShanIce::Variant	valPokeHoleSessID;

	try
	{
		//get client mac(client_id) ,and mac may not available
		try
		{
			valDestMac = GetResourceMapData(resMap,TianShanIce::SRM::rtEthernetInterface,"destMac");
			if ( valDestMac.type != TianShanIce::vtStrings || valDestMac.strs.size () == 0 )
			{
				delete rtspClientSession;
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(envlog,EXPFMT("NSS",1045,"invalid dest mac type,should be vtStrings,or no destmac data with ticket [%s]"), strTicket.c_str());
				return NULL;
			}
			pTransHeader.strClient_id = valDestMac.strs[0];
			envlog(::ZQ::common::Log::L_INFO,EXPFMT("NSS",872,"Get dest mac [%s] through ticket[%s]"), pTransHeader.strClient_id.c_str (),strTicket.c_str ());
		}
		catch (TianShanIce::InvalidParameter&)
		{
			pTransHeader.strClient_id = "";
			envlog(::ZQ::common::Log::L_INFO,EXPFMT("NSS",873,"no dest mac information in ticket[%s]"),strTicket.c_str());
		}

		//get destination IP
		valDestAddr = GetResourceMapData (resMap,TianShanIce::SRM::rtEthernetInterface,"destIP");
		if (valDestAddr.type != TianShanIce::vtStrings || valDestAddr.strs.size () == 0)
		{
			delete rtspClientSession;
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(envlog,EXPFMT("NSS",1043,"invalid destAddress type should be vtString or no destAddress data with ticket [%s]"), strTicket.c_str());		
			return NULL;
		}
		else
		{
			pTransHeader.strDestination = valDestAddr.strs[0];
			envlog(::ZQ::common::Log::L_INFO,EXPFMT("NSS",873, "Get DestIP[%s] through ticket[%s]"),pTransHeader.strDestination.c_str (),strTicket.c_str ());
		}

		//get destination port
		valDestPort = GetResourceMapData(resMap,TianShanIce::SRM::rtEthernetInterface,"destPort");
		if ( valDestPort.type != TianShanIce::vtInts || valDestPort.ints.size() == 0 )
		{
			delete rtspClientSession;
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(envlog,EXPFMT("NSS",1044,"invalid dest por type,should be vtInts,or no destPort data with ticket [%s]"),strTicket.c_str());
			return NULL;
		}
		else
		{
			::std::stringstream ss;
			ss << valDestPort.ints[0];
			pTransHeader.strClient_port = ss.str();
			envlog(::ZQ::common::Log::L_INFO,EXPFMT("NSS",872,"Get destPort[%s] through ticket[%s]"),pTransHeader.strClient_port.c_str(),strTicket.c_str ());
		}

		//get bandwidth
		valBandwidth = GetResourceMapData(resMap , TianShanIce::SRM::rtTsDownstreamBandwidth,"bandwidth");
		if (valBandwidth.type != TianShanIce::vtLongs || valBandwidth.lints.size () == 0 )
		{
			delete rtspClientSession;
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(envlog,EXPFMT("NSS",872, "Invalid BandWidth type should be vtLongs , or no bandwidth data with ticket [%s]"),	strTicket.c_str ());
			return NULL;
		}
		else
		{
			::std::stringstream ss;
			long lBandwidth;
			lBandwidth = *(long*)getVariantValue(valBandwidth);
			ss << lBandwidth;
			pTransHeader.strBandwidth = ss.str();
			envlog(::ZQ::common::Log::L_INFO,EXPFMT("NSS",872, "Get bandwidth [%d]bps through ticket[%s]"), lBandwidth,strTicket.c_str ());
		}

		pTransHeader.strSop_name = *((string *)getPrivateDataValue(PathTicketPD_Field(sop_name), pMap));
	}
	catch(...)
	{
		delete rtspClientSession;
		return NULL;
	}

	pTransHeader.strType = "MP2T/DVBC/UDP";
	rtspClientSession->m_RTSPR2Header.m_RTSPTransportHeader.TransportUdpHeader.push_back(pTransHeader);

	rtspClientSession->fScale = 1.00;
	rtspClientSession->strCurrentTimePoint = "0";
	return rtspClientSession;
}

void* getVariantValue(::TianShanIce::Variant &val)
{
	switch (val.type)
	{
		case ::TianShanIce::vtStrings:
			return (void *)&(val.strs[0]);
		case ::TianShanIce::vtBin:
			return (void *)&(val.bin);
		case ::TianShanIce::vtFloats:
			return (void *)&(val.floats);
		case ::TianShanIce::vtInts:
			return (void *)&(val.ints);
		case ::TianShanIce::vtLongs:
			return (void *)&(val.lints[0]);
		default:
			return "";
	}
}

void* getPrivateDataValue(const ::std::string &str, ::TianShanIce::ValueMap &pVal)
{
	::TianShanIce::ValueMap::iterator iter;
	iter = pVal.find(str);
	if (iter == pVal.end())
		return "";
	else
		return getVariantValue((*iter).second);
}

TianShanIce::Variant GetResourceMapData(const TianShanIce::SRM::ResourceMap& rcMap,
										const TianShanIce::SRM::ResourceType& type,
										const std::string& strkey)
{
	TianShanIce::SRM::ResourceMap::const_iterator itResMap=rcMap.find(type);
	if(itResMap==rcMap.end())
	{
		char szBuf[1024];
		sprintf(szBuf,"GetResourceMapData() type %d not found",type);
		
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NSS",1001,szBuf );
	}
	TianShanIce::ValueMap::const_iterator it=itResMap->second.resourceData.find(strkey);
	if(it==itResMap->second.resourceData.end())
	{
		char szBuf[1024];
		sprintf(szBuf,"GetResourceMapData() value with key=%s not found",strkey.c_str());
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("NSS",1002,szBuf);
	}
	return it->second;
}

}//NSS
}//ZQTianShan
