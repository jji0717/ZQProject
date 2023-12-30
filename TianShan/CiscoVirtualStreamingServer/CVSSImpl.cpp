// **********************************************************************
//
//
// **********************************************************************

#include <winsock2.h>
#include "CVSSImpl.h"
#include <IceUtil/UUID.h>
#include "RTSPLib/rtsp_action.h"
#include "RTSPLib/CVSSRtspParser/ClientSocket.h"
#include "IPathHelperObj.h"
#include <sstream>
#include "CVSSCommitThrd.h"
extern int32 iDefaultBufferSize;

namespace ZQTianShan{
namespace CVSS{

static ::std::string strCategory = "CVSS";
/***************
//CiscoVirtualStreamImpl
****************/
CiscoVirtualStreamImpl::
CiscoVirtualStreamImpl(CVSSEnv &env):
_env(env),
_cvssRTSPSession(NULL),
_logFile(env._logFile),
_pool(env._thpool)
{
	if (!sessKey.empty())
	{
		_index = atoi(sessKey.c_str());
		if (sessId.empty())
			_cvssRTSPSession = env._daemonThrd->findRTSPSession(_index);
	}
	else if (!sessId.empty())
		_cvssRTSPSession = env._daemonThrd->findRTSPSession(sessId);
	else
		_cvssRTSPSession = NULL;
}

CiscoVirtualStreamImpl::~CiscoVirtualStreamImpl()
{
}

void CiscoVirtualStreamImpl::renewPathTicket(const ::Ice::Current& c)
{
	::Ice::Identity tmpident = _env._communicator->stringToIdentity(pathTicketStr);
	::TianShanIce::Transport::PathTicketPrx& _pathTicketPrx = ::TianShanIce::Transport::PathTicketPrx::uncheckedCast(_env._adapter->createProxy(tmpident));
	int32 iTime = GetTickCount();
	try
	{
		if(_pathTicketPrx!=NULL)
		{
			_pathTicketPrx->renew(iTime);//renew pathticket
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
		envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(renewPathTicket,"session(%s) renew fail with renew time =%d and error is %s"), ident.name.c_str(), iTime,ex.ice_name().c_str());
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

void CiscoVirtualStreamImpl::
setItemInfo(::TianShanIce::Streamer::CiscoVirtualStreamServer::stItem &item, ::Ice::Int userCtrlNum, const ::TianShanIce::Streamer::PlaylistItemSetupInfo& newItemInfo)
{
	//record item info
	item.contentName		= newItemInfo.contentName;
	item.criticalStart		= (time_t)newItemInfo.criticalStart;
	item.userCtrlNum		= userCtrlNum;
	item.forceNormal		= newItemInfo.forceNormal;
	item.inTimeOffset		= (uint32)newItemInfo.inTimeOffset;
	item.outTimeOffset		= (uint32)newItemInfo.outTimeOffset;
	item.spliceIn			= newItemInfo.spliceIn;
	item.spliceOut			= newItemInfo.spliceOut;
	item.flags				= newItemInfo.flags;
}

void CiscoVirtualStreamImpl::setPlayItem(::Ice::Long timeOffset,::Ice::Int startPos)
{
	if (startPos == 2)
	{
		int iTime = 0;

		for (::TianShanIce::Streamer::CiscoVirtualStreamServer::ItemList::const_iterator iter = playList.end(); iter != playList.begin(); iter--)
		{
			iTime = iTime + (iter->contentLen);
			if (iTime > timeOffset)
			{
				iTime = iTime - timeOffset;
				stringstream ss;
				ss << iTime;
				_cvssRTSPSession->_commonReqHeader._strRang = ss.str();
				_cvssRTSPSession->_commonReqHeader._strContentId = iter->contentName;
				break;
			}
		}
	}
	else if (startPos == 1)
	{
		int iTime = 0;

		for (::TianShanIce::Streamer::CiscoVirtualStreamServer::ItemList::const_iterator iter = playList.begin(); iter != playList.end(); iter++)
		{
			iTime = iTime + (iter->contentLen);
			if (iTime > timeOffset)
			{
				iTime = timeOffset - (iTime - iter->contentLen);
				stringstream ss;
				ss << iTime;
				_cvssRTSPSession->_commonReqHeader._strRang = ss.str();
				_cvssRTSPSession->_commonReqHeader._strContentId = iter->contentName;
				break;
			}
		}
	}
}
void CiscoVirtualStreamImpl::
setRtspClientSession(CVSSRtspSession *rtspClientSession)
{
	_cvssRTSPSession = rtspClientSession;
}

//impl of CVSS
::std::string CiscoVirtualStreamImpl::
getSessionId(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return sessId;
}

::std::string CiscoVirtualStreamImpl::
getCtrlURL(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return controlURl;
}

::TianShanIce::Streamer::CiscoVirtualStreamServer::ItemList CiscoVirtualStreamImpl::getItemList(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return playList;
}

::Ice::Long CiscoVirtualStreamImpl::getCurPlayListIdx(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return curPlayListIdx;
}

::std::string CiscoVirtualStreamImpl::getPathTicketStr(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return pathTicketStr;
}

::std::string CiscoVirtualStreamImpl::getDestination(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return destination;
}

::Ice::Long CiscoVirtualStreamImpl::getClientPort(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return clientPort;
}

void CiscoVirtualStreamImpl::doHeartBeat(const ::Ice::Current& c) const
{
	RLock sync(*this);
	checkSessionStatus("doHeartBeat");

	_cvssRTSPSession->_getParameterReqHeader._strSDPField.clear();

	//TODO: send play message
	DWORD iTime = GetTickCount();
	bool b = rtsp_action::RTSPAction(GETPARAMETER, _env._rtspCSeqSignal, _cvssRTSPSession, _pool, envlog);
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(CiscoVirtualStreamImpl, "heartbeat: Ident(%s)Socket(%d)session(%s)CSeq(%d) send GET_PARAMETER over, cost %dms"), ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq, GetTickCount() - iTime);

	if (!b)
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"CiscoVirtualStreamImpl",173,"heartbeat: Ident(%s)Socket(%d)session(%s)CSeq(%d) fail to heartbeat with video server", ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq);
}

void CiscoVirtualStreamImpl::playNextItem(const ::Ice::Current& c)
{
	WLock sync(*this);
	checkSessionStatus("playNextItem");
	for (::TianShanIce::Streamer::CiscoVirtualStreamServer::ItemList::iterator iter = playList.begin(); iter != playList.end(); iter++)
	{
		if (curPlayListIdx == iter->userCtrlNum)
		{
			iter++;
			if (iter == playList.end())
				break;
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(CiscoVirtualStreamImpl, "playNextItem(%d): Ident(%s)Socket(%d)Sess(%s)CSeq(%d) find next item(%d)"), curPlayListIdx, ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId,_cvssRTSPSession->_commonReqHeader._iCSeq, iter->userCtrlNum);
			_cvssRTSPSession->_commonReqHeader._strContentId = iter->contentName;
			stringstream ss;
			ss << iter->inTimeOffset;
			_cvssRTSPSession->_commonReqHeader._strRang = ss.str();

			//try to send PLAY
			DWORD iTime = GetTickCount();
			bool b = rtsp_action::RTSPAction(PLAY, _env._rtspCSeqSignal, _cvssRTSPSession, _pool, _logFile);
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(CiscoVirtualStreamImpl, "playNextItem: Ident(%s)Socket(%d)Sess(%s)CSeq(%d) send play over, cost %dms"), ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId,_cvssRTSPSession->_commonReqHeader._iCSeq, GetTickCount() - iTime);
			return;
		}
	}

