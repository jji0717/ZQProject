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

#include "NSSCfgLoader.h"
//extern ZQ::common::Config::Loader< ::ZQTianShan::NSS::NSSCfg > pConfig;
extern ::ZQTianShan::NSS::NSSBaseConfig::NSSHolder *pNSSBaseConfig;

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

//add for CCUR
void NGODStreamImpl::calcScale(::Ice::Float &fScale)
{
	if (pNSSBaseConfig->_videoServer.vendor.compare("CCUR") == 0 || pNSSBaseConfig->_videoServer.FixedSpeedSetEnable)
	{
		bool bHasStatus = false;
		bool bDirection = true;// true forward , false backward
		if( forwardBase > 0.0f && (fScale - 1.0f > 0.0001f))
		{
			bDirection = true;			
			bHasStatus = true;
		}
		else if (backwardBase < 0.0f && fScale < 0.0f)
		{
			bDirection = false;
			bHasStatus = true;
		}
		//check last speed or status
		if(  ( fScale - 1.0f ) > 0.0001f )
		{//fast forward
			int iFastforwardSetSize = static_cast<int>(pNSSBaseConfig->_videoServer.FixedSpeedSetForwardSet.size());
			if(iFastforwardSetSize > 0 )
			{
				if( !bHasStatus )
				{
					speedIdx = 0;
				}
				else
				{
					if( !bDirection)
					{
						speedIdx = 0;
					}
					else
					{
						speedIdx = (++speedIdx)% iFastforwardSetSize ;
					}
				}
				if (speedIdx == 0)
					fScale = (float)pNSSBaseConfig->_videoServer.FixedSpeedSetForwardSet[ speedIdx];
				else
					fScale = pNSSBaseConfig->_videoServer.FixedSpeedSetForwardSet[ speedIdx] * forwardBase;
			}
			envlog(::ZQ::common::Log::L_INFO,CLOGFMT(NGODStreamImpl,"fastforward , set speed(idx:%d) to [%f=%f*%f]"),speedIdx, fScale, pNSSBaseConfig->_videoServer.FixedSpeedSetForwardSet[ speedIdx], forwardBase);
		}
		else if( fScale < 0.0f )
		{//fast rewind
			int iFastRewindSetSize =  static_cast<int>(pNSSBaseConfig->_videoServer.FixedSpeedSetBackwardSet.size());
			//envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(NGODStreamImpl,"fastrewind(fscale:%f) , sFastRewindSetSize:%d"), fScale, iFastRewindSetSize);
			if( iFastRewindSetSize > 0 )
			{
				//speedIdx = -speedIdx;
				if( !bHasStatus  )
				{
					speedIdx = 0;
					//fScale = pConfig._fixedSpeedSet.backwardSet[speedIdx] * backwardBase;
				}
				else
				{
					if(bDirection)
					{
						//envlog(::ZQ::common::Log::L_INFO,CLOGFMT(NGODStreamImpl,"from fastforward to fastrewind"));
						speedIdx = 0;
					}
					else
					{
						speedIdx = (++speedIdx)% iFastRewindSetSize ;
						//envlog(::ZQ::common::Log::L_INFO,CLOGFMT(NGODStreamImpl,"from fastrewind to fastrewind, set speedIdx to %d"), speedIdx);
					}
					//fScale = pConfig._fixedSpeedSet.backwardSet[speedIdx] * backwardBase;
				}
				if (speedIdx == 0)
					fScale = pNSSBaseConfig->_videoServer.FixedSpeedSetBackwardSet[ speedIdx];
				else
					fScale = pNSSBaseConfig->_videoServer.FixedSpeedSetBackwardSet[ speedIdx] * (-backwardBase);
				//speedIdx = -speedIdx;
			}
			envlog(::ZQ::common::Log::L_INFO,CLOGFMT(NGODStreamImpl,"fastrewind , set speed(idx:%d) to [%f=%f*(%f)]"),speedIdx, fScale, pNSSBaseConfig->_videoServer.FixedSpeedSetBackwardSet[ speedIdx], backwardBase);
		}
		else
		{
			forwardBase = 0;
			backwardBase = 0;
			envlog(::ZQ::common::Log::L_INFO,CLOGFMT(NGODStreamImpl,"normal status , set speed to [%f]"),fScale);
		}
	}
}


