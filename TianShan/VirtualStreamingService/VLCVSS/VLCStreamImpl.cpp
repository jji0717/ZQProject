#include <winsock2.h>
#include "VLCStreamImpl.h"
#include "VLCVSSCommitThrd.h"
#include "VLCVSSCfgLoader.h"
#include <iostream>
using namespace std;

extern ZQ::common::Config::Loader< ::ZQTianShan::VSS::VLC::VLCVSSCfg > pConfig;

namespace ZQTianShan{
namespace VSS{
namespace VLC{

static std::string strCategory = "VLCVSS";
/***************
//VLCStreamImpl
****************/
VLCStreamImpl::VLCStreamImpl(VLCVSSEnv &env):
_env(env),
_logFile(env._logFile),
_pool(env._thpool),
_isPlayed(false),
_isEnded(false)
{
	_vlcClientSession = NULL;
	//checkClientSession();

	//construct the playlist
	int32 idx = 0;
	for (::TianShanIce::Streamer::VLCStreamServer::VLCPlayList::iterator iter = vlcPL.begin(); iter != vlcPL.end(); iter++)
	{
		VLCPlayItem item;
		item._path = iter->path;
		item._length = iter->timeLen;
		item._userCtrlNum = iter->userCtrlNum;
		item._index = idx++;
		_pl._inputItem.push_back(item);
	}
	_pl._name = ident.name;
	_pl._ouputItem._access = "udp";
	_pl._ouputItem._mux = "ts";
	_pl._ouputItem._dstIp = destIp;
	_pl._ouputItem._dstPort = destPort;
	
	_pl._type = "broadcast";
}

VLCStreamImpl::~VLCStreamImpl()
{
	_vlcClientSession = NULL;
}

void VLCStreamImpl::renewPathTicket(const ::Ice::Current& c)
{
	try
	{
		::TianShanIce::Transport::PathTicketPrx& _pathTicketPrx = ::TianShanIce::Transport::PathTicketPrx::uncheckedCast(_env._communicator->stringToProxy(pathTicketStr));
		if(_pathTicketPrx!=NULL)
		{
			_pathTicketPrx->renew(pConfig._streamServiceProp.synInterval * 1000);//renew path ticket
		}
		//renewStatusReport(RENEW_OK); //if renew ok, just continue the rest job
	}
	catch (::TianShanIce::InvalidParameter&) 
	{
		//here is just a log message 
		envlog(::ZQ::common::Log::L_DEBUG,CLOGFMT(renewPathTicket,"session(%s) invalid renew time =%d(s)"), ident.name.c_str(),pConfig._streamServiceProp.synInterval);
	}
	catch(Ice::ObjectNotExistException& ex)
	{
		//if the relative path ticket is not available , kill playlist
		envlog(::ZQ::common::Log::L_ERROR,CLOGFMT(renewPathTicket,"session(%s) renew fail with renew time =%d(s) and error is %s"), ident.name.c_str(), pConfig._streamServiceProp.synInterval,ex.ice_name().c_str());
		::Ice::Current c;
		try
		{
			destroy(c);
			//delete this;
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
		envlog(::ZQ::common::Log::L_ERROR,CLOGFMT(renewPathTicket,"session(%s) renew fail with time =%d(s) and unexpect error"), ident.name.c_str(), pConfig._streamServiceProp.synInterval);
		//renewStatusReport(RENEW_NOCONNECT);
	}
}

bool VLCStreamImpl::checkClientSession()
{
	return true;
	//if (_vlcClientSession != NULL)
	//{
	//	_vlcClientSession->_pTelnetClient = _env._vlcTelnetSessionPool.getActiveTelnet();
	//	return true;
	//}
	//else
	//{
	//	//try to find in map
	//	_vlcClientSession = const_cast<VLCTelnetSession *>(_env._vlcTelnetSessionPool.findSession(sessKey));
	//	if (_vlcClientSession == NULL)
	//	{
	//		envlog(::ZQ::common::Log::L_ERROR, CLOGFMT(VLCStreamImpl,"checkClientSession: stream(%s) can't use key(%s) to find session"), ident.name.c_str(), sessKey.c_str());
	//		return false;
	//	}
	//	else
	//	{
	//		envlog(::ZQ::common::Log::L_DEBUG, CLOGFMT(VLCStreamImpl,"checkClientSession: stream(%s) use key(%s) to find session ok"), ident.name.c_str(), sessKey.c_str());
	//		return true;
	//	}
	//}
}

//impl of TMVSS
::std::string VLCStreamImpl::
	getOnDemandSessionId(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return ident.name;
}

::std::string VLCStreamImpl::
	getSessionId(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return sessId;
}

::TianShanIce::Streamer::VLCStreamServer::VLCPlayList VLCStreamImpl::
	getPlayList(const ::Ice::Current& ) const
{
	RLock rLock(*this);
	return vlcPL;
}

::std::string VLCStreamImpl::
	getPathticketStr(const ::Ice::Current& ) const
{
	RLock rLock(*this);
	return pathTicketStr;
}

::std::string VLCStreamImpl::
getDestIp(const ::Ice::Current& ) const
{
	RLock rLock(*this);
	return destIp;
}

::Ice::Int VLCStreamImpl::
getDestPort(const ::Ice::Current& ) const
{
	RLock rLock(*this);
	return destPort;
}

::std::string VLCStreamImpl::
	getCtrlURL(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return "";
}

//impl of stream
void VLCStreamImpl::
	allotPathTicket(const ::TianShanIce::Transport::PathTicketPrx& ticket,
	const ::Ice::Current& c)
	throw (::TianShanIce::Transport::ExpiredTicket,
	::TianShanIce::ServerError,
	::TianShanIce::NotSupported)
{

}

void VLCStreamImpl::
	destroy(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError)
{
	WLock sync(*this);
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCStreamImpl, ".......destroy() Entry [%s]........"), ident.name.c_str());
	if (checkClientSession() == false)
	{
		//throw exception
		_env._eVLCStream->remove(ident);

		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"VLCStreamImpl",143,"destroy() Sess(%s) fail to find exist telnet client session", ident.name.c_str());
		return;
	}
	DWORD sTime = GetTickCount();
	std::string strMsg;
	bool b = _env._vlcTelnetSessionPool.ControlVLC(VLCTEARDOWN, sessKey, _pl, strNull, strNull, strNull, strMsg, pConfig._telnetProp.timeOut);
	//bool b = _vlcClientSession->teardownPL(_pl, strMsg);
	//_env._vlcTelnetSessionPool.deActiveTelnet(_vlcClientSession->_pTelnetClient);
	_env._vlcTelnetSessionPool.delSession(sessKey);
	_env._eVLCStream->remove(ident);
	if (b == false)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"VLCStreamImpl",143,"destroy() Sess(%s) fail, telnet call fail", ident.name.c_str());
	}
	else
	{
		// sink end event
		//if (!_isEnded)
		//{
		//	_isEnded = true;
		//	listmem params;
		//	params.type = E_PLAYLIST_END;
		//	params.param[EventField_PlaylistGuid] = ident.name;
		//	params.param[EventField_EventCSEQ] = InterlockedIncrement(&_env._eventSequence); // lock problem
		//	_env._VLCVSSEventSink._paramsList.PushBack(params);
		//	envlog(ZQ::common::Log::L_NOTICE, CLOGFMT(VLCStreamImpl, "destroy() play list[%s] reach end.Sink E_PLAYLIST_END event"), ident.name.c_str());
		//}

		// sink destroy event 
		ZQTianShan::VSS::VLC::listmem params;
		params.type = ZQTianShan::VSS::VLC::E_PLAYLIST_DESTROYED;
		params.param[EventField_PlaylistGuid] = ident.name;
		params.param[EventField_EventCSEQ] = InterlockedIncrement(&_env._eventSequence);
		_env._VLCVSSEventSink._paramsList.PushBack(params);
		envlog(ZQ::common::Log::L_NOTICE, CLOGFMT(VLCStreamImpl, "destroy() play list [%s] is destroyed.Sink E_PLAYLIST_DESTROYED event. teardown cost %dms"), ident.name.c_str(),  GetTickCount() - sTime);
	}
}