	envlog(ZQ::common::Log::L_INFO, CLOGFMT(CiscoVirtualStreamImpl, "playNextItem(%d): Ident(%s)Socket(%d)Sess(%s)CSeq(%d) can't find next item"), curPlayListIdx, ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId,_cvssRTSPSession->_commonReqHeader._iCSeq);
}

//impl of stream
void CiscoVirtualStreamImpl::
allotPathTicket(const ::TianShanIce::Transport::PathTicketPrx& ticket,
				const ::Ice::Current& c)
	throw (::TianShanIce::Transport::ExpiredTicket,
		   ::TianShanIce::ServerError,
		   ::TianShanIce::NotSupported)
{
	WLock sync(*this);
	if (ticket == NULL)
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"CiscoVirtualStreamImpl",180,"allotPathTicket:Ident(%s), input parameter error", ident.name.c_str());

	pathTicketStr = _env._communicator->proxyToString(ticket);
}

void CiscoVirtualStreamImpl::
destroy(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError)
{
	WLock sync(*this);
	checkSessionStatus("destroy");

	_env._eCvssStream->remove(ident);

	//try to send TEARDOWM
	DWORD iTime = GetTickCount();
	bool b = rtsp_action::RTSPAction(TEARDOWN, _env._rtspCSeqSignal, _cvssRTSPSession, _pool, _logFile);
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(CiscoVirtualStreamImpl, "destroy: Ident(%s)Socket(%d)Sess(%s)CSeq(%d) send TEARDOWN over, cost %dms"), ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId,_cvssRTSPSession->_commonReqHeader._iCSeq, GetTickCount() - iTime);

	_env._daemonThrd->removeRTSPSession(_cvssRTSPSession->_rtspSocket._socket);
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoVirtualStreamImpl,"destroy: Ident(%s)Socket(%d)Sess(%s)CSeq(%d)) fail to drive video server"), ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(),_cvssRTSPSession->_commonReqHeader._iCSeq, ident.name.c_str());

	delete _cvssRTSPSession;
	_cvssRTSPSession = NULL;

	if (!b)
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"CiscoVirtualStreamImpl",146,"destroy: Ident(%s) fail to destroy session from video server", ident.name.c_str());
}

void CiscoVirtualStreamImpl::
commit_async(const ::TianShanIce::Streamer::AMD_Stream_commitPtr& amdStream, 
			 const ::Ice::Current& c)
{	
	//no play list
	if (playList.empty())
		amdStream->ice_response();

	//set content id and start range
	::TianShanIce::Streamer::CiscoVirtualStreamServer::ItemList::iterator iter = playList.begin();
	_cvssRTSPSession->_commonReqHeader._strContentId = iter->contentName;
	stringstream ss;
	ss << iter->inTimeOffset;
	_cvssRTSPSession->_commonReqHeader._strRang = ss.str();
	CVSSCommitThrd *commitThrd = new CVSSCommitThrd(this, amdStream);
	commitThrd->start();
}

::std::string CiscoVirtualStreamImpl::
lastError(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return string("");
}
  
::Ice::Identity CiscoVirtualStreamImpl::
getIdent(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return ident;
}

void CiscoVirtualStreamImpl::
setConditionalControl(const ::TianShanIce::Streamer::ConditionalControlMask& mask, 
					  const ::TianShanIce::Streamer::ConditionalControlPrx& condCtrl, 
					  const ::Ice::Current& c)
{
}
    
bool CiscoVirtualStreamImpl::play(const ::Ice::Current& c)
{
	RLock sync(*this);
	checkSessionStatus("play");

	RTSPClientState oldState = _cvssRTSPSession->iRTSPClientState;

	//TODO: send play message
	DWORD iTime = GetTickCount();
	bool b = rtsp_action::RTSPAction(PLAY, _env._rtspCSeqSignal, _cvssRTSPSession, _pool, envlog);
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(CiscoVirtualStreamImpl, "play: Ident(%s)Socket(%d)session(%s)CSeq(%d) send PLAY over, cost %dms"), ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq, GetTickCount() - iTime);
	
	if (!b)
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"CiscoVirtualStreamImpl",173,"play: Ident(%s)Socket(%d)session(%s)CSeq(%d) fail to drive video server", ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq);
	else
	{
		if (oldState != _cvssRTSPSession->iRTSPClientState)
		{
			//send state changed event
			::ZQTianShan::CVSS::listmem params;
			params.type = ::ZQTianShan::CVSS::E_PLAYLIST_STATECHANGED;
			params.param[EventField_PlaylistGuid] = _cvssRTSPSession->_commonReqHeader._strSessionId;
			params.param[EventField_EventCSEQ] = (long)(_cvssRTSPSession->_commonReqHeader._iCSeq);
			_env._daemonThrd->_eventList.PushBack(params);
		}
	}
	return b;
}
    
bool CiscoVirtualStreamImpl::
setSpeed(::Ice::Float newSpeed,
		 const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter, 
		   ::TianShanIce::ServerError, 
		   ::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	checkSessionStatus("setSpeed");

	::Ice::Float fOldSpeed = (::Ice::Float)atof(_cvssRTSPSession->_commonResHeader._strScale.c_str());
	stringstream ss;
	ss << newSpeed;
	_cvssRTSPSession->_commonReqHeader._strScale = ss.str();
	DWORD iTime = GetTickCount();
	bool b = rtsp_action::RTSPAction(PLAY, _env._rtspCSeqSignal, _cvssRTSPSession, _pool, envlog);
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(CiscoVirtualStreamImpl, "setSpeed: Ident(%s)Socket(%d)session(%s)CSeq(%d) send PLAY over, cost %dms"), ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq, GetTickCount() - iTime);
	
	if (!b)
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"CiscoVirtualStreamImpl",174,"setSpeed: Ident(%s)Socket(%d)session(%s)CSeq(%d) failed to drive video server", ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq);
	else
	{
		::Ice::Float fScale = (::Ice::Float)atof(_cvssRTSPSession->_commonResHeader._strScale.c_str());
		if (fScale != fOldSpeed)
		{
			::ZQTianShan::CVSS::listmem params;
			params.type = ::ZQTianShan::CVSS::E_PLAYLIST_SPEEDCHANGED;
			params.param[EventField_PlaylistGuid] = _cvssRTSPSession->_commonReqHeader._strSessionId;
			params.param[EventField_EventCSEQ] = (long)(_cvssRTSPSession->_commonReqHeader._iCSeq);
			_env._daemonThrd->_eventList.PushBack(params);
		}
	}
	return b;
}
    