void NGODStreamImpl::renewPathTicket(const ::Ice::Current& c)
{
	int32 iTime = GetTickCount();
	try
	{
		::TianShanIce::Transport::PathTicketPrx& _pathTicketPrx = ::TianShanIce::Transport::PathTicketPrx::uncheckedCast(_env._communicator->stringToProxy(pathTicketStr));
		if(_pathTicketPrx!=NULL)
		{
			_pathTicketPrx->renew(SessionSyncTime);//renew path ticket
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
setItemInfo(::TianShanIce::Streamer::NGODStreamServer::stItem &item, ::Ice::Int userCtrlNum, const ::TianShanIce::Streamer::PlaylistItemSetupInfo& newItemInfo, ::Ice::Int whereUserCtrlNum)
{
	//record item info
	item.criticalStart		= (time_t)newItemInfo.criticalStart;
	item.userCtrlNum		= userCtrlNum;
	item.forceNormal		= newItemInfo.forceNormal;
	item.inTimeOffset		= (uint32)newItemInfo.inTimeOffset;
	item.outTimeOffset		= (uint32)newItemInfo.outTimeOffset;
	item.spliceIn			= newItemInfo.spliceIn;
	item.spliceOut			= newItemInfo.spliceOut;
	item.whereUserCtrlNum	= whereUserCtrlNum;
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
		tmpItem.userCtrlNum = (*pIter).iCtrlNum;
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

::std::string NGODStreamImpl::getC1ConnectionUrl(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return c1ConnectionUrl;
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

void NGODStreamImpl::play_async(const ::TianShanIce::Streamer::AMD_Stream_playPtr& amd, const ::Ice::Current& c)
{
	bool b = false;
	try
	{
		b = play(c);
		amd->ice_response(b);
	}
	catch (::TianShanIce::ServerError &ex)
	{
		amd->ice_exception(ex);
		return;
	}
	catch (...)
	{
		::std::string lastError = "unknown exception";
		::TianShanIce::ServerError ex("NGODStreamImpl", 180, lastError);
		amd->ice_exception(ex);
		return;
	}
}

bool NGODStreamImpl::play(const ::Ice::Current& c)
throw (::TianShanIce::ServerError, 
	   ::TianShanIce::InvalidStateOfArt)
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
		calcScale(_rtspClientSession->fScale);
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

void NGODStreamImpl::seekStream_async(const ::TianShanIce::Streamer::AMD_Stream_seekStreamPtr& amd, ::Ice::Long offset, ::Ice::Int startPos, const ::Ice::Current& c)
{
	::Ice::Long i = 0;
	try
	{
		i = seekStream(offset, startPos, c);
		amd->ice_response(i);
	}
	catch (::TianShanIce::ServerError &ex)
	{
		amd->ice_exception(ex);
		return;
	}
	catch (...)
	{
		::std::string lastError = "unknown exception";
		::TianShanIce::ServerError ex("NGODStreamImpl", 180, lastError);
		amd->ice_exception(ex);
		return;
	}
}

::Ice::Long NGODStreamImpl::
seekStream(::Ice::Long offset,
		   ::Ice::Int startPos,
		   const ::Ice::Current& c)
	throw (::TianShanIce::ServerError,
		   ::TianShanIce::InvalidParameter)
{
	if (startPos == 0)
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"NGODStreamImpl",180,"NGODStreamImpl::session(%s) seekStream() seek from current position don't supported", ident.name.c_str());
	if (startPos == 1 && offset < 0)
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"NGODStreamImpl",180,"NGODStreamImpl::session(%s) seekStream() seek from begin but offset(%d) is negative", ident.name.c_str(), offset);
	if (startPos == 2 && offset > 0)
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"NGODStreamImpl",180,"NGODStreamImpl::session(%s) seekStream() seek from end but offset(%d) is positive", ident.name.c_str(), offset);

	RTSPClientSession *_rtspClientSession = _env._daemonThrd->findSessionByOnDemandSessionId(ident.name);
	if (_rtspClientSession != NULL)
	{
		_rtspClientSession->strCurrentTimePoint.clear();
		bool b = false;
		vecGETPARAMETER_EXT headerList;
		//for convenient use
		GetPramameterRes_ExtHeader &sessState = _rtspClientSession->m_RTSPR2Header.m_GetPramameterRes_ExtHeader;

		headerList.clear();
		headerList.push_back(scale);

		DWORD iTime = GetTickCount();
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "playEx: session(%s) begin send GETPARAMETER"), ident.name.c_str());
		b = ngod_rtsp_action::GetParameterActionBySessionId(ident.name, *_nssSessionGroupList, _pool, _logFile, headerList);
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "playEx: session(%s) send GETPARAMETER over, cost %dms"), ident.name.c_str(), GetTickCount() - iTime);
		if (b)
		{
			::TianShanIce::StrValues expectedProps;
			::TianShanIce::Streamer::StreamInfo streamInfo = playEx(atof(_rtspClientSession->m_RTSPR2Header.m_GetPramameterRes_ExtHeader.strScale.c_str()), offset, startPos, expectedProps, c);
			return atoi(streamInfo.props["CURRENTPOS"].c_str());
		}			
	}

	ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"NGODStreamImpl",180,"NGODStreamImpl::session(%s) seekStream() fail to perform seekStream", ident.name.c_str());
	return 0;
}