void VLCStreamImpl::
	commit_async(const ::TianShanIce::Streamer::AMD_Stream_commitPtr& amdStream, 
	const ::Ice::Current& c)
{	
	if (checkClientSession() == false)
	{
		//throw exception
		::std::string strErrMsg = ::std::string("VLCVSSCommitThrd::commit_async() Sess(") + ident.name + std::string(") failed to get telnet connection from pool");
		::ZQ::common::Exception ex(strErrMsg);
		amdStream->ice_exception(ex);
		return;
	}
	int32 idx = 0;
	for (VLCPlayItemList::iterator iter = _pl._inputItem.begin(); iter != _pl._inputItem.end(); iter++)
	{
		iter->_index = idx++;
	}
	VLCVSSCommitThrd *commitThrd = new VLCVSSCommitThrd(this, pathTicketStr, amdStream);
	commitThrd->start();
}

::std::string VLCStreamImpl::
	lastError(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return std::string("");
}

::Ice::Identity VLCStreamImpl::
	getIdent(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return ident;
}

void VLCStreamImpl::
	setConditionalControl(const ::TianShanIce::Streamer::ConditionalControlMask& mask, 
	const ::TianShanIce::Streamer::ConditionalControlPrx& condCtrl, 
	const ::Ice::Current& c)
{
}

bool VLCStreamImpl::play(const ::Ice::Current& c)
{
	RLock sync(*this);
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCStreamImpl, ".......play() Entry [%s]........"), ident.name.c_str());

	//TODO: send play message
	std::string strMsg;
	DWORD sTime = GetTickCount();
	std::string strVLCCmd;
	long timeout = 0;

	int curItem = 0;
	// find current item position and index
	TianShanIce::ValueMap instanceStats;
	getInfo(TianShanIce::Streamer::infoSTREAMNPTPOS, instanceStats, c);
	TianShanIce::Variant res = instanceStats[PLPosition];
	float position = 0.0;
	if (!res.floats.empty())
		position = res.floats.at(0);
	res = instanceStats[PLIndex];
	if (!res.ints.empty())
		curItem = res.ints.at(0);

	if (getCurrentState(c) == ::TianShanIce::Streamer::stsPause)
	{
		strVLCCmd = strVLCPause;
	}
	else
	{
		strVLCCmd = strVLCPlay;
	}
	// calculate timeout
	VLCPlayItemList::iterator iter;
	if (curItem == 0)
	{
		iter = _pl._inputItem.begin();
	}
	else
	{
		iter = find_if(_pl._inputItem.begin(), _pl._inputItem.end(), FindByPLCtrlNum(curItem));
	}
	if (iter != _pl._inputItem.end())
	{
		timeout = static_cast<long>(iter->_length * (1 - position));
		while (++iter != _pl._inputItem.end())
		{
			timeout += iter->_length;
		}
	}
	else
		timeout = 5;
	

	bool b = _env._vlcTelnetSessionPool.ControlVLC(VLCCONTROL, sessKey, _pl, strVLCCmd, strNull, strNull, strMsg, pConfig._telnetProp.timeOut);
	//bool b = _vlcClientSession->controlPL(_pl, strVLCCmd, strNull, strNull, strMsg);
	//_env._vlcTelnetSessionPool.deActiveTelnet(_vlcClientSession->_pTelnetClient);
	if (b)
	{
		_isPlayed = true;
		_env._streamWatchDog->watch(ident, timeout);
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(VLCStreamImpl, "play() session(%s) cost %dms"), ident.name.c_str(), GetTickCount() - sTime);
	}
	else
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(VLCStreamImpl, "play() session(%s) failed, cost %dms"), ident.name.c_str(), GetTickCount() - sTime);
	}
	return b;
}