bool CiscoVirtualStreamImpl::
pause(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError, 
		   ::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	checkSessionStatus("pause");

	if (_cvssRTSPSession->iRTSPClientState != PAUSE)
	{
		DWORD iTime = GetTickCount();
		bool b = rtsp_action::RTSPAction(PAUSE, _env._rtspCSeqSignal, _cvssRTSPSession, _pool, _logFile);
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(CiscoVirtualStreamImpl, "pause: Ident(%s)Socket(%d)Session(%s)CSeq(%d) send PAUSE over, cost %dms"), ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq, GetTickCount() - iTime);
		
		if (!b)
			ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"CiscoVirtualStreamImpl",176,"pause: Ident(%s)Socket(%d)Session(%s)CSeq(%d) failed to drive video server", ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq);
		return b;
	}
	else
	{
		ZQTianShan::_IceThrow<::TianShanIce::InvalidStateOfArt> (envlog,"CiscoVirtualStreamImpl",177,"pause: Ident(%s)Socket(%d)Session(%s) client already in PAUSE state", ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq);
		return false;
	}
}
    
bool CiscoVirtualStreamImpl::
resume(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError, 
		   ::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	checkSessionStatus("resume");

	if (_cvssRTSPSession->iRTSPClientState != PAUSE)
	{
		DWORD iTime = GetTickCount();
		bool b = rtsp_action::RTSPAction(PLAY, _env._rtspCSeqSignal, _cvssRTSPSession, _pool, _logFile);
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(CiscoVirtualStreamImpl, "setSpeed: Ident(%s)Socket(%d)Session(%s)CSeq(%d) send PLAY over, cost %dms"), ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq, GetTickCount() - iTime);
		
		if (!b)
			ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"CiscoVirtualStreamImpl",179,"resume: Ident(%s)Socket(%d)Session(%s)CSeq(%d) failed to drive video server", ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq);
		return b;
	}
	else
	{
		ZQTianShan::_IceThrow<::TianShanIce::InvalidStateOfArt> (envlog,"CiscoVirtualStreamImpl",177,"resume: Ident(%s)Socket(%d)Session(%s) client already in PLAY state", ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq);
		return false;
	}
}
    
::TianShanIce::Streamer::StreamState CiscoVirtualStreamImpl::
getCurrentState(const ::Ice::Current& c) const
	throw (::TianShanIce::ServerError)
{
	RLock sync(*this);
	checkSessionStatus("getCurrentState");

	envlog(ZQ::common::Log::L_INFO, CLOGFMT(CiscoVirtualStreamImpl, "Ident(%s)Socket(%d)Session(%s)CSeq(%d) enter getCurrentState"), ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq);
	if (_cvssRTSPSession != NULL)
	{
		switch (_cvssRTSPSession->iRTSPClientState)
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
    
::TianShanIce::SRM::SessionPrx CiscoVirtualStreamImpl::
getSession(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError)
{
	return NULL;
	//TODO
}

void CiscoVirtualStreamImpl::
setMuxRate(::Ice::Int nowRate, 
		   ::Ice::Int maxRate,
		   ::Ice::Int minRate,
		   const ::Ice::Current& c)
{
}

bool CiscoVirtualStreamImpl::
allocDVBCResource(::Ice::Int serviceGroupID,
				  ::Ice::Int bandWidth,
				  const ::Ice::Current& c)
	throw (::TianShanIce::ServerError)
{
	return true;
}

::Ice::Long CiscoVirtualStreamImpl::
seekStream(::Ice::Long timeOffset,
		   ::Ice::Int startPos,
		   const ::Ice::Current& c)
	throw (::TianShanIce::ServerError,
		   ::TianShanIce::InvalidParameter)
{
	RLock sync(*this);
	checkSessionStatus("seekStream");

	if (startPos == 0)
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"CiscoVirtualStreamImpl",180,"CiscoVirtualStreamImpl::Ident(%s)Socket(%d)Session(%s)CSeq(%d) seekToPosition() seek from current position don't supported", ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq);

	//if (startPos == 1 && timeOffset < 0)
	//	ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"CiscoVirtualStreamImpl",180,"CiscoVirtualStreamImpl::Ident(%s)Socket(%d)Session(%s)CSeq(%d) seekToPosition() seek from begin but offset(%d) is negative", ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq, timeOffset);

	//if (startPos == 2 && timeOffset > 0)
	//	ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"CiscoVirtualStreamImpl",180,"CiscoVirtualStreamImpl::Ident(%s)Socket(%d)Session(%s)CSeq(%d) seekToPosition() seek from end but offset(%d) is positive", ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq, timeOffset);

	setPlayItem(timeOffset, startPos);

	DWORD iTime = GetTickCount();
	bool b = rtsp_action::RTSPAction(PLAY, _env._rtspCSeqSignal, _cvssRTSPSession, _pool, _logFile);
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(CiscoVirtualStreamImpl, "seekStream: Ident(%s)Socket(%d)session(%s)CSeq(%d) send PLAY over, cost %dms"), ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq, GetTickCount() - iTime);
	if (b)
	{
		::TianShanIce::StrValues expectedProps;
		::TianShanIce::Streamer::StreamInfo streamInfo = playEx((::Ice::Float)atof(_cvssRTSPSession->_getParameterResHeader._strRange.c_str()), timeOffset, startPos, expectedProps, c);
		return atoi(streamInfo.props["CURRENTPOS"].c_str());
	}
	else
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"CiscoVirtualStreamImpl",180,"seekToPosition:Ident(%s)Socket(%d)Session(%s)CSeq(%d) fail to drive video server", ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq, timeOffset);
		return false;
	}
}