void NGODStreamImpl::playEx_async(const ::TianShanIce::Streamer::AMD_Stream_playExPtr& amd, ::Ice::Float newSpeed, ::Ice::Long timeOffset, ::Ice::Short from, const ::TianShanIce::StrValues& expectedProps, const ::Ice::Current& c)
{
	::TianShanIce::Streamer::StreamInfo streamInfo;
	try
	{
		streamInfo = playEx(newSpeed, timeOffset, from, expectedProps, c);
		amd->ice_response(streamInfo);
	}
	catch (::TianShanIce::ServerError &ex)
	{
		amd->ice_exception(ex);
		return;
	}
	catch (...)
	{
		::std::string lastError = "unknown exception";
		::TianShanIce::ServerError ex("NGODStreamImpl", 180, lastError);
		amd->ice_exception(ex);
		return;
	}
}

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
		calcScale(_rtspClientSession->fScale);

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
			if (forwardBase == 0 && _rtspClientSession->fScale - 1.0 > 0.0f)
				forwardBase = _rtspClientSession->fScale;
			else if (backwardBase == 0 && _rtspClientSession->fScale < 0.0f)
				backwardBase = _rtspClientSession->fScale;
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
	//headerList.push_back(presentation_state);
	headerList.push_back(position);
	//headerList.push_back(scale);

	bool b = false;

	DWORD iTime = GetTickCount();
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "getInfo: session(%s) begin send GETPARAMETER"), ident.name.c_str());
	b = ngod_rtsp_action::GetParameterActionBySessionId(ident.name, *_nssSessionGroupList, _pool, _logFile, headerList);
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "getInfo: session(%s) send GETPARAMETER over, cost %dms"), ident.name.c_str(), GetTickCount() - iTime);
	
	if (!b)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(NGODStreamImpl, "getInfo: session(%s) fail to get parameter"), ident.name.c_str());
		return false;
	}
	else
	{
		RTSPClientSession *_rtspClientSession = _env._daemonThrd->findSessionByOnDemandSessionId(ident.name);
		if (NULL == _rtspClientSession)
		{
			envlog(::ZQ::common::Log::L_WARNING,CLOGFMT(NGODStreamImpl,"guid[%s]enter get info fail, could not find session"), ident.name.c_str());

			return false;
		}

		TianShanIce::Variant res;
		res.floats.clear();
		res.bRange = false;
		::std::string strPosition = _rtspClientSession->m_RTSPR2Header.m_GetPramameterRes_ExtHeader.strPosition;
		int iTime = 0;
		if (strPosition.find(".") != ::std::string::npos)//npt range format
			iTime = atof(strPosition.c_str());			
		else
		{
			int msTime = 0;;
			sscanf(strPosition.c_str(), "%x", &msTime);
			iTime = msTime/1000;
		}
		res.ints.push_back(iTime);
		envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(NGODStreamImpl,"guid[%s]enter get info, position[%s]=%d"), ident.name.c_str(), strPosition.c_str(), iTime);
		res.type=TianShanIce::vtInts;
		varOut["playposition"]=res;		

		/*res.ints.clear();
		res.bRange = false;
		res.strs.push_back(_rtspClientSession->m_RTSPR2Header.m_GetPramameterRes_ExtHeader.strPosition);
		res.type=TianShanIce::vtInts;
		varOut["totalplaytime"]=res;*/

		//res.bRange = false;
		//res.strs.clear();
		//res.strs.push_back(_rtspClientSession->m_RTSPR2Header.m_GetPramameterRes_ExtHeader.strScale);
		//res.type=TianShanIce::vtStrings;
		//varOut["scale"]=res;
	}

	headerList.clear();
	headerList.push_back(scale);
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "getInfo: session(%s) begin send GETPARAMETER"), ident.name.c_str());
	b = ngod_rtsp_action::GetParameterActionBySessionId(ident.name, *_nssSessionGroupList, _pool, _logFile, headerList);
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "getInfo: session(%s) send GETPARAMETER over, cost %dms"), ident.name.c_str(), GetTickCount() - iTime);

	if (!b)
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(NGODStreamImpl, "getInfo: session(%s) fail to get parameter"), ident.name.c_str());
		return false;
	}
	else
	{
		RTSPClientSession *_rtspClientSession = _env._daemonThrd->findSessionByOnDemandSessionId(ident.name);
		if (NULL == _rtspClientSession)
		{
			envlog(::ZQ::common::Log::L_WARNING,CLOGFMT(NGODStreamImpl,"guid[%s]enter get info fail, could not find session"), ident.name.c_str());

			return false;
		}

		TianShanIce::Variant res;
		//res.ints.clear();
		//res.bRange = false;
		//res.ints.push_back(atoi(_rtspClientSession->m_RTSPR2Header.m_GetPramameterRes_ExtHeader.strPosition.c_str()));
		//res.type=TianShanIce::vtInts;
		//varOut["playposition"]=res;		

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
	//for get PAID and PID from content name
	::std::string strAE_UDI = newItemInfo.contentName;
	::std::string parsedPAID = "";
	::std::string parsedPID = "";
	size_t index = strAE_UDI.find("_");
	if(index != std::string::npos)
	{
		parsedPAID	= strAE_UDI.substr(0,index);
		parsedPID	= strAE_UDI.substr(index+1);
		envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(NGODStreamImpl,"pushBack: session(%s) parse PAID(%s)/PID(%s) from contentName(%s)"), ident.name.c_str(), parsedPAID.c_str(), parsedPID.c_str(), strAE_UDI.c_str());
	}
	else
	    envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(NGODStreamImpl,"pushBack: session(%s) parse PAID/PID from contentName(%s) error"), ident.name.c_str(), parsedPAID.c_str(), parsedPID.c_str(), strAE_UDI.c_str());
	///////////////////////////
	if ((mapIter = pMap.find("PAID")) != pMap.end())
	{
		//get client_id
		if ((*mapIter).second.type != ::TianShanIce::vtStrings)
			pSDP.strAsset_id = parsedPAID;
		else			
			pSDP.strAsset_id = (*mapIter).second.strs[0];
		envlog(::ZQ::common::Log::L_WARNING,CLOGFMT(NGODStreamImpl,"insert: session(%s) get PAID %s"), ident.name.c_str(), pSDP.strAsset_id.c_str());
	}
	else
		pSDP.strAsset_id = parsedPAID;
	if ((mapIter = pMap.find("PID")) != pMap.end())
	{
		//get client_id
		if ((*mapIter).second.type != ::TianShanIce::vtStrings)
			pSDP.strProvider_id = parsedPID;
		else
			pSDP.strProvider_id = (*mapIter).second.strs[0];
		envlog(::ZQ::common::Log::L_WARNING,CLOGFMT(NGODStreamImpl,"insert: session(%s) get PID %s"), ident.name.c_str(), pSDP.strProvider_id.c_str());
	}
	else
		pSDP.strProvider_id = parsedPID;

	WLock sync(*this);
	//IceUtil::Mutex::Lock lock(*this);
	::TianShanIce::Streamer::NGODStreamServer::stItem tmpItem;
	tmpItem.assetId = pSDP.strAsset_id;
	tmpItem.providerId = pSDP.strProvider_id;

	setItemInfo(tmpItem, userCtrlNum, newItemInfo, whereUserCtrlNum);

	::std::stringstream sIn;
	::std::stringstream sOut;
	sIn << tmpItem.inTimeOffset;
	sOut << tmpItem.outTimeOffset;
	if (tmpItem.outTimeOffset > 0)
		pSDP.strRange = sIn.str() + "-" + sOut.str();
	else
		pSDP.strRange = sIn.str();

	SDPRequestContentVec *pSDPList = &(_rtspClientSession->m_RTSPR2Header.m_SDPRequestContent);
	SDPRequestContentVec::iterator listIter = find_if(pSDPList->begin(), pSDPList->end(), FindByCtlrNum(whereUserCtrlNum));
	if (listIter != pSDPList->end())
		//insert before
		pSDPList->insert(listIter, pSDP);
	else
		pSDPList->push_back(pSDP);

	if (assetId.empty())
		assetId.push_back(tmpItem);
	else
	{
		for (::TianShanIce::Streamer::NGODStreamServer::ItemList::iterator iter = assetId.begin();
			iter != assetId.end(); iter++)
		{
			if ( whereUserCtrlNum = (*iter).whereUserCtrlNum)
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

	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(NSS, "NGODStreamImpl::pushBack() successfully find session %s contentName(%s)") , ident.name.c_str(), newItemInfo.contentName.c_str());

	const ::TianShanIce::ValueMap &pMap = newItemInfo.privateData;

	::TianShanIce::ValueMap::const_iterator mapIter;
	SDPRequestContent pSDP;
	
	pSDP.iCtrlNum = userCtrlNum;
	//for get PAID and PID from content name
	::std::string strAE_UDI = newItemInfo.contentName;
	::std::string parsedPAID = "";
	::std::string parsedPID = "";
	size_t index = strAE_UDI.find("_");
	if(index != std::string::npos)
	{
		parsedPAID	= strAE_UDI.substr(0,index);
		parsedPID	= strAE_UDI.substr(index+1);
		envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(NGODStreamImpl,"pushBack: session(%s) parse PAID(%s)/PID(%s) from contentName"), ident.name.c_str(), parsedPAID.c_str(), parsedPID.c_str());
	}
	else
		envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(NGODStreamImpl,"pushBack: session(%s) parse PAID/PID from contentName(%s) error"), ident.name.c_str(), parsedPAID.c_str(), parsedPID.c_str(), strAE_UDI.c_str());
	///////////////////////////
	if ((mapIter = pMap.find("PAID")) != pMap.end())
	{
		//get asset_id
		if ((*mapIter).second.type != ::TianShanIce::vtStrings)
		{
			pSDP.strAsset_id = parsedPAID;
			envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(NGODStreamImpl,"pushBack: session(%s) PAID type isn't string"), ident.name.c_str());
		}
		else
		{
			pSDP.strAsset_id = (*mapIter).second.strs[0];
			envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(NGODStreamImpl,"pushBack: session(%s) get PAID %s "), ident.name.c_str(), pSDP.strAsset_id.c_str());
		}
	}
	else
		pSDP.strAsset_id = parsedPAID;
	if ((mapIter = pMap.find("PID")) != pMap.end())
	{
		//get provider_id
		if ((*mapIter).second.type != ::TianShanIce::vtStrings)
		{
			pSDP.strProvider_id = parsedPID;
			envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(NGODStreamImpl,"pushBack: session(%s) PID type isn't string"), ident.name.c_str());
		}
		else
			pSDP.strProvider_id = (*mapIter).second.strs[0];
		envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(NGODStreamImpl,"pushBack: session(%s) get PID %s"), ident.name.c_str(), pSDP.strProvider_id.c_str());
	}
	else
		pSDP.strProvider_id = parsedPID;

	//IceUtil::Mutex::Lock lock(*this);
	::TianShanIce::Streamer::NGODStreamServer::stItem tmpItem;
	tmpItem.assetId = pSDP.strAsset_id;
	tmpItem.providerId = pSDP.strProvider_id;

	setItemInfo(tmpItem, userCtrlNum, newItemInfo, assetId.size());
	assetId.push_back(tmpItem);

	::std::stringstream sIn;
	::std::stringstream sOut;
	sIn << tmpItem.inTimeOffset;
	sOut << tmpItem.outTimeOffset;
	if (tmpItem.outTimeOffset > 0)
		pSDP.strRange = sIn.str() + "-" + sOut.str();
	else
		pSDP.strRange = sIn.str() + "-";
	_rtspClientSession->m_RTSPR2Header.m_SDPRequestContent.push_back(pSDP);

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
	//try to get current play npt and change to item play time point
	vecGETPARAMETER_EXT headerList;
	headerList.push_back(position);

	bool b = false;

	DWORD iTime = GetTickCount();
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "left: session(%s) begin send GETPARAMETER"), ident.name.c_str());
	b = ngod_rtsp_action::GetParameterActionBySessionId(const_cast<::std::string &>(ident.name), *const_cast<NSSSessionGroupList *>(_nssSessionGroupList), const_cast<::ZQ::common::NativeThreadPool &>(_pool), const_cast<::ZQ::common::FileLog &>(_logFile), headerList);
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "left: session(%s) send GETPARAMETER over, cost %dms"), ident.name.c_str(), GetTickCount() - iTime);
	if (b)
	{
		RTSPClientSession *_rtspClientSession = _env._daemonThrd->findSessionByOnDemandSessionId(ident.name);
		int iTime = 0;
		for (::TianShanIce::Streamer::NGODStreamServer::ItemList::const_iterator iter = assetId.begin(); iter != assetId.end(); iter++)
		{
			iTime += iter->outTimeOffset - iter->inTimeOffset;
			if (iTime > atoi(_rtspClientSession->strCurrentTimePoint.c_str()))
				break;
		}
		iTime = iTime - atoi(_rtspClientSession->strCurrentTimePoint.c_str());
		return iTime;
	}
	else
		return -1;
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
	//try to get current play npt and change to item play time point
	vecGETPARAMETER_EXT headerList;
	headerList.push_back(position);

	bool b = false;

	DWORD iTime = GetTickCount();
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "current: session(%s) begin send GETPARAMETER"), ident.name.c_str());
	b = ngod_rtsp_action::GetParameterActionBySessionId(const_cast<::std::string &>(ident.name), *const_cast<NSSSessionGroupList *>(_nssSessionGroupList), const_cast<::ZQ::common::NativeThreadPool &>(_pool), const_cast<::ZQ::common::FileLog &>(_logFile), headerList);
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "current: session(%s) send GETPARAMETER over, cost %dms"), ident.name.c_str(), GetTickCount() - iTime);
	if (b)
	{
		RTSPClientSession *_rtspClientSession = _env._daemonThrd->findSessionByOnDemandSessionId(ident.name);
		int iTime = 0;
		for (::TianShanIce::Streamer::NGODStreamServer::ItemList::const_iterator iter = assetId.begin(); iter != assetId.end(); iter++)
		{
			iTime += iter->outTimeOffset - iter->inTimeOffset;
			if (iTime > atoi(_rtspClientSession->strCurrentTimePoint.c_str()))
				return iter->userCtrlNum;
		}
		return -1;
	}
	else
		return -1;
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
		if ( whereUserCtrlNum = (*iter).whereUserCtrlNum)
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
		tmpValues.push_back((*iter).userCtrlNum);
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
		if ((*iter).userCtrlNum == userCtrlNum)
			return (*iter).userCtrlNum;
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