bool VLCStreamImpl::
	setSpeed(::Ice::Float newSpeed,
	const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter, 
	::TianShanIce::ServerError, 
	::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);

	if (checkClientSession() == false)
	{
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"VLCStreamImpl",500,"setSpeed() Sess(%s) fail to find telnet client session", ident.name.c_str());
		return false;
	}

	std::string strMsg;
	DWORD sTime = GetTickCount();

	bool b = _env._vlcTelnetSessionPool.ControlVLC(VLCCONTROL, sessKey, _pl, strVLCPlay, strNull, strNull, strMsg, pConfig._telnetProp.timeOut);
	//bool b = _vlcClientSession->controlPL(_pl, strVLCPlay, strNull, strNull, strMsg);
	//_env._vlcTelnetSessionPool.deActiveTelnet(_vlcClientSession->_pTelnetClient);
	if (b)
	{
		// when vlc can be set speed, this event must be sink
		/*if ()
		{
			listmem params;
			params.type = ::ZQTianShan::VSS::VLC::E_PLAYLIST_SPEEDCHANGED;
			params.param[EventField_EventCSEQ] = InterlockedIncrement(&_env._eventSequence); // lock problem
			_env._VLCVSSEventSink._paramsList.PushBack(params);
			envlog(ZQ::common::Log::L_NOTICE, CLOGFMT(VLCStreamImpl, "setSpeed() play list speed is changed.Sink E_PLAYLIST_SPEEDCHANGED event"));
		}*/
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(VLCStreamImpl, "setSpeed() session(%s) cost %dms"), ident.name.c_str(), GetTickCount() - sTime);
	}
	else
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(VLCStreamImpl, "setSpeed() session(%s) failed, cost %dms"), ident.name.c_str(), GetTickCount() - sTime);
	}
	return b;
}

bool VLCStreamImpl::
	pause(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError, 
	::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCStreamImpl, ".......pause() Entry [%s]........"), ident.name.c_str());
	if (checkClientSession() == false)
	{
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"VLCStreamImpl",500,"pause() Sess(%s) fail to find telnet client session", ident.name.c_str());
		return false;
	}

	if (getCurrentState(c) == ::TianShanIce::Streamer::stsPause)
	{
		ZQTianShan::_IceThrow<::TianShanIce::InvalidStateOfArt> (envlog,"VLCStreamImpl",143,"pause() session(ident:%s, sessId:%s) already in pause state", ident.name.c_str(), sessId.c_str());
		return false;
	}
		
	std::string strMsg;
	DWORD sTime = GetTickCount();

	TianShanIce::Streamer::StreamState preState = getCurrentState(c);
	bool b = _env._vlcTelnetSessionPool.ControlVLC(VLCCONTROL, sessKey, _pl, strVLCPause, strNull, strNull, strMsg, pConfig._telnetProp.timeOut);
	//bool b = _vlcClientSession->controlPL(_pl, strVLCPause, strNull, strNull, strMsg);
	//_env._vlcTelnetSessionPool.deActiveTelnet(_vlcClientSession->_pTelnetClient);
	if (b)
	{
		listmem params;
		params.type = ZQTianShan::VSS::VLC::E_PLAYLIST_STATECHANGED;
		params.param[EventField_PlaylistGuid] = ident.name;
		params.param[EventField_PrevState] = preState;
		params.param[EventField_CurrentState] = getCurrentState(c);
		params.param[EventField_EventCSEQ] =InterlockedIncrement(&_env._eventSequence); // lock problem
		_env._VLCVSSEventSink._paramsList.PushBack(params);
		envlog(ZQ::common::Log::L_NOTICE, CLOGFMT(VLCStreamImpl, "pause() play list[%s] status is changed.Sink E_PLAYLIST_STATECHANGED event"), ident.name.c_str());
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(VLCStreamImpl, "pause() session(%s) cost %dms"), ident.name.c_str(), GetTickCount() - sTime);
	}
	else
	{
		envlog(ZQ::common::Log::L_ERROR, CLOGFMT(VLCStreamImpl, "pause() session(%s) failed, cost %dms"), ident.name.c_str(), GetTickCount() - sTime);
	}
	return b;
}

bool VLCStreamImpl::
	resume(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError, 
	::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);

	return play(c);
}

::TianShanIce::Streamer::StreamState VLCStreamImpl::
	getCurrentState(const ::Ice::Current& c) const
	throw (::TianShanIce::ServerError)
{
	RLock sync(*this);
	//VLCTelnetSession  *tmpSess = const_cast<VLCTelnetSession *>(_env._vlcTelnetSessionPool.findSession(sessKey));
	//if (tmpSess == NULL)
	//{
	//	ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"VLCStreamImpl",143,"getCurrentState() session(ident:%s) fail to find exist telnet ClientSession", ident.name.c_str());
	//	return ::TianShanIce::Streamer::stsStop;
	//}
	
	int ret = 0;
	DWORD sTime = GetTickCount();
	std::string strMsg;

	bool b = _env._vlcTelnetSessionPool.ControlVLC(VLCSHOW, sessKey, const_cast<CommonVLCPlaylist &>(_pl), strNull, strNull, strNull, strMsg, pConfig._telnetProp.timeOut);
	//bool b = tmpSess->showPL(const_cast<CommonVLCPlaylist &>(_pl), strMsg);
	//_env._vlcTelnetSessionPool.deActiveTelnet(_vlcClientSession->_pTelnetClient);
	if (b == false)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"VLCStreamImpl",143,"getCurrentState() session(ident:%s) fail, telnet call fail", ident.name.c_str());
		return ::TianShanIce::Streamer::stsStop;
	}
	else
	{
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(VLCStreamImpl, "getCurrentState() session(ident:%s) cost %dms"), ident.name.c_str(), GetTickCount() - sTime);
		std::string strState;
		if (_env._telnetParser.getContentByHeader(strMsg, PLState, strState))
		{
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(VLCStreamImpl, "getCurrentState() session(ident:%s) return state: %s"), ident.name.c_str(), strState.c_str());
			if (strState.compare(PLStatePlay) == 0)
				return ::TianShanIce::Streamer::stsStreaming;
			else if (strState.compare(PLStatePause) == 0)
				return ::TianShanIce::Streamer::stsPause;
			else if (strState.compare(PLStateStop) == 0)
				return ::TianShanIce::Streamer::stsStop;
			else
				return  ::TianShanIce::Streamer::stsStop;
		}
		else
		{
			if (!strMsg.empty() && strMsg.find("show") == std::string::npos)
			{
				// return previous status
				if (_isPlayed)
				{
					return ::TianShanIce::Streamer::stsStreaming;
				}
				return ::TianShanIce::Streamer::stsSetup;
			}
			if (_env._telnetParser.checkPLName(strMsg, _pl._name.c_str()) == false)
			{
				envlog(ZQ::common::Log::L_ERROR, CLOGFMT(VLCStreamImpl, "getCurrentState() session(ident:%s) failed to parse state"), ident.name.c_str());
				return ::TianShanIce::Streamer::stsStop;
			}
			else
			{
				if (_isPlayed)
				{
					envlog(ZQ::common::Log::L_INFO, CLOGFMT(VLCStreamImpl, "getCurrentState() session(ident:%s) in stop state"), ident.name.c_str());
					return ::TianShanIce::Streamer::stsStop;
				}
				else
				{
					envlog(ZQ::common::Log::L_INFO, CLOGFMT(VLCStreamImpl, "getCurrentState() session(ident:%s) in setup state"), ident.name.c_str());
					return ::TianShanIce::Streamer::stsSetup;
				}
			}
		}
	}
}