::TianShanIce::Streamer::StreamInfo CiscoVirtualStreamImpl::
playEx(::Ice::Float newSpeed, 
	   ::Ice::Long timeOffset,
	   ::Ice::Short from,
	   const ::TianShanIce::StrValues& expectedProps,
	   const ::Ice::Current& c)
	   throw (::TianShanIce::ServerError, 
	   ::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	checkSessionStatus("playEx");
	DWORD iTime;
	::TianShanIce::Streamer::StreamInfo streamInfo;

	bool b = false;
	//::Ice::Float fPos = 0;
	//if (from < 0)
	//{
	//	fPos = (::Ice::Float)atof(_cvssRTSPSession->_describeResHeader._strRangeEnd.c_str()) - (::Ice::Float)atof(_cvssRTSPSession->_describeResHeader._strRangeStart.c_str()) - timeOffset;
	//}
	//else if (from > 0)
	//	fPos = (::Ice::Long)timeOffset / 1000;
	stringstream ss;
	//ss << fPos;
	//_cvssRTSPSession->_commonReqHeader._strRang = ss.str();

	ss.clear();
	ss << newSpeed;
	_cvssRTSPSession->_commonReqHeader._strScale = ss.str();
	setPlayItem(timeOffset, from);

	iTime = GetTickCount();
	b = rtsp_action::RTSPAction(PLAY, _env._rtspCSeqSignal, _cvssRTSPSession, _pool, _logFile);
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(CiscoVirtualStreamImpl, "playEx: Ident(%s)Socet(%d)Session(%s), CSeq(%d) send PLAY over cost %dms, "), ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket,_cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq, GetTickCount() - iTime);

	if (!b)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"CiscoVirtualStreamImpl",180,"playEx: fail to play, OnDemandSession(%s), SessionId(%s), CSeq(%d)", ident.name.c_str(), sessId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq);
		streamInfo.state	= TianShanIce::Streamer::stsStop;			
	}
	else
	{
		streamInfo.state = TianShanIce::Streamer::stsStreaming;
		float fCurPos = (::Ice::Float)atof(_cvssRTSPSession->_commonResHeader._strRangeStart.c_str());
		stringstream ss;
		ss << fCurPos * 1000;
		streamInfo.props["CURRENTPOS"] = ss.str();
		
		streamInfo.props["SPEED"] = _cvssRTSPSession->_commonResHeader._strScale;
	}


	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoVirtualStreamImpl,"CiscoVirtualStreamImpl(OnDemandSession(%s), SessionId(%s), CSeq(%d)) playEx successfully"), ident.name.c_str(), sessId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq);
	return streamInfo;
}

::TianShanIce::Streamer::StreamInfo CiscoVirtualStreamImpl::
pauseEx(const ::TianShanIce::StrValues& expectedProps, 
		const ::Ice::Current& c)
		throw (::TianShanIce::ServerError, 
		::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	DWORD iTime;
	::TianShanIce::Streamer::StreamInfo streamInfo;
	if (_cvssRTSPSession != NULL)
	{
		bool b = false;

		iTime = GetTickCount();
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(CiscoVirtualStreamImpl, "pauseEx: begin send PAUSE, OnDemandSession(%s), SessionId(%s), CSeq(%d)"), ident.name.c_str(), sessId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq);
		b = rtsp_action::RTSPAction(PAUSE, _env._rtspCSeqSignal, _cvssRTSPSession, _pool, _logFile);
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(CiscoVirtualStreamImpl, "pauseEx: send PAUSE over cost %dms, OnDemandSession(%s), SessionId(%s), CSeq(%d)"), GetTickCount() - iTime, ident.name.c_str(), sessId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq);

		if (!b)
		{
			ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"CiscoVirtualStreamImpl",180,"pauseEx: fail to play, OnDemandSession(%s), SessionId(%s), CSeq(%d)", ident.name.c_str(), sessId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq);
			streamInfo.state	= TianShanIce::Streamer::stsStop;			
		}
		else
		{
			streamInfo.state = TianShanIce::Streamer::stsPause;
			stringstream ss;
			float fCurPos = (::Ice::Float)atof(_cvssRTSPSession->_commonResHeader._strRangeStart.c_str());
			ss << fCurPos * 1000;
			streamInfo.props["CURRENTPOS"] = ss.str();

			streamInfo.props["SPEED"] = _cvssRTSPSession->_commonResHeader._strScale;
		}
	}
	else
		ZQTianShan::_IceThrow<::TianShanIce::InvalidStateOfArt> (envlog,"CiscoVirtualStreamImpl",181,"pauseEx() fail to find exist session, OnDemandSession(%s), SessionId(%s), CSeq(%d)", ident.name.c_str(), sessId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq);

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(CiscoVirtualStreamImpl,"CiscoVirtualStreamImpl(Socket(%s), SessionId(%s), CSeq(%d)) pauseEx successfully"), ident.name.c_str(), sessId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq);
	return streamInfo;
}
    
void CiscoVirtualStreamImpl::
setSpeedEx(::Ice::Float newSpeed,
		   ::Ice::Long& timeOffset,
		   ::Ice::Float& currentSpeed,
		   const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter,
		   ::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	stringstream ss;

	//set speed
	ss << newSpeed;
	_cvssRTSPSession->_commonReqHeader._strScale = ss.str();

	//set time off set
	//ss.clear();
	//ss << timeOffset;
	//_cvssRTSPSession->_commonReqHeader._strRang = ss.str();
	setPlayItem(timeOffset, 1);

	DWORD iTime = GetTickCount();
	bool b = rtsp_action::RTSPAction(PAUSE, _env._rtspCSeqSignal, _cvssRTSPSession, _pool, _logFile);
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(CiscoVirtualStreamImpl, "setSpeedEx: send PLAY over cost %dms, Indent(%s), Socket(%d), SessionId(%s), CSeq(%d)"), GetTickCount() - iTime, ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, sessId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq);

	if (!b)
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"CiscoVirtualStreamImpl",180,"setSpeedEx: fail to play with new scale, Socket(%s), SessionId(%s), CSeq(%d)", ident.name.c_str(), sessId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq);
	else
	{
		float fCurSpeed = (::Ice::Float)atof(_cvssRTSPSession->_commonResHeader._strScale.c_str());
		if (fCurSpeed != newSpeed)
		{
			::ZQTianShan::CVSS::listmem params;
			params.type = ::ZQTianShan::CVSS::E_PLAYLIST_SPEEDCHANGED;
			params.param[EventField_PlaylistGuid] = ident.name;
			params.param[EventField_EventCSEQ] = (long)_cvssRTSPSession->_commonReqHeader._iCSeq;
			_env._daemonThrd->_eventList.PushBack(params);
		}
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(CiscoVirtualStreamImpl, "setSpeedEx: PLAY OK, Ident(%s) Socket(%s), SessionId(%s), CSeq(%d)"), ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, sessId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq);
	}
}
    
//impl of playlist
::std::string CiscoVirtualStreamImpl::
getId(const ::Ice::Current& c) const
{
	RLock sync(*this);
	//IceUtil::Mutex::Lock lock(*this);
	return ident.name;
}
   