void NGODStreamImpl::skipToItem_async( const ::TianShanIce::Streamer::AMD_Playlist_skipToItemPtr& amd,::Ice::Int where, bool bPlay, const ::Ice::Current& c)
{
	bool b = false;
	try
	{
		b = skipToItem(where, bPlay, c);
		amd->ice_response(b);
	}
	catch (::TianShanIce::ServerError &ex)
	{
		amd->ice_exception(ex);
		return;
	}
	catch (...)
	{
		::std::string lastError = "unknown exception";
		::TianShanIce::ServerError ex("NGODStreamImpl", 180, lastError);
		amd->ice_exception(ex);
		return;
	}
}

bool NGODStreamImpl::
skipToItem(::Ice::Int where, 
		   bool bPlay,
		   const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter, 
		   ::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
	//try to get current play npt and change to item play time point
	vecGETPARAMETER_EXT headerList;
	headerList.push_back(position);

	bool b = false;

	DWORD iTime = GetTickCount();
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "skipToItem: session(%s) begin send GETPARAMETER"), ident.name.c_str());
	b = ngod_rtsp_action::GetParameterActionBySessionId(const_cast<::std::string &>(ident.name), *const_cast<NSSSessionGroupList *>(_nssSessionGroupList), const_cast<::ZQ::common::NativeThreadPool &>(_pool), const_cast<::ZQ::common::FileLog &>(_logFile), headerList);
	envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl, "skipToItem: session(%s) send GETPARAMETER over, cost %dms"), ident.name.c_str(), GetTickCount() - iTime);
	if (b)
	{
		RTSPClientSession *_rtspClientSession = _env._daemonThrd->findSessionByOnDemandSessionId(ident.name);
		int iTime = 0;
		for (::TianShanIce::Streamer::NGODStreamServer::ItemList::const_iterator iter = assetId.begin(); iter != assetId.end(); iter++)
		{
			if (iter->userCtrlNum == where)
			{
				::std::stringstream ss;
				ss << iTime;
				_rtspClientSession->strCurrentTimePoint = ss.str();
				return play(c);
			}
			iTime += iter->outTimeOffset - iter->inTimeOffset;
		}
		return false;
	}
	else
		return false;
}