::TianShanIce::SRM::SessionPrx VLCStreamImpl::
	getSession(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError)
{
	return NULL;
	//TODO
}

void VLCStreamImpl::
	setMuxRate(::Ice::Int nowRate, 
	::Ice::Int maxRate,
	::Ice::Int minRate,
	const ::Ice::Current& c)
{
}

bool VLCStreamImpl::
	allocDVBCResource(::Ice::Int serviceGroupID,
	::Ice::Int bandWidth,
	const ::Ice::Current& c)
	throw (::TianShanIce::ServerError)
{
	return true;
}

::Ice::Long VLCStreamImpl::
	seekStream(::Ice::Long offset,
	::Ice::Int startPos,
	const ::Ice::Current& c)
	throw (::TianShanIce::ServerError,
	::TianShanIce::InvalidParameter)
{
	//::Ice::Int iCurrentIdx = current(c);
	RLock sync(*this);

	if (checkClientSession() == false)
	{
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"VLCStreamImpl",500,"seekStream() Sess(%s) fail to find telnet client session", ident.name.c_str());
		return -1;
	}

	::Ice::Long timeOffset = 0;
	for (VLCPlayItemList::iterator iter = _pl._inputItem.begin(); iter != _pl._inputItem.end(); iter++)
	{
		timeOffset += iter->_length;
		if (timeOffset > offset)//get the specified play item
		{
			::Ice::Long index = iter->_index;
			::Ice::Long timePoint = offset - (timeOffset - iter->_length);

			//add by zjm
			::Ice::Float itemPercent = timePoint;
			itemPercent = (itemPercent / iter->_length) * 100;

			DWORD sTime = GetTickCount();

			std::stringstream ssItemPercent;
			ssItemPercent << itemPercent;
			std::string strItemPercent = ssItemPercent.str();
		/*	std::stringstream ssTimeOffset;
			ssTimeOffset << timePoint;
			std::string strTimeOffset = ssTimeOffset.str() + "ms";*/


			std::stringstream ssPLIdx;
			ssPLIdx << (index+1);
			std::string strMsg;
			
			//not play current item, we need first to skip to this item
			bool b = false;
			//if (current(c) != index + 1)
			{
				b = _env._vlcTelnetSessionPool.ControlVLC(VLCCONTROL, sessKey, _pl, strVLCPlay, strNull, ssPLIdx.str(), strMsg, pConfig._telnetProp.timeOut);
				if ( b == false)
				//if ( _vlcClientSession->controlPL(_pl, strVLCPlay, strNull, ssPLIdx.str(), strMsg) == false)	
				{
					//_env._vlcTelnetSessionPool.deActiveTelnet(_vlcClientSession->_pTelnetClient);
					ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"VLCStreamImpl",500,"seekStream() Sess(%s) fail to seek to the specified timepoint(%d)", ident.name.c_str(), offset);
					return -1;
				}
			}

			b = _env._vlcTelnetSessionPool.ControlVLC(VLCCONTROL, sessKey, _pl, strVLCSeek, strItemPercent, strNull, strMsg, pConfig._telnetProp.timeOut);
			//b = _vlcClientSession->controlPL(_pl, strVLCSeek, strTimeOffset, strNull, strMsg);
			//_env._vlcTelnetSessionPool.deActiveTelnet(_vlcClientSession->_pTelnetClient);
			DWORD oldTick = GetTickCount();

			if (b == false)
			{
				//envlog(ZQ::common::Log::L_INFO, CLOGFMT(VLCStreamImpl, "seekStream() Sess(%s) fail to seek to the specified timepoint(%d)"), ident.name.c_str(), offset);
				ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"VLCStreamImpl",500,"seekStream() Sess(%s) fail to seek to the specified timepoint(%d)", ident.name.c_str(), offset);
				return -1;
			}
			else
			{
				if (getCurrentState(c) == ::TianShanIce::Streamer::stsPause)
				{
					b = _env._vlcTelnetSessionPool.ControlVLC(VLCCONTROL, sessKey, _pl, strVLCPause, strNull, strNull, strMsg, pConfig._telnetProp.timeOut);
				}

				if (b == false)
				{
					ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"VLCStreamImpl",500,"seekStream() Sess(%s) fail to seek/play to the specified timepoint(%d)", ident.name.c_str(), offset);
					return -1;
				}

				_isPlayed = true;
				// add to watch dog
				long timeout = 0;
				for (VLCPlayItemList::iterator iterInner = _pl._inputItem.begin(); iterInner != _pl._inputItem.end(); iterInner++)
				{
					timeout += iterInner->_length;
				}
				timeout -= (offset + (GetTickCount() - oldTick));
				timeout = (timeout > 0 ? timeout : 1);
				_env._streamWatchDog->watch(ident, timeout);
				envlog(ZQ::common::Log::L_INFO, CLOGFMT(VLCStreamImpl, "seekStream() Sess(%s) success seek to specified timepoint(%d)"), ident.name.c_str(), offset);
				return timePoint;
			}
		}
	}

	//_env._vlcTelnetSessionPool.deActiveTelnet(_vlcClientSession->_pTelnetClient);
	ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"VLCStreamImpl",500,"seekStream() Sess(%s) fail to seek, the time offset(%d) invalid", ident.name.c_str(), offset);
	return -1;
}