bool CiscoVirtualStreamImpl::
getInfo(::Ice::Int mask,
		::TianShanIce::ValueMap& varOut,
		const ::Ice::Current& c)
{
	RLock sync(*this);
	checkSessionStatus("getInfo");

	envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(CiscoVirtualStreamImpl,"guid[%s]enter get info"), ident.name.c_str());

	switch(mask)
	{
	case TianShanIce::Streamer::infoSTREAMNPTPOS:
		break;
	default:
		return false;
	}

	bool b = false;

	ZQ::common::Variant var;
	if(!var.valid())
		return false;


	DWORD iTime = GetTickCount();
	_cvssRTSPSession->_getParameterReqHeader._strSDPField = g_strPosition;
	b = rtsp_action::RTSPAction(GETPARAMETER, _env._rtspCSeqSignal, _cvssRTSPSession, _pool, _logFile);
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(CiscoVirtualStreamImpl, "getInfo: send GETPARAMETER over cost %dms, Ident(%s), Socket(%d), SessionId(%s), CSeq(%d)"), GetTickCount() - iTime, ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, sessId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq, GetTickCount() - iTime);
	if (!b)
	{
		envlog(::ZQ::common::Log::L_WARNING,CLOGFMT(CiscoVirtualStreamImpl,"guid[%s]enter get info failed"), ident.name.c_str());
		return false;
	}
	else
	{
		TianShanIce::Variant res;
		res.ints.clear();
		res.bRange = false;
		res.ints.push_back(atoi(_cvssRTSPSession->_getParameterResHeader._strRange.c_str()));
		res.type=TianShanIce::vtInts;
		varOut["playposition"]=res;		

		res.bRange = false;
		res.strs.clear();
		res.strs.push_back(_cvssRTSPSession->_commonResHeader._strScale);
		res.type=TianShanIce::vtStrings;
		varOut["scale"]=res;
		envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(CiscoVirtualStreamImpl,"guid[%s]enter get info success"), ident.name.c_str());
	}

	return true;
}
   
::Ice::Int CiscoVirtualStreamImpl::
insert(::Ice::Int userCtrlNum, 
	   const ::TianShanIce::Streamer::PlaylistItemSetupInfo& newItemInfo,
	   ::Ice::Int whereUserCtrlNum,
	   const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter, 
		   ::TianShanIce::ServerError, 
		   ::TianShanIce::InvalidStateOfArt)
{
	WLock sync(*this);
	::TianShanIce::Streamer::CiscoVirtualStreamServer::stItem tmpItem;

	setItemInfo(tmpItem, userCtrlNum, newItemInfo);

	for (::TianShanIce::Streamer::CiscoVirtualStreamServer::ItemList::iterator iter = playList.begin(); iter != playList.end(); iter++)
	{
		if (iter->userCtrlNum == whereUserCtrlNum)
		{
			playList.insert(iter, tmpItem);
			return userCtrlNum;
		}
	}
	playList.push_back(tmpItem);
	return userCtrlNum;
}
   
::Ice::Int CiscoVirtualStreamImpl::
pushBack(::Ice::Int userCtrlNum,
		 const ::TianShanIce::Streamer::PlaylistItemSetupInfo& newItemInfo,
		 const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter,
		   ::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
	WLock sync(*this);
	::TianShanIce::Streamer::CiscoVirtualStreamServer::stItem tmpItem;

	setItemInfo(tmpItem, userCtrlNum, newItemInfo);

	playList.push_back(tmpItem);
	return userCtrlNum;
}
   
::Ice::Int CiscoVirtualStreamImpl::
size(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return playList.size();
}
   
::Ice::Int CiscoVirtualStreamImpl::
left(const ::Ice::Current& c) const
{
	::Ice::Int iLeft = playList.size();
	for (::TianShanIce::Streamer::CiscoVirtualStreamServer::ItemList::const_iterator iter = playList.begin(); iter != playList.end(); iter++)
	{
		if (curPlayListIdx != iter->userCtrlNum)
			iLeft--;
	}
	return iLeft;
}
   
bool CiscoVirtualStreamImpl::
empty(const ::Ice::Current& c) const
	throw (::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	return playList.empty();
}

::Ice::Int CiscoVirtualStreamImpl::
current(const ::Ice::Current& c) const
	throw (::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	return curPlayListIdx;
}
   
void CiscoVirtualStreamImpl::
erase(::Ice::Int whereUserCtrlNum,
	  const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter,
		   ::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
	WLock sync(*this);
	for (::TianShanIce::Streamer::CiscoVirtualStreamServer::ItemList::iterator iter = playList.begin(); iter != playList.end(); iter++)
	{
		if (whereUserCtrlNum == iter->userCtrlNum)
		{
			playList.erase(iter);
			envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(CiscoVirtualStreamImpl,"erase: Ident(%s)Socket(%d)Session(%s)CSeq(%d) find exist item(%d) and delete from playlist"), ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq, whereUserCtrlNum);
			return;
		}
	}
	ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"CiscoVirtualStreamImpl",704,"erase: Ident(%s)Socket(%d)Session(%s)CSeq(%d) fail to find exist item(%d)", ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq, whereUserCtrlNum);
}
   
::Ice::Int CiscoVirtualStreamImpl::
flushExpired(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
	return 1;
}
   
bool CiscoVirtualStreamImpl::
clearPending(bool includeInitedNext,
			 const ::Ice::Current& c)
	throw (::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
	return true;
}
   
bool CiscoVirtualStreamImpl::
isCompleted(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
		return true;
}
   
::TianShanIce::IValues CiscoVirtualStreamImpl::
getSequence(const ::Ice::Current& c) const
{
	RLock sync(*this);
	::TianShanIce::IValues tmpValues;
	::TianShanIce::Streamer::CiscoVirtualStreamServer::ItemList::const_iterator iter = playList.begin();
	for (; iter != playList.end(); iter++)
		tmpValues.push_back((*iter).userCtrlNum);
	return tmpValues;
}
   
::Ice::Int CiscoVirtualStreamImpl::
findItem(::Ice::Int userCtrlNum,
		 ::Ice::Int from,
		 const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter,
		   ::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	::TianShanIce::Streamer::CiscoVirtualStreamServer::ItemList::const_iterator iter = playList.begin();
	for (; iter != playList.end(); iter++)
	{
		if ((*iter).userCtrlNum == userCtrlNum)
			return (*iter).userCtrlNum;
	}
	return 0;
}
   
bool CiscoVirtualStreamImpl::
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
   