void NGODStreamImpl::seekToPosition_async(const ::TianShanIce::Streamer::AMD_Playlist_seekToPositionPtr& amd,
								  ::Ice::Int UserCtrlNum, 
								  ::Ice::Int timeOffset, 
								  ::Ice::Int startPos, 
								  const ::Ice::Current& c)
{
	bool b = false;
	try
	{
		b = seekToPosition(UserCtrlNum, timeOffset, startPos, c);
		amd->ice_response(b);
	}
	catch (::TianShanIce::ServerError &ex)
	{
		amd->ice_exception(ex);
		return;
	}
	catch (...)
	{
		::std::string lastError = "unknown exception";
		::TianShanIce::ServerError ex("NGODStreamImpl", 180, lastError);
		amd->ice_exception(ex);
		return;
	}
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
	if (startPos == 0)
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"NGODStreamImpl",180,"NGODStreamImpl::session(%s) seekToPosition() seek from current position don't supported", ident.name.c_str());
	if (startPos == 1 && timeOffset < 0)
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"NGODStreamImpl",180,"NGODStreamImpl::session(%s) seekToPosition() seek from begin but offset(%d) is negative", ident.name.c_str(), timeOffset);
	if (startPos == 2 && timeOffset > 0)
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"NGODStreamImpl",180,"NGODStreamImpl::session(%s) seekToPosition() seek from end but offset(%d) is positive", ident.name.c_str(), timeOffset);

	RTSPClientSession *_rtspClientSession = _env._daemonThrd->findSessionByOnDemandSessionId(ident.name);
	int iTime = 0;
	for (::TianShanIce::Streamer::NGODStreamServer::ItemList::const_iterator iter = assetId.begin(); iter != assetId.end(); iter++)
	{
		if (iter->userCtrlNum == UserCtrlNum)
		{
			if (startPos == 2)
				iTime = iTime + (iter->outTimeOffset - iter->inTimeOffset);
			iTime += timeOffset;
			::std::stringstream ss;
			ss << iTime;
			_rtspClientSession->strCurrentTimePoint = ss.str();
			return play(c);
		}
		iTime += iter->outTimeOffset - iter->inTimeOffset;
	}
	ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"NGODStreamImpl",180,"NGODStreamImpl::session(%s) seekToPosition() the given userCtrlNum(%d) is invalid", ident.name.c_str(), UserCtrlNum);
	return false;
}
   