::TianShanIce::Streamer::StreamInfo VLCStreamImpl::
	playEx(::Ice::Float newSpeed, 
	::Ice::Long timeOffset,
	::Ice::Short from,
	const ::TianShanIce::StrValues& expectedProps,
	const ::Ice::Current& c)
	throw (::TianShanIce::ServerError, 
	::TianShanIce::InvalidStateOfArt)
{
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCStreamImpl, "........palyEx() : entry [%s]......."), ident.name.c_str());
	::TianShanIce::Streamer::StreamInfo streamInfo;
	if (0 == timeOffset && 0 == from)
	{
		play(c);
		return streamInfo;
	}
	RLock sync(*this);
	
	if (seekStream(timeOffset, from, c) >=0)
		return streamInfo;
	else
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"VLCStreamImpl",500,"playEx() Sess(%s) fail to play", ident.name.c_str());
		return streamInfo;
	}
}

::TianShanIce::Streamer::StreamInfo VLCStreamImpl::
	pauseEx(const ::TianShanIce::StrValues& expectedProps, 
	const ::Ice::Current& c)
	throw (::TianShanIce::ServerError, 
	::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCStreamImpl, ".......pauseEx() Entry [%s]........"), ident.name.c_str());
	::TianShanIce::Streamer::StreamInfo streamInfo;

	if (checkClientSession() == false)
	{
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"VLCStreamImpl",500,"pauseEx() Sess(%s) fail to find telnet client session", ident.name.c_str());
		return streamInfo;
	}
	
	std::string strMsg;
	DWORD sTime = GetTickCount();

	TianShanIce::Streamer::StreamState preState = getCurrentState(c);
	bool b = _env._vlcTelnetSessionPool.ControlVLC(VLCCONTROL, sessKey, _pl, strVLCPause, strNull, strNull, strMsg, pConfig._telnetProp.timeOut);
	//bool b = _vlcClientSession->controlPL(_pl, strVLCPause, strNull, strNull, strMsg);
	//_env._vlcTelnetSessionPool.deActiveTelnet(_vlcClientSession->_pTelnetClient);
	if (b)
	{
		listmem params;
		params.type = ZQTianShan::VSS::VLC::E_PLAYLIST_STATECHANGED;
		params.param[EventField_PlaylistGuid] = ident.name;
		params.param[EventField_PrevState] = preState;
		params.param[EventField_CurrentState] = getCurrentState(c);
		params.param[EventField_EventCSEQ] = InterlockedIncrement(&_env._eventSequence); // lock problem
		_env._VLCVSSEventSink._paramsList.PushBack(params);
		envlog(ZQ::common::Log::L_NOTICE, CLOGFMT(VLCStreamImpl, "pauseEx() play list[%s] status is changed.Sink E_PLAYLIST_STATECHANGED event"), ident.name.c_str());
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(VLCStreamImpl, "pauseEx() session(%s) cost %dms"), ident.name.c_str(), GetTickCount() - sTime);
		return streamInfo;
	}
	else
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(envlog, "VLCStreamImpl", 500, "pauseEx() session(%s) failed, cost %dms", ident.name.c_str(), GetTickCount() - sTime);
		return streamInfo;
	}
}

void VLCStreamImpl::
	setSpeedEx(::Ice::Float newSpeed,
	::Ice::Long& timeOffset,
	::Ice::Float& currentSpeed,
	const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter,
	::TianShanIce::ServerError,
	::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	if (seekStream(timeOffset, 1, c) < 0)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError>(envlog,"VLCStreamImpl",500,"setSpeedEx() Sess(%s) fail to play", ident.name.c_str());
	}
}

//impl of playlist
::std::string VLCStreamImpl::
	getId(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return ident.name;
}

bool VLCStreamImpl::
	getInfo(::Ice::Int mask,
	::TianShanIce::ValueMap& varOut,
	const ::Ice::Current& c)
{
	RLock sync(*this);
	switch(mask)
	{
	case TianShanIce::Streamer::infoSTREAMNPTPOS:
		break;
	default:
		return false;
	}
	DWORD sTime = GetTickCount();
	std::string strMsg;

	bool b = _env._vlcTelnetSessionPool.ControlVLC(VLCSHOW, sessKey, const_cast<CommonVLCPlaylist &>(_pl), strNull, strNull, strNull, strMsg, pConfig._telnetProp.timeOut);
	if (b == false)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"VLCStreamImpl",143,"getInfo() session(ident:%s) fail, telnet call fail", ident.name.c_str());
		return false;
	}
	else
	{
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(VLCStreamImpl, "getInfo() session(ident:%s) cost %dms"), ident.name.c_str(), GetTickCount() - sTime);
		std::string strPosition;
		if (_env._telnetParser.getContentByHeader(strMsg, PLPosition, strPosition))
		{
			TianShanIce::Variant res;
			res.bRange = false;
			res.floats.clear();
			res.floats.push_back(atof(strPosition.c_str()));
			res.type = TianShanIce::vtFloats;
			varOut[PLPosition] = res;
		}
		std::string strIndex;
		if (_env._telnetParser.getContentByHeader(strMsg, PLIndex, strIndex))
		{
			TianShanIce::Variant res;
			res.bRange = false;
			res.ints.clear();
			res.ints.push_back(atoi(strIndex.c_str()));
			varOut[PLIndex] = res;
		}
	/*	std::string strState;
		if (_env._telnetParser.getContentByHeader(strMsg, PLState, strState))
		{
			TianShanIce::Variant res;
			res.bRange = false;
			TianShanIce::Streamer::StreamState curState;
			if (strState.compare(PLStatePlay) == 0)
				curState = TianShanIce::Streamer::stsStreaming;
			else if (strState.compare(PLStatePause) == 0)
				curState = TianShanIce::Streamer::stsPause;
			else if (strState.compare(PLStateStop) == 0)
				curState = TianShanIce::Streamer::stsStop;
			else
				curState = TianShanIce::Streamer::stsStop;
			
		}*/
	}
	return true;
}