bool CiscoVirtualStreamImpl::
skipToItem(::Ice::Int where, 
		   bool bPlay,
		   const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter, 
		   ::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
	WLock sync(*this);
	int pos = 0;
	for (::TianShanIce::Streamer::CiscoVirtualStreamServer::ItemList::iterator iter = playList.begin(); iter != playList.end(); iter++)
	{
		if (iter->userCtrlNum == where)
		{
			curPlayListIdx = where;
			break;
		}
		pos += iter->contentLen;
	}
	if (curPlayListIdx != where)
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"CiscoVirtualStreamImpl",180,"skipToItem:Ident(%s)Socket(%d)Session(%s)CSeq(%d) fail to find the specify item(%d)", ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq, where);

	stringstream ss;
	ss << 0;
	_cvssRTSPSession->_commonReqHeader._strRang = ss.str();
	return play(c);
}
   
bool CiscoVirtualStreamImpl::
seekToPosition(::Ice::Int UserCtrlNum,
			   ::Ice::Int timeOffset,
			   ::Ice::Int startPos,
			   const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter,
		   ::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
	if (startPos == 0)
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"CiscoVirtualStreamImpl",180,"CiscoVirtualStreamImpl::Ident(%s)Socket(%d)Session(%s)CSeq(%d) seekToPosition() seek from current position don't supported", ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq);

	if (startPos == 1 && timeOffset < 0)
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"CiscoVirtualStreamImpl",180,"CiscoVirtualStreamImpl::Ident(%s)Socket(%d)Session(%s)CSeq(%d) seekToPosition() seek from begin but offset(%d) is negative", ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq, timeOffset);

	if (startPos == 2 && timeOffset > 0)
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"CiscoVirtualStreamImpl",180,"CiscoVirtualStreamImpl::Ident(%s)Socket(%d)Session(%s)CSeq(%d) seekToPosition() seek from end but offset(%d) is positive", ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq, timeOffset);

	int iTime = 0;
	for (::TianShanIce::Streamer::CiscoVirtualStreamServer::ItemList::const_iterator iter = playList.begin(); iter != playList.end(); iter++)
	{
		if (iter->userCtrlNum == UserCtrlNum)
		{
			if (startPos == 2)
				iTime = iTime + (iter->contentLen);
			iTime += timeOffset;
			::std::stringstream ss;
			ss << iTime;
			_cvssRTSPSession->_commonReqHeader._strRang = ss.str();
			curPlayListIdx = UserCtrlNum;
			return play(c);
		}
		//iTime += iter->contentLen;
	}
	ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"CiscoVirtualStreamImpl",180,"CiscoVirtualStreamImpl::Ident(%s)Socket(%d)Session(%s)CSeq(%d) seekToPosition() the given userCtrlNum(%d) is invalid", ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq, UserCtrlNum);
	return false;
}
   
void CiscoVirtualStreamImpl::
enableEoT(bool enable,
		  const ::Ice::Current& c)
	throw (::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
}

::TianShanIce::Streamer::StreamInfo CiscoVirtualStreamImpl::
playItem(::Ice::Int UserCtrlNum,
		 ::Ice::Int timeOffset,
		 ::Ice::Short startPos,
		 ::Ice::Float newSpeed,
		 const ::TianShanIce::StrValues& expectedProps,
		 const ::Ice::Current& c)
		 throw (::TianShanIce::ServerError,
		 ::TianShanIce::InvalidStateOfArt, 
		 ::TianShanIce::InvalidParameter)
{
	if (startPos == 0)
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"CiscoVirtualStreamImpl",180,"CiscoVirtualStreamImpl::Ident(%s)Socket(%d)Session(%s)CSeq(%d) seekToPosition() seek from current position don't supported", ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq);

	if (startPos == 1 && timeOffset < 0)
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"CiscoVirtualStreamImpl",180,"CiscoVirtualStreamImpl::Ident(%s)Socket(%d)Session(%s)CSeq(%d) seekToPosition() seek from begin but offset(%d) is negative", ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq, timeOffset);

	if (startPos == 2 && timeOffset > 0)
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"CiscoVirtualStreamImpl",180,"CiscoVirtualStreamImpl::Ident(%s)Socket(%d)Session(%s)CSeq(%d) seekToPosition() seek from end but offset(%d) is positive", ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq, timeOffset);

	int iTime = 0;
	for (::TianShanIce::Streamer::CiscoVirtualStreamServer::ItemList::const_iterator iter = playList.begin(); iter != playList.end(); iter++)
	{
		if (iter->userCtrlNum == UserCtrlNum)
		{
			if (startPos == 2)
				iTime = iTime + (iter->contentLen);
			iTime += timeOffset;
			return playEx(newSpeed, iTime, startPos, expectedProps, c);
		}
		iTime += iter->contentLen;
	}
	ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"CiscoVirtualStreamImpl",180,"CiscoVirtualStreamImpl::Ident(%s)Socket(%d)Session(%s)CSeq(%d)  seekToPosition() the given userCtrlNum(%d) is invalid", ident.name.c_str(), _cvssRTSPSession->_rtspSocket._socket, _cvssRTSPSession->_commonReqHeader._strSessionId.c_str(), _cvssRTSPSession->_commonReqHeader._iCSeq, UserCtrlNum);
	::TianShanIce::Streamer::StreamInfo streamInfo;
	return streamInfo;
}

/***********************
//CiscoVirtualStreamServiceImpl
************************/
CiscoVirtualStreamServiceImpl::
CiscoVirtualStreamServiceImpl(ZQ::common::FileLog &LogFile,
							  ::ZQ::common::NativeThreadPool &recvPool,
							  ::ZQ::common::NativeThreadPool &sendPool,
							  ::Ice::CommunicatorPtr& communicator,
							  ::std::string &strServerPath,
							  uint16 &uServerPort,
							  int32 &evictoreSize,
							  CVSSEnv &env):