void NGODStreamImpl::
enableEoT(bool enable,
		  const ::Ice::Current& c)
	throw (::TianShanIce::ServerError,
		   ::TianShanIce::InvalidStateOfArt)
{
}

void NGODStreamImpl::playItem_async(const ::TianShanIce::Streamer::AMD_Playlist_playItemPtr& amd, ::Ice::Int UserCtrlNum, ::Ice::Int timeOffset, ::Ice::Short from, ::Ice::Float newSpeed, const ::TianShanIce::StrValues& expectedProps, const ::Ice::Current& c)
{
	::TianShanIce::Streamer::StreamInfo streamInfo;
	try
	{
		streamInfo = playItem(UserCtrlNum, timeOffset, from, newSpeed, expectedProps, c);
		amd->ice_response(streamInfo);
	}
	catch (::TianShanIce::ServerError &ex)
	{
		amd->ice_exception(ex);
		return;
	}
	catch (...)
	{
		::std::string lastError = "unknown exception";
		::TianShanIce::ServerError ex("NGODStreamImpl", 180, lastError);
		amd->ice_exception(ex);
		return;
	}
}

::TianShanIce::Streamer::StreamInfo NGODStreamImpl::
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
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"NGODStreamImpl",180,"NGODStreamImpl::session(%s) playItem() seek from current position don't supported", ident.name.c_str());
	if (startPos == 1 && timeOffset < 0)
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"NGODStreamImpl",180,"NGODStreamImpl::session(%s) playItem() seek from begin but offset(%d) is negative", ident.name.c_str(), timeOffset);
	if (startPos == 2 && timeOffset > 0)
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"NGODStreamImpl",180,"NGODStreamImpl::session(%s) playItem() seek from end but offset(%d) is positive", ident.name.c_str(), timeOffset);

	RTSPClientSession *_rtspClientSession = _env._daemonThrd->findSessionByOnDemandSessionId(ident.name);
	int iTime = 0;
	for (::TianShanIce::Streamer::NGODStreamServer::ItemList::const_iterator iter = assetId.begin(); iter != assetId.end(); iter++)
	{
		if (iter->userCtrlNum == UserCtrlNum)
		{
			if (startPos == 2)
				iTime = iTime + (iter->outTimeOffset - iter->inTimeOffset);
			iTime += timeOffset;
			return playEx(newSpeed, iTime, 0, expectedProps, c);
		}
		iTime += iter->outTimeOffset - iter->inTimeOffset;
	}
	ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"NGODStreamImpl",180,"NGODStreamImpl::session(%s) playItem() the given userCtrlNum(%d) is invalid", ident.name.c_str(), UserCtrlNum);
	::TianShanIce::Streamer::StreamInfo streamInfo;
	return streamInfo;
}

::TianShanIce::Properties NGODStreamImpl::getProperties(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	return props;
}