::Ice::Int VLCStreamImpl::
	insert(::Ice::Int userCtrlNum, 
	const ::TianShanIce::Streamer::PlaylistItemSetupInfo& newItemInfo,
	::Ice::Int whereUserCtrlNum,
	const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter, 
	::TianShanIce::ServerError, 
	::TianShanIce::InvalidStateOfArt)
{
	WLock wLock(*this);

	//check the new item info
	::TianShanIce::Streamer::VLCStreamServer::VLCItem newItem;
	if (!setItemInfo(newItem, userCtrlNum, newItemInfo, whereUserCtrlNum))
		{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"VLCStreamImpl",500,"setItemInfo() Sess(%s) fail to get item information", ident.name.c_str());
		return 0;
	}

	for (::TianShanIce::Streamer::VLCStreamServer::VLCPlayList::iterator iter = vlcPL.begin(); iter != vlcPL.end(); iter++)
	{
		if (whereUserCtrlNum == iter->userCtrlNum)
		{
			vlcPL.insert(iter, newItem);

			//update VLC playlist
			VLCPlayItem vlcNewItem;
			vlcNewItem._userCtrlNum = userCtrlNum;
			vlcNewItem._path = newItem.path;
			vlcNewItem._length = newItem.timeLen;
			VLCPlayItemList::iterator vlcIter = find_if(_pl._inputItem.begin(), _pl._inputItem.end(), FindByPLCtrlNum(userCtrlNum));
			if (vlcIter != _pl._inputItem.end())
				_pl._inputItem.insert(vlcIter, vlcNewItem);
			else
				_pl._inputItem.insert(vlcIter, vlcNewItem);

			return whereUserCtrlNum;
		}
	}

	vlcPL.push_back(newItem);
	return whereUserCtrlNum;
}


::Ice::Int VLCStreamImpl::
	pushBack(::Ice::Int userCtrlNum,
	const ::TianShanIce::Streamer::PlaylistItemSetupInfo& newItemInfo,
	const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter,
	::TianShanIce::ServerError,
	::TianShanIce::InvalidStateOfArt)
{
	WLock wLock(*this);
	::Ice::Int whereUserCtrlNum = 0;
	for (::TianShanIce::Streamer::VLCStreamServer::VLCPlayList::iterator iter = vlcPL.begin(); iter != vlcPL.end(); iter++)
	{
		if (userCtrlNum == iter->userCtrlNum)
		{
			ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"VLCStreamImpl",500,"pushBack() Sess(%s) userCtrlNum(%d) already exist", ident.name.c_str(), userCtrlNum);
		}
		whereUserCtrlNum = iter->whereUserCtrlNum;
	}

	whereUserCtrlNum++;
	::TianShanIce::Streamer::VLCStreamServer::VLCItem newItem;
	if (setItemInfo(newItem, userCtrlNum, newItemInfo, whereUserCtrlNum))
	{
		vlcPL.push_back(newItem);
		VLCPlayItem vlcNewItem;
		vlcNewItem._userCtrlNum = userCtrlNum;
		vlcNewItem._path = newItem.path;
		vlcNewItem._length = newItem.timeLen;
		_pl._inputItem.push_back(vlcNewItem);
		return whereUserCtrlNum;
	}
	else
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"VLCStreamImpl",500,"setItemInfo() Sess(%s) fail to get item information", ident.name.c_str());
		return 0;
	}
}

::Ice::Int VLCStreamImpl::
	size(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return 1;
}

::Ice::Int VLCStreamImpl::
	left(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	return 1;
}

bool VLCStreamImpl::
	empty(const ::Ice::Current& c) const
	throw (::TianShanIce::ServerError,
	::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	return vlcPL.empty();
}

::Ice::Int VLCStreamImpl::
	current(const ::Ice::Current& c) const
	throw (::TianShanIce::ServerError,
	::TianShanIce::InvalidStateOfArt)
{
	RLock rLock(*this);

	/*VLCTelnetSession  *tmpSess = const_cast<VLCTelnetSession *>(_env._vlcTelnetSessionPool.findSession(sessKey));
	if (tmpSess == NULL)
	{
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"VLCStreamImpl",500,"current() Sess(%s) fail to find telnet client session", ident.name.c_str());
		return -1;
	}*/
	
	//tmpSess->_pTelnetClient = _env._vlcTelnetSessionPool.getActiveTelnet();
	//send and recv response from vlc
	std::string strMsg;

	bool b = _env._vlcTelnetSessionPool.ControlVLC(VLCSHOW, sessKey, const_cast<CommonVLCPlaylist &>(_pl), strNull, strNull, strNull, strMsg, pConfig._telnetProp.timeOut);
	//bool b = tmpSess->showPL(const_cast<CommonVLCPlaylist &>(_pl), strMsg);
	//_env._vlcTelnetSessionPool.deActiveTelnet(tmpSess->_pTelnetClient);
	if (b)
	{
		//TODO:parse message
		std::string strPLIdx;
		if (_env._telnetParser.getContentByHeader(strMsg, PLIndex, strPLIdx))
		{
			//TODO:log success
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(VLCStreamImpl, "current() Sess(%s) get current playlist index(%d)"), ident.name.c_str(), atoi(strPLIdx.c_str()));
			return atoi(strPLIdx.c_str());
		}
		else
		{
			if (_env._telnetParser.checkPLName(strMsg, _pl._name.c_str()) == false)
			{
				ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"VLCStreamImpl",500,"current() Sess(%s) fail to get play index", ident.name.c_str());
				return -1;
			}
			else
				return -1;
		}
	}
	else
	{
		//TODO:log error
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"VLCStreamImpl",500,"current() Sess(%s) fail to get play index", ident.name.c_str());
		return -1;
	}
}