_env(env)
,_logFile(LogFile)
,_recvPool(recvPool)
,_sendPool(sendPool)
,_strServerPath(strServerPath)
,_uServerPort(uServerPort)
{
	_guid.create();

	//initialize daemon thread
	_daemodThrdPtr = new daemon_thread(&_logFile, _recvPool, _sendPool, _env._rtspCSeqSignal, _env._cvssEventSinkI._paramsList, _env._communicator, _env._adapter);
	_env.SetDaemonThrd(_daemodThrdPtr);
	//_daemodThrdPtr->initialize();

	//active adapter
	try
	{
		_env._adapter->ZQADAPTER_ADD(_env._communicator, this, ADAPTER_NAME_CVSS);
		//_env._adapter->activate();
	} catch (const Ice::Exception& e) {
		cerr << e << endl;
	} catch (const char* msg) {
		cerr << msg << endl;
	}

	// Initialize the CVSSEventSinkI class
	//::ZQTianShan::CVSS::CVSSEventSinkI eventSink(_logFile, iceStormEndpoint, _env._adapter);

	//initialize environment for CVSS evictor
	_env._adapter->findServantLocator(DBFILENAME_CVSSSession);
	if (evictoreSize <= 0)
		_env._eCvssStream->setSize(1000);
	else
		_env._eCvssStream->setSize(evictoreSize);
	Freeze::EvictorIteratorPtr evicIter = _env._eCvssStream->getIterator("", _env._eCvssStream->getSize());
	const ::Ice::Current c;
	while (evicIter && evicIter->hasNext())
	{
		::Ice::Identity _ident = evicIter->next();
		try
		{
			::TianShanIce::Streamer::CiscoVirtualStreamServer::CVStreamPrx streamPrx = CVSSIdentityToObjEnv(_env, CVStream, _ident);

			//use proxy to restore data
			CVSSRtspSession *sess = InitRTSPSession(streamPrx, _strServerPath, _uServerPort);

			_env._daemonThrd->addRTSPSession(sess, sess->_rtspSocket._socket);
		}
		catch  (::Ice::UnmarshalOutOfBoundsException &ex)
		{
			envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(CVSS, "catch UnmarshalOutOfBoundsException when restore group from db error(%s)"), _ident.name.c_str());
			_env._eCvssStream->remove(_ident);
		}
		catch(...)
		{
			envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(CVSS, "restore group from db error(%s)"), _ident.name.c_str());
			_env._eCvssStream->remove(_ident);
		}
	}

	_daemodThrdPtr->start();
}

CiscoVirtualStreamServiceImpl::
~CiscoVirtualStreamServiceImpl()
{
	delete _daemodThrdPtr;
}

::TianShanIce::Streamer::StreamPrx CiscoVirtualStreamServiceImpl::
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
		::Ice::Identity ident = pathTicket->getIdent();

		CVSSRtspSession *rtspClientSession = InitRTSPSession(pathTicket, _strServerPath, _uServerPort);

		//add session to group
		if (_env._daemonThrd->addRTSPSession(rtspClientSession, rtspClientSession->_rtspSocket._socket) == false)//create session fail
		{
			delete rtspClientSession;
			::ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, EXPFMT(CVSS, 306, "createSession(%s) fail to join session map"), ident.name.c_str());
		}

		CiscoVirtualStreamImplPtr stream = new CiscoVirtualStreamImpl(_env);
		if (!stream)
			return NULL;

		stream->ident.name = ident.name;
		//stream->ident.category = strCategory;
		stream->ident.category = DBFILENAME_CVSSSession;
		stream->pathTicketStr = _env._communicator->proxyToString(pathTicket);

		stream->setRtspClientSession(rtspClientSession);

		::TianShanIce::Streamer::StreamPrx streamProxy = CVSSIdentityToObjEnv(_env, CVStream, stream->ident);
		rtspClientSession->_strStreamName = _env._communicator->proxyToString(streamProxy);

		envlog(ZQ::common::Log::L_DEBUG,CLOGFMT(CVSS, "createSession() add the new Stream session[%s] into DB"), stream->ident.name.c_str());

		stream->sessKey = stream->ident.name;
		_env._eCvssStream->add(stream, stream->ident);
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(CVSS, "created a new Stream session[%s]"), stream->ident.name.c_str());

		//envlog(ZQ::common::Log::L_INFO, CLOGFMT(CVSS, "NGODStream session number %d"), _env._eNssStream->getSize());

		return streamProxy;
	}
	catch (Ice::Exception& ex)
	{
		//printf("name:%s\n", ex.ice_name().c_str());
		//ex.ice_print(cout);
		::ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, EXPFMT(CVSS, 306, "createSession() ice exception[%s]"), ex.ice_name().c_str());
	}
	catch (...)
	{
		::ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, EXPFMT(CVSS, 307, "createSession() unknown exception"));
		//cerr << "unknow exception" << endl;
	}

	//catch exception, return null
	return NULL;
}

::TianShanIce::Streamer::StreamerDescriptors
CiscoVirtualStreamServiceImpl::listStreamers(const ::Ice::Current& c)
throw (::TianShanIce::ServerError)
{
	::TianShanIce::Streamer::StreamerDescriptors _Descriptors;

	::TianShanIce::Streamer::StreamerDescriptor _Descriptor;
	char tmpGUid[128];

	memset(tmpGUid, 0 , 128);
	gethostname(tmpGUid, 127);
	_Descriptor.deviceId = tmpGUid;
	_Descriptor.type = DBFILENAME_CVSSSession;
	_Descriptors.push_back(_Descriptor);
	return _Descriptors;
}

::std::string CiscoVirtualStreamServiceImpl::
getNetId(const ::Ice::Current& c) const
throw (::TianShanIce::ServerError)
{
	return string(DBFILENAME_CVSSSession);
}

void CiscoVirtualStreamServiceImpl::
getStreamStat_async(const::TianShanIce::Streamer::CiscoVirtualStreamServer::AMD_CVStreamService_getStreamStatPtr& amdCB, 
					const ::TianShanIce::StrValues&, 
					const ::std::string&, 
					const ::Ice::Current& c)
{
	//TODO:
	::TianShanIce::Streamer::CiscoVirtualStreamServer::StreamStatCollection streamStateCollection;
	amdCB->ice_response(streamStateCollection);
}

::TianShanIce::Streamer::CiscoVirtualStreamServer::CVStreamPrx 
CiscoVirtualStreamServiceImpl::findStreamBySocket(::Ice::Long sockId, const ::Ice::Current& c)
{
	SOCKET sock = sockId;
	CVSSRtspSession *sess = _env._daemonThrd->findRTSPSession(sock);
	if (sess == NULL)
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(CVSS, "fail to find Session with Socket ID(%d)"), sockId);
		return NULL;
	}
	else
	{
		::std::stringstream ss;
		::std::string strSessKey;
		ss << sockId;
		strSessKey = ss.str();
		::std::vector<::Ice::Identity> identities = _env._idxSessionIdx->find(strSessKey);
		if (identities.size() > 0)
		{
			envlog(ZQ::common::Log::L_ERROR,CLOGFMT(CVSS, "find Sess evictor with Socket ID(%d)"), sess->_rtspSocket._socket);
			return CVSSIdentityToObjEnv(_env, CVStream, identities[0]);
		}
		else
		{
			envlog(ZQ::common::Log::L_ERROR,CLOGFMT(CVSS, "fail to find Sess evictor with Socket ID(%d)"), sess->_rtspSocket._socket);
			return NULL;
		}
	}
}