void NGODStreamImpl::setProperties(const ::TianShanIce::Properties& prop, const ::Ice::Current& c)
{
	WLock wLock(*this);
	props = prop;
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
					  SessionGroup::Groups &sessionGroups,
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
	
	SessionGroup::Groups &groups = sessionGroups;
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
			if (pNSSBaseConfig->_videoServer.vendor.compare("CCUR") == 0)
				pSess->_serverType = CCUR;
			else
				pSess->_serverType = NGOD;

			//re-setup for C1 connection
			pSess->strStreamName = _env._communicator->proxyToString(streamPrx);
			::std::string strC1ConnectUrl = streamPrx->getC1ConnectionUrl();
			pSess->m_streamCtrlURL.parse(strC1ConnectUrl.c_str());
			pSess->m_RTSPR2Header.m_SDPResponseContent.strHost = _strServerPath;
			pSess->m_RTSPR2Header.m_SDPResponseContent.uPort = _uServerPort;
			pSess->m_RTSPR2Header.m_SDPResponseContent.strProtocol = "rtsp";
			pSess->m_RTSPR2Header.m_SDPResponseContent.strStreamhandle = strRtspSessionId;
			int iPort = pSess->m_streamCtrlURL.getPort();
			if (_env._daemonThrd->addSessionGroupConnection(strSessionGroup, pSess->m_streamCtrlURL.generate(), pSess->m_streamCtrlURL.getHost(), iPort, pSess->RTSPC1Socket))
			{
				envlog(ZQ::common::Log::L_INFO, CLOGFMT(NGODStreamImpl,"NGODStreamServiceImpl() successfully re-setup sesion(%s) C1 connection(%08x) with video server"), strOnDemandSessId.c_str(), *(pSess->RTSPC1Socket));
			}
			else
			{
				envlog(ZQ::common::Log::L_WARNING, CLOGFMT(NGODStreamImpl,"NGODStreamServiceImpl() fail to re-setup sesion(%s) C1 connection with video server"), strOnDemandSessId.c_str());
			}

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
				pSDP.strProvider_id = (*pIter).providerId;
				::std::stringstream sIn;
				::std::stringstream sOut;
				sIn << (*pIter).inTimeOffset;
				sOut << (*pIter).outTimeOffset;
				pSDP.strRange = sIn.str() + "-" + sOut.str();
				pSDP.strAsset_id = (*pIter).assetId;
				pSDP.iCtrlNum = (*pIter).userCtrlNum;
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

	for (NSSSessionGroupList::iterator iter = _nssSessionGroupList.begin(); iter != _nssSessionGroupList.end(); iter++)
	{
		for (SessionGroupSocketMap::iterator sockIter = (*iter)->m_SessionGroupSocketMap.begin(); sockIter != (*iter)->m_SessionGroupSocketMap.end(); sockIter++)
		{
			CloseSocket(sockIter->second->_sessSock.m_Socket);
			delete sockIter->second;
		}
		(*iter)->m_SessionGroupSocketMap.clear();
	}
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
		if (rtspClientSession == NULL)
		{
			::ZQTianShan::_IceThrow<TianShanIce::ServerError>(envlog, EXPFMT(NSS, 306, "createSession(%s) fail create session because of error format pathticket"), ident.name.c_str());
			return NULL;
		}

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

		stream->speedIdx = -1;
		stream->forwardBase = 0;
		stream->backwardBase = 0;

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
	return string("NSS_") + pNSSBaseConfig->netId;
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
	return ::TianShanIce::stInService;
}

void NGODStreamServiceImpl::
queryReplicas_async(const ::TianShanIce::AMD_ReplicaQuery_queryReplicasPtr& amdReplicaQuery, const ::std::string& category, const ::std::string& groupId, bool Only, const ::Ice::Current& c)
{
	::TianShanIce::Replicas replicas;
	amdReplicaQuery->ice_response(replicas);
}

//don't need impl

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
	pGroup->_groupUrlStr.setProtocol("rtsp");
	pGroup->_groupUrlStr.setHost(_strServerPath.c_str());
	pGroup->_groupUrlStr.setPort(_uServerPort);
	InitializeCriticalSection(&pGroup->m_CS_SessionMap);
	InitializeCriticalSection(&pGroup->m_CS);
	InitializeCriticalSection(&pGroup->m_CS_ClientSeq);
	InitializeCriticalSection(&pGroup->m_CS_ServerSeq);
	if (bConnection(strServerPath, uServerPort, pGroup->m_SessionSocket.m_Socket, 1))
	{
		{
			SessionGroupSocket *pSessionGrouopSocket = new SessionGroupSocket();
			pSessionGrouopSocket->_iServerPort = _uServerPort;
			pSessionGrouopSocket->_strServerIp = strServerPath;
			pSessionGrouopSocket->_sessSock.m_Socket = pGroup->m_SessionSocket.m_Socket;
			pSessionGrouopSocket->_sessSock.m_Status = true;

			::ZQ::common::MutexGuard guard(pGroup->m_SessionGroupSocketMapMutex);
			const char *key = pGroup->_groupUrlStr.generate();
			pGroup->m_SessionGroupSocketMap[std::string(key)] = pSessionGrouopSocket;
		}
		//_env._daemonThrd->addSessionGroupConnection(_strServerPath, pGroup->_groupUrlStr.generate(), pGroup->_groupUrlStr.getHost(), iPort, pSock);
		return pGroup;
	}
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
	::TianShanIce::Transport::StorageLinkPrx sLink = _pathTicket->getStorageLink();
	
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
		valDestMac = GetResourceMapData(resMap,TianShanIce::SRM::rtEthernetInterface,"destMac");
		if ( valDestMac.type != TianShanIce::vtStrings || valDestMac.strs.size () == 0 )
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(envlog,EXPFMT("NSS",1045,"invalid dest mac type,should be vtStrings,or no destmac data with ticket [%s]"), strTicket.c_str());
			return NULL;
		}
		pTransHeader.strClient_id = valDestMac.strs[0];
		envlog(::ZQ::common::Log::L_INFO, CLOGFMT("NSS", "Get dest mac [%s] through ticket[%s]"), pTransHeader.strClient_id.c_str(),strTicket.c_str());

		//get destination IP
		valDestAddr = GetResourceMapData (resMap,TianShanIce::SRM::rtEthernetInterface,"destIP");
		if (valDestAddr.type != TianShanIce::vtStrings || valDestAddr.strs.size () == 0)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(envlog,EXPFMT("NSS",1043,"invalid destAddress type should be vtString or no destAddress data with ticket [%s]"), strTicket.c_str());		
			return NULL;
		}
		else
		{
			pTransHeader.strDestination = valDestAddr.strs[0];
			envlog(::ZQ::common::Log::L_INFO, CLOGFMT("NSS", "Get DestIP[%s] through ticket[%s]"),pTransHeader.strDestination.c_str(),strTicket.c_str());
		}

		//get destination port
		valDestPort = GetResourceMapData(resMap,TianShanIce::SRM::rtEthernetInterface,"destPort");
		if ( valDestPort.type != TianShanIce::vtInts || valDestPort.ints.size() == 0 )
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(envlog,EXPFMT("NSS",1044,"invalid dest por type,should be vtInts,or no destPort data with ticket [%s]"),strTicket.c_str());
			return NULL;
		}
		else
		{
			::std::stringstream ss;
			ss << valDestPort.ints[0];
			pTransHeader.strClient_port = ss.str();
			envlog(::ZQ::common::Log::L_INFO,CLOGFMT("NSS", "Get destPort[%s] through ticket[%s]"),pTransHeader.strClient_port.c_str(),strTicket.c_str());
		}

		//get bandwidth
		valBandwidth = GetResourceMapData(resMap , TianShanIce::SRM::rtTsDownstreamBandwidth,"bandwidth");
		if (valBandwidth.type != TianShanIce::vtLongs || valBandwidth.lints.size () == 0 )
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(envlog,EXPFMT("NSS",872, "Invalid BandWidth type should be vtLongs , or no bandwidth data with ticket [%s]"),	strTicket.c_str());
			return NULL;
		}
		else
		{
			::std::stringstream ss;
			long lBandwidth;
			void *tmp = getVariantValue(valBandwidth);
			if (tmp == NULL)
			{
				ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(envlog,EXPFMT("NSS",872, "Invalid BandWidth type should be vtLongs , or no bandwidth data with ticket [%s]"),	strTicket.c_str ());
				return NULL;
			}

			lBandwidth = *(long*)tmp;
			ss << lBandwidth;
			pTransHeader.strBandwidth = ss.str();
			envlog(::ZQ::common::Log::L_INFO,CLOGFMT("NSS","Get bandwidth [%d]bps through ticket[%s]"), lBandwidth,strTicket.c_str ());
		}

		void *tmp = getPrivateDataValue(PathTicketPD_Field(sop_name), pMap);
		if (tmp == NULL)
		{
			ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(envlog,EXPFMT("NSS",872, "NULL SOP_Name"));
			return NULL;
		}
		pTransHeader.strSop_name = *(string *)tmp;
	}
	catch(...)
	{
		envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT("NSS" ,"session initial failed"));
		return NULL;
	}

	RTSPClientSession *rtspClientSession = new RTSPClientSession();
	if (pNSSBaseConfig->_videoServer.vendor.compare("CCUR") == 0)
		rtspClientSession->_serverType = CCUR;
	else
		rtspClientSession->_serverType = NGOD;
	//rtspClientSession->m_RTSPR2Header.Volume.strName = sLink->getStorageInfo().netId;
	rtspClientSession->m_RTSPR2Header.Volume.strName = pNSSBaseConfig->_videoServer.vols[0].targetName;
	//rtspClientSession->strLocalIP = pConfig._a3LocalInfo.ip;
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
		return NULL;
	else
		return getVariantValue((*iter).second);
}

TianShanIce::Variant NGODStreamServiceImpl::GetResourceMapData(const TianShanIce::SRM::ResourceMap& rcMap,
										const TianShanIce::SRM::ResourceType& type,
										const std::string& strkey)
{
	TianShanIce::SRM::ResourceMap::const_iterator itResMap=rcMap.find(type);
	if(itResMap==rcMap.end())
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(envlog,EXPFMT("NSS",1001, "GetResourceMapData() type %d not found"), type);
	}
	TianShanIce::ValueMap::const_iterator it=itResMap->second.resourceData.find(strkey);
	if(it==itResMap->second.resourceData.end())
	{
		ZQTianShan::_IceThrow<TianShanIce::InvalidParameter>(envlog,EXPFMT("NSS",1001, "GetResourceMapData() value with key=%s not found"), strkey.c_str());
	}
	return it->second;
}

}//NSS
}//ZQTianShan