void VLCStreamImpl::
	erase(::Ice::Int whereUserCtrlNum,
	const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter,
	::TianShanIce::ServerError,
	::TianShanIce::InvalidStateOfArt)
{
	RLock rLock(*this);
	::TianShanIce::Streamer::VLCStreamServer::VLCPlayList::iterator iter = find_if(vlcPL.begin(), vlcPL.end(), FindByCtlrNum(whereUserCtrlNum));
	if (iter == vlcPL.end())
	{
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"VLCStreamImpl",500,"erase() Sess(%s) fail to find item(index:%d)", ident.name.c_str(), whereUserCtrlNum);
	}
	else
	{
		VLCInputDelItem delItem;
		delItem._path = iter->path;

		_pl._inputDelItem.push_back(delItem);
		std::string strMsg;

		bool b = _env._vlcTelnetSessionPool.ControlVLC(VLCSETUPDel, sessKey, _pl, strNull, strNull, strNull, strMsg, pConfig._telnetProp.timeOut);
		//bool b = _vlcClientSession->setupDelPL(_pl, strMsg);
		//_env._vlcTelnetSessionPool.deActiveTelnet(_vlcClientSession->_pTelnetClient);
		if (b)
		{
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(VLCStreamImpl, "erase() Sess(%s) delete the specified playlist index(%d) name(%s)"), ident.name.c_str(), whereUserCtrlNum, delItem._path);
			vlcPL.erase(iter);
			for (VLCPlayItemList::iterator iter1 = _pl._inputItem.begin(); iter1 != _pl._inputItem.end(); iter++)
			{
				if (iter1->_path.compare(delItem._path) == 0)
				{
					_pl._inputItem.erase(iter1);
					break;
				}
			}
		}
		else
		{
			//TODO:log error
			ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"VLCStreamImpl",500,"current() Sess(%s) fail to get play index", ident.name.c_str());
		}
	}
}

::Ice::Int VLCStreamImpl::
	flushExpired(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError,
	::TianShanIce::InvalidStateOfArt)
{
	RLock rLock(*this);
	return 1;
}

bool VLCStreamImpl::
	clearPending(bool includeInitedNext,
	const ::Ice::Current& c)
	throw (::TianShanIce::ServerError,
	::TianShanIce::InvalidStateOfArt)
{
	RLock rLock(*this);
	return true;
}

bool VLCStreamImpl::
	isCompleted(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError,
	::TianShanIce::InvalidStateOfArt)
{
	RLock rLock(*this);
	return true;
}

::TianShanIce::IValues VLCStreamImpl::
	getSequence(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	::TianShanIce::IValues tmpValues;
	for (::TianShanIce::Streamer::VLCStreamServer::VLCPlayList::const_iterator iter = vlcPL.begin(); iter != vlcPL.end(); iter++)
		tmpValues.push_back(iter->userCtrlNum);
	return tmpValues;
}

::Ice::Int VLCStreamImpl::
	findItem(::Ice::Int userCtrlNum,
	::Ice::Int from,
	const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter,
	::TianShanIce::ServerError,
	::TianShanIce::InvalidStateOfArt)
{
	RLock rLock(*this);
	for (::TianShanIce::Streamer::VLCStreamServer::VLCPlayList::iterator iter = vlcPL.begin(); iter != vlcPL.end();iter++)
	{
		if (iter->userCtrlNum == userCtrlNum)
			return userCtrlNum;
	}
	
	ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"VLCStreamImpl",500,"findItem() Sess(%s) fail to get play index", ident.name.c_str());
	return -1;
}

bool VLCStreamImpl::
	distance(::Ice::Int to,
	::Ice::Int from,
	::Ice::Int& dist,
	const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter,
	::TianShanIce::ServerError,
	::TianShanIce::InvalidStateOfArt)
{
	RLock rLock(*this);
	return true;
}

bool VLCStreamImpl::
	skipToItem(::Ice::Int where, 
	bool bPlay,
	const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter, 
	::TianShanIce::ServerError,
	::TianShanIce::InvalidStateOfArt)
{
	WLock wLock(*this);
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCStreamImpl, ".......skipToItem() Entry [%s]........"), ident.name.c_str());
	//find the specified item
	VLCPlayItemList::iterator iter = find_if(_pl._inputItem.begin(), _pl._inputItem.end(), FindByPLCtrlNum(where));
	if (iter == _pl._inputItem.end())
	{
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"VLCStreamImpl",500,"skipToItem() Sess(%s) fail to get play index(CtrlNum:%d)", ident.name.c_str(), where);
		return false;
	}

	if (checkClientSession() == false)
	{
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"VLCStreamImpl",500,"skipToItem() Sess(%s) fail to find telnet client session", ident.name.c_str());
		return false;
	}

	//control the item
	std::string strMsg;
	std::stringstream ss;
	ss << where;

	//bool b = _vlcClientSession->controlPL(_pl, strVLCPlay, strNull, ss.str(), strMsg);
	bool b = _env._vlcTelnetSessionPool.ControlVLC(VLCCONTROL, sessKey, _pl, strVLCPlay, strNull, ss.str(), strMsg, pConfig._telnetProp.timeOut);
	DWORD sTickTime = GetTickCount();
	if (b)
	{
		_isPlayed = true;
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(VLCStreamImpl, "skipToItem() Sess(%s) skip to specified item(CtrlNum:%d, index:%d)"), ident.name.c_str(), where, iter->_index);

		if (bPlay == false)
		{
			 b = _env._vlcTelnetSessionPool.ControlVLC(VLCCONTROL, sessKey, _pl, strVLCPause, strNull, strNull, strMsg, pConfig._telnetProp.timeOut);
			if (b == false)
			//if (_vlcClientSession->controlPL(_pl, strVLCPause, strNull, strNull, strMsg) == false)
			{
				envlog(ZQ::common::Log::L_INFO, CLOGFMT(VLCStreamImpl, "skipToItem() Sess(%s) fail to set specified item(CtrlNum:%d, index:%d) pause"), ident.name.c_str(), where, iter->_index);
			}
		}
		else
		{
			// add to watch dog
			_isPlayed = true;
			long timeout = 0;
			while (iter != _pl._inputItem.end())
			{
				timeout += iter->_length;
				iter++;
			}
			_env._streamWatchDog->watch(ident, timeout - (GetTickCount() - sTickTime));
		}
		return true;
	}
	else
	{
		//TODO:log error
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"VLCStreamImpl",500,"current() Sess(%s) fail to get play index", ident.name.c_str());
		return false;
	}
}