::TianShanIce::Streamer::CiscoVirtualStreamServer::CVStreamPrx 
CiscoVirtualStreamServiceImpl::findStreamBySession(const ::std::string& sessionId, const ::Ice::Current& c)
{
	CVSSRtspSession *sess = _env._daemonThrd->findRTSPSession(const_cast<::std::string &>(sessionId));
	if (sess == NULL)
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(CVSS, "fail to find Sess(%s)"), sessionId.c_str());
		return NULL;
	}
	else
	{
		::Ice::Identity tmpident = _env._communicator->stringToIdentity(sess->_strStreamName);
		//got stream
		if (tmpident.name.size() > 0)
		{
			envlog(ZQ::common::Log::L_ERROR,CLOGFMT(CVSS, "find Sess(%s) evictor with socket(%d)"), sessionId.c_str(), sess->_rtspSocket._socket);
			return CVSSIdentityToObjEnv(_env, CVStream, tmpident);
		}
		else
		{
			envlog(ZQ::common::Log::L_ERROR,CLOGFMT(CVSS, "fail to find Sess(%s) evictor with socket(%d)"), sessionId.c_str(), sess->_rtspSocket._socket);
			return NULL;
		}
	}
}

::std::string CiscoVirtualStreamServiceImpl::
getAdminUri(const ::Ice::Current& c)
{
	return string("");
}

::TianShanIce::State CiscoVirtualStreamServiceImpl::
getState(const ::Ice::Current& c)
{
	return ::TianShanIce::stNotProvisioned;
}

//don't need impl
void CiscoVirtualStreamServiceImpl::
queryReplicas_async(const ::TianShanIce::AMD_ReplicaQuery_queryReplicasPtr& amdReplicaQuery, const ::std::string& category, const ::std::string& groupId, bool Only, const ::Ice::Current& c)
{
	::TianShanIce::Replicas replicas;
	amdReplicaQuery->ice_response(replicas);
}

CVSSRtspSession* CiscoVirtualStreamServiceImpl::
InitRTSPSession(const ::TianShanIce::Transport::PathTicketPrx &_pathTicket, ::std::string &strServerIp, uint16 &iServerPort)
{
	if (_pathTicket == NULL)
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(CVSS, "fail to initialize stream session with NULL pathticket proxy"));
		return NULL;
	}

	CVSSRtspSession *rtspClientSession = new CVSSRtspSession();
	rtspClientSession->_commonReqHeader._strServerIp = strServerIp;
	rtspClientSession->_commonReqHeader._iServerPort = iServerPort;
	if (InitConnection(rtspClientSession) == false)
	{
		delete rtspClientSession;
		return NULL;
	}
	
	//::TianShanIce::Transport::StorageLinkPrx sLink = _pathTicket->getStorageLink();
	//rtspClientSession->m_RTSPR2Header.Volume.strName = sLink->getStorageInfo().netId;

	::TianShanIce::ValueMap pMap = _pathTicket->getPrivateData();
	::TianShanIce::SRM::ResourceMap resMap = _pathTicket->getResources();
	::std::string strTicket = _env._communicator->proxyToString(_pathTicket);

	SetupReqHeader &setupReqHeader = rtspClientSession->_setupReqHeader;

	TianShanIce::Variant	valBandwidth;
	TianShanIce::Variant	valDestAddr;
	TianShanIce::Variant	valDestPort;
	TianShanIce::Variant	valDestMac;

	try
	{
		//get destination IP
		valDestAddr = GetResourceMapData (resMap,TianShanIce::SRM::rtEthernetInterface,"destIP");
		if (valDestAddr.type != TianShanIce::vtStrings || valDestAddr.strs.size () == 0)
		{
			delete rtspClientSession;
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(envlog,EXPFMT("CVSS",1043,"invalid destAddress type should be vtString or no destAddress data with ticket [%s]"), strTicket.c_str());		
			return NULL;
		}
		else
		{
			setupReqHeader._strDestination = valDestAddr.strs[0];
			envlog(::ZQ::common::Log::L_INFO,EXPFMT("CVSS",873, "Get DestIP[%s] through ticket[%s]"),setupReqHeader._strDestination.c_str (),strTicket.c_str ());
		}

		//get destination port
		valDestPort = GetResourceMapData(resMap,TianShanIce::SRM::rtEthernetInterface,"destPort");
		if ( valDestPort.type != TianShanIce::vtInts || valDestPort.ints.size() == 0 )
		{
			delete rtspClientSession;
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(envlog,EXPFMT("CVSS",1044,"invalid dest por type,should be vtInts,or no destPort data with ticket [%s]"),strTicket.c_str());
			return NULL;
		}
		else
		{
			setupReqHeader._iClientPort = valDestPort.ints[0];
			envlog(::ZQ::common::Log::L_INFO,EXPFMT("CVSS",872,"Get destPort[%d] through ticket[%s]"),setupReqHeader._iClientPort,strTicket.c_str ());
		}
	}
	catch(...)
	{
		delete rtspClientSession;
		return NULL;
	}

	return rtspClientSession;
}

CVSSRtspSession *CiscoVirtualStreamServiceImpl::InitRTSPSession(::TianShanIce::Streamer::CiscoVirtualStreamServer::CVStreamPrx cvStreamPrx, ::std::string &strServerIp, uint16 &iServerPort)
{
	//get PathTicket Proxy
	::std::string pathTicketStr = cvStreamPrx->getPathTicketStr();
	::Ice::Identity tmpident = _env._communicator->stringToIdentity(pathTicketStr);
	::TianShanIce::Transport::PathTicketPrx& _pathTicketPrx = ::TianShanIce::Transport::PathTicketPrx::uncheckedCast(_env._adapter->createProxy(tmpident));

	CVSSRtspSession *rtspClientSession = InitRTSPSession(_pathTicketPrx, strServerIp, iServerPort);
	if (rtspClientSession == NULL)
		return NULL;
	
	return rtspClientSession;
}

bool CiscoVirtualStreamServiceImpl::InitConnection(CVSSRtspSession *sess)
{
	bool b = bConnection(sess->_commonReqHeader._strServerIp, sess->_commonReqHeader._iServerPort, sess->_rtspSocket._socket, 5);
	if (b)
	{
		sess->_rtspSocket._status = true;
		sess->_rtspSocket._smartBuffer._pBuffer = new char[iDefaultBufferSize];
		sess->_rtspSocket._smartBuffer._iBufferMaxSize = iDefaultBufferSize;
		return true;
	}
	else
	{
		envlog(ZQ::common::Log::L_ERROR,CLOGFMT(CVSS, "fail to connect to stream server(ip:%s, port:%d)"), sess->_commonReqHeader._strServerIp.c_str(), sess->_commonReqHeader._iServerPort);
		return false;
	}
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

		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("CVSS",1001,szBuf );
	}
	TianShanIce::ValueMap::const_iterator it=itResMap->second.resourceData.find(strkey);
	if(it==itResMap->second.resourceData.end())
	{
		char szBuf[1024];
		sprintf(szBuf,"GetResourceMapData() value with key=%s not found",strkey.c_str());
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>("CVSS",1002,szBuf);
	}
	return it->second;
}
}//CVSS
}//ZQTianShan