bool VLCStreamImpl::
	seekToPosition(::Ice::Int UserCtrlNum,
	::Ice::Int timeOffset,
	::Ice::Int startPos,
	const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter,
	::TianShanIce::ServerError,
	::TianShanIce::InvalidStateOfArt)
{
	WLock wLock(*this);
	envlog(ZQ::common::Log::L_DEBUG, CLOGFMT(VLCStreamImpl, ".......seekToPosition() Entry [%s]........"), ident.name.c_str());
	if (skipToItem(UserCtrlNum, false, c))
	{
		// add by zjm
		VLCPlayItemList::iterator iter = find_if(_pl._inputItem.begin(), _pl._inputItem.end(), FindByPLCtrlNum(UserCtrlNum));
		::Ice::Float itemPercent = timeOffset;
		itemPercent = (itemPercent / iter->_length) * 100;
		std::stringstream ssItemPercent;
		ssItemPercent << itemPercent;
		std::string strItemPercent = ssItemPercent.str();

		std::string strMsg;

		if (checkClientSession() == false)
		{
			ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"VLCStreamImpl",500,"seekToPosition() Sess(%s) fail to find telnet client session", ident.name.c_str());
			return false;
		}

		bool b = _env._vlcTelnetSessionPool.ControlVLC(VLCCONTROL, sessKey, _pl, strVLCPlay, strItemPercent, strNull, strMsg, pConfig._telnetProp.timeOut);
		//bool b = _vlcClientSession->controlPL(_pl, strVLCPlay, ss.str(), strNull, strMsg);
		//_env._vlcTelnetSessionPool.deActiveTelnet(_vlcClientSession->_pTelnetClient);
		DWORD sTickTime = GetTickCount();
		if (b == false)
		{
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(VLCStreamImpl, "skipToItem() seekToPosition(%s) fail to play the specified item(CtrlNum:%d) pause"), ident.name.c_str(), UserCtrlNum);
			return false;
		}
		else
		{
			_isPlayed = true;
			long timeout = 0;
			for (VLCPlayItemList::iterator iter = _pl._inputItem.begin(); iter != _pl._inputItem.end(); iter++)
			{
				timeout += iter->_length;
			}
			timeout -= timeOffset;
			_env._streamWatchDog->watch(ident, timeout - (GetTickCount() - sTickTime));
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(VLCStreamImpl, "skipToItem() seekToPosition(%s) success seek to specified item(CtrlNum:%d) pause"), ident.name.c_str(), UserCtrlNum);
			return true;
		}
	}
	else
	{
		ZQTianShan::_IceThrow<::TianShanIce::InvalidParameter> (envlog,"VLCStreamImpl",500,"seekToPosition() Sess(%s) fail to seek", ident.name.c_str());
		return false;
	}
}

void VLCStreamImpl::
	enableEoT(bool enable,
	const ::Ice::Current& c)
	throw (::TianShanIce::ServerError,
	::TianShanIce::InvalidStateOfArt)
{
	WLock wLock(*this);
}

::TianShanIce::Streamer::StreamInfo VLCStreamImpl::
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
	WLock wLock(*this);
	::TianShanIce::Streamer::StreamInfo streamInfo;
	seekToPosition(UserCtrlNum, timeOffset, startPos, c);
	return streamInfo;
}

::TianShanIce::Properties VLCStreamImpl::getProperties(const ::Ice::Current&) const
{
	RLock rLock(*this);
	return props;
}

void VLCStreamImpl::setProperties(const ::TianShanIce::Properties&, const ::Ice::Current&)
{
	WLock wLock(*this);
}

bool VLCStreamImpl::setItemInfo(::TianShanIce::Streamer::VLCStreamServer::VLCItem &item, ::Ice::Int userCtrlNum, const ::TianShanIce::Streamer::PlaylistItemSetupInfo& newItemInfo, ::Ice::Int whereUserCtrlNum)
{
	//record item info
	item.name				= newItemInfo.contentName;
	try
	{
		::TianShanIce::Storage::ContentPrx contentPrx = _env._contentStoreExPrx->openContentByFullname(newItemInfo.contentName);
		::TianShanIce::Storage::UnivContentPrx univContentPrx = ::TianShanIce::Storage::UnivContentPrx::uncheckedCast(contentPrx);
		item.path = univContentPrx->getMainFilePathname();
		item.timeLen = univContentPrx->getPlayTimeEx();
	}
	catch (::Ice::Exception &ex)
	{
		//TODO: log error
		envlog(::ZQ::common::Log::L_ERROR,CLOGFMT(VLCStreamImpl, "setItemInfo() fail to get content(name:%s) full path, exception message(%s)"), newItemInfo.contentName.c_str(), ex.ice_name().c_str());
		return false;
	}
	catch(...)
	{
		envlog(::ZQ::common::Log::L_ERROR,CLOGFMT(VLCStreamImpl, "setItemInfo() fail to get content(name:%s) full path, unknown exception"), newItemInfo.contentName.c_str());
		return false;
	}
	
	item.criticalStart		= newItemInfo.criticalStart;
	item.userCtrlNum		= userCtrlNum;
	item.forceNormal		= newItemInfo.forceNormal;
	item.inTimeOffset		= (uint32)newItemInfo.inTimeOffset;
	item.outTimeOffset		= (uint32)newItemInfo.outTimeOffset;
	item.spliceIn			= newItemInfo.spliceIn;
	item.spliceOut			= newItemInfo.spliceOut;
	item.whereUserCtrlNum	= whereUserCtrlNum;
	return true;
}

void VLCStreamImpl::OnTimer(const Ice::Current& cur)
{
	::TianShanIce::Streamer::StreamState curState = getCurrentState(cur);
	WLock wLock(*this);
	if (::TianShanIce::Streamer::stsStop == curState && !_isEnded)
	{
		_isEnded = true;
		listmem params;
		params.type = E_PLAYLIST_END;
		params.param[EventField_PlaylistGuid] = ident.name;
		params.param[EventField_EventCSEQ] = InterlockedIncrement(&_env._eventSequence); // lock problem
		_env._VLCVSSEventSink._paramsList.PushBack(params);
		envlog(ZQ::common::Log::L_NOTICE, CLOGFMT(VLCStreamImpl, "OnTimer() play list[%s] reach end.Sink E_PLAYLIST_END event"), ident.name.c_str());

	}
	else if (::TianShanIce::Streamer::stsStreaming == curState)
	{
		_env._streamWatchDog->watch(ident, 1000); // Îó²î,µÝÔö
	}
}

}//namespace VLC
}//namespace VSS
}//namespace ZQTianShan