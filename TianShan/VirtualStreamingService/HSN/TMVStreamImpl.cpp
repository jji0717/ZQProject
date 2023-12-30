#include <winsock2.h>
#include "TMVStreamImpl.h"
#include "TMVSSCommitThrd.h"
#include <iostream>
using namespace std;

namespace ZQTianShan{
namespace VSS{
namespace TM{

static std::string strCategory = "TMVSS";
/***************
//TMVStreamImpl
****************/
TMVStreamImpl::TMVStreamImpl(TMVSSEnv &env):
_env(env),
_logFile(env._logFile),
_pool(env._thpool)
{
	if (sessKey.empty())
		_soapClientSession = NULL;
	else
	{
		checkSoapClientSession();
	}
}

TMVStreamImpl::~TMVStreamImpl()
{
	_soapClientSession = NULL;
}

void TMVStreamImpl::renewPathTicket(const ::Ice::Current& c)
{
	int32 iTime = GetTickCount();
	try
	{
		::TianShanIce::Transport::PathTicketPrx& _pathTicketPrx = ::TianShanIce::Transport::PathTicketPrx::uncheckedCast(_env._communicator->stringToProxy(pathTicketStr));
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

bool TMVStreamImpl::checkSoapClientSession()
{
	if (_soapClientSession != NULL)
		return true;
	else if (sessId.empty())
	{
		envlog(::ZQ::common::Log::L_ERROR, CLOGFMT(TMVStreamImpl,"checkSoapClientSession: session key is null"));
		return false;
	}
	else
	{
		//try to find in map
		_soapClientSession = const_cast<TMVSoapClientSession *>(_env._pTMVSSSoapServer->_soapClientMap.getSoapClient(sessId));
		if (_soapClientSession == NULL)
		{
			envlog(::ZQ::common::Log::L_ERROR, CLOGFMT(TMVStreamImpl,"checkSoapClientSession: stream(%s) can't use key(%s) to find session"), ident.name.c_str(), sessId.c_str());
			return false;
		}
		else
		{
			envlog(::ZQ::common::Log::L_DEBUG, CLOGFMT(TMVStreamImpl,"checkSoapClientSession: stream(%s) use key(%s) to find session ok"), ident.name.c_str(), sessId.c_str());
			return true;
		}
	}
}

//impl of TMVSS
::std::string TMVStreamImpl::
	getOnDemandSessionId(const ::Ice::Current& c) const
{
	RLock sync(*this);
	//IceUtil::Mutex::Lock lock(*this);
	return ident.name;
}

::std::string TMVStreamImpl::
	getSessionId(const ::Ice::Current& c) const
{
	RLock sync(*this);
	//IceUtil::Mutex::Lock lock(*this);
	return sessId;
}

::std::string TMVStreamImpl::
	getCtrlURL(const ::Ice::Current& c) const
{
	//IceUtil::Mutex::Lock lock(*this);
	RLock sync(*this);
	return controlURl;
}

//impl of stream
void TMVStreamImpl::
	allotPathTicket(const ::TianShanIce::Transport::PathTicketPrx& ticket,
	const ::Ice::Current& c)
	throw (::TianShanIce::Transport::ExpiredTicket,
	::TianShanIce::ServerError,
	::TianShanIce::NotSupported)
{
}

void TMVStreamImpl::
	destroy(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError)
{
	WLock sync(*this);
	if (checkSoapClientSession() == false)
	{
		//throw exception
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(TMVStreamImpl, "TMVStreamImpl::session(ident:%s, sessId:%s) destroy() fail to find exist soapClientSession"), ident.name.c_str(), sessId.c_str());
		_env._eTMVStream->remove(ident);
		return;
	}
	int ret = 0;
	DWORD sTime = GetTickCount();
	if (_soapClientSession->soapTeardown(sessId, ret) == false)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"TMVStreamImpl",143,"TMVStreamImpl::session(ident:%s, sessId:%s) destroy() fail, soap call fail", ident.name.c_str(), sessId.c_str());
	}
	else if (ret != 1)
	{
		TMVSoapClientSession *tmpSess = _env._pTMVSSSoapServer->_soapClientMap.removeSoapClient(sessId);
		_env._eTMVStream->remove(ident);
		delete tmpSess;
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"TMVStreamImpl",143,"TMVStreamImpl::session(ident:%s, sessId:%s) destroy() fail, soap return value(ret=%d) fail", ident.name.c_str(), sessId.c_str(), ret);
	}
	else
	{
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(TMVStreamImpl, "destroy: session(%s) teardown cost %dms"), ident.name.c_str(), GetTickCount() - sTime);
		if (ret)
		{
			//log teardown success
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(TMVStreamImpl, "destroy: session(%s) teardown over"), ident.name.c_str());
		}
		TMVSoapClientSession *tmpSess = _env._pTMVSSSoapServer->_soapClientMap.removeSoapClient(sessId);
		_env._eTMVStream->remove(ident);
		delete tmpSess;
	}
}

void TMVStreamImpl::
	commit_async(const ::TianShanIce::Streamer::AMD_Stream_commitPtr& amdStream, 
	const ::Ice::Current& c)
{	
	TMVSSCommitThrd *commitThrd = new TMVSSCommitThrd(_env._urlStrNotify.generate(), ident.name.c_str(), this, pathTicketStr, amdStream);
	commitThrd->start();
}

::std::string TMVStreamImpl::
	lastError(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return std::string("");
}

::Ice::Identity TMVStreamImpl::
	getIdent(const ::Ice::Current& c) const
{
	RLock sync(*this);
	//IceUtil::Mutex::Lock lock(*this);
	return ident;
}

void TMVStreamImpl::
	setConditionalControl(const ::TianShanIce::Streamer::ConditionalControlMask& mask, 
	const ::TianShanIce::Streamer::ConditionalControlPrx& condCtrl, 
	const ::Ice::Current& c)
{
}

void TMVStreamImpl::play_async(const ::TianShanIce::Streamer::AMD_Stream_playPtr& amd, const ::Ice::Current& c)
{
	RLock sync(*this);
	//TODO: send play message
	amd->ice_response(true);
}

bool TMVStreamImpl::
	setSpeed(::Ice::Float newSpeed,
	const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter, 
	::TianShanIce::ServerError, 
	::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	//TODO: send play message
	return true;
}

bool TMVStreamImpl::
	pause(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError, 
	::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	return true;
}

bool TMVStreamImpl::
	resume(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError, 
	::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	return true;
}

::TianShanIce::Streamer::StreamState TMVStreamImpl::
	getCurrentState(const ::Ice::Current& c) const
	throw (::TianShanIce::ServerError)
{
	RLock sync(*this);
	TMVSoapClientSession  *tmpSess = const_cast<TMVSoapClientSession *>(_soapClientSession);
	if (tmpSess == NULL)
	{
		tmpSess = _env._pTMVSSSoapServer->_soapClientMap.getSoapClient(const_cast<std::string &>(sessId));
		if (tmpSess == NULL)
		{
			ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"TMVStreamImpl",143,"TMVStreamImpl::session(ident:%s, sessId:%s) getCurrentState() fail to find exist soapClientSession", ident.name.c_str(), sessId.c_str());
			return ::TianShanIce::Streamer::stsStop;
		}
	}
	int ret = 0;
	DWORD sTime = GetTickCount();
	ZQ2__getStatusResponse responseInfo;
	if (tmpSess->soapGetStatus(sessId, responseInfo) == false)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"TMVStreamImpl",143,"TMVStreamImpl::session(ident:%s, sessId:%s) getCurrentState() fail, soap call fail", ident.name.c_str(), sessId.c_str());
		return ::TianShanIce::Streamer::stsStop;
	}
	else if (responseInfo.ret != 1)
	{
		ZQTianShan::_IceThrow<::TianShanIce::ServerError> (envlog,"TMVStreamImpl",143,"TMVStreamImpl::session(ident:%s, sessId:%s) getCurrentState() fail, soap return value(ret=%d) fail", ident.name.c_str(), sessId.c_str(), responseInfo.ret);
		return ::TianShanIce::Streamer::stsStop;
	}
	else
	{
		envlog(ZQ::common::Log::L_INFO, CLOGFMT(TMVStreamImpl, "getCurrentState: session(%s) cost %dms"), ident.name.c_str(), GetTickCount() - sTime);

		cout << "lastEror: " << responseInfo.lastError << endl;
		cout << "ret: " << responseInfo.ret << endl;
		cout << "stat: " << responseInfo.state << endl;
		cout << "uptime: " << responseInfo.upTime << endl;
		//TODO: parse response
		if (ret != 1)
		{
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(TMVStreamImpl, "getCurrentState: session(%s) return value=%d"), ident.name.c_str(), responseInfo.ret);
			return ::TianShanIce::Streamer::stsStop;
		}
		else
		{
			envlog(ZQ::common::Log::L_INFO, CLOGFMT(TMVStreamImpl, "getCurrentState: session(%s) state=%s"), ident.name.c_str(), responseInfo.state.c_str());
			if (responseInfo.state.compare("RUNNING") == 0)
				return ::TianShanIce::Streamer::stsStreaming;
			else if (responseInfo.state.compare("STOP") == 0)
				return ::TianShanIce::Streamer::stsStop;
			else
				return ::TianShanIce::Streamer::stsStop;
		}
	}
}

::TianShanIce::SRM::SessionPrx TMVStreamImpl::
	getSession(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError)
{
	return NULL;
	//TODO
}

void TMVStreamImpl::
	setMuxRate(::Ice::Int nowRate, 
	::Ice::Int maxRate,
	::Ice::Int minRate,
	const ::Ice::Current& c)
{
}

bool TMVStreamImpl::
	allocDVBCResource(::Ice::Int serviceGroupID,
	::Ice::Int bandWidth,
	const ::Ice::Current& c)
	throw (::TianShanIce::ServerError)
{
	return true;
}

void TMVStreamImpl::
	seekStream_async(const ::TianShanIce::Streamer::AMD_Stream_seekStreamPtr& amd,
	::Ice::Long offset,
	::Ice::Int startPos,
	const ::Ice::Current& c)
	throw (::TianShanIce::ServerError,
	::TianShanIce::InvalidParameter)
{
	amd->ice_response(0);
}

void TMVStreamImpl::
	playEx_async(const ::TianShanIce::Streamer::AMD_Stream_playExPtr& amd,
	::Ice::Float newSpeed, 
	::Ice::Long timeOffset,
	::Ice::Short from,
	const ::TianShanIce::StrValues& expectedProps,
	const ::Ice::Current& c)
	throw (::TianShanIce::ServerError, 
	::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	::TianShanIce::Streamer::StreamInfo streamInfo;
	amd->ice_response(streamInfo);
}

::TianShanIce::Streamer::StreamInfo TMVStreamImpl::
	pauseEx(const ::TianShanIce::StrValues& expectedProps, 
	const ::Ice::Current& c)
	throw (::TianShanIce::ServerError, 
	::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	::TianShanIce::Streamer::StreamInfo streamInfo;
	return streamInfo;
}

void TMVStreamImpl::
	setSpeedEx(::Ice::Float newSpeed,
	::Ice::Long& timeOffset,
	::Ice::Float& currentSpeed,
	const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter,
	::TianShanIce::ServerError,
	::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
}

//impl of playlist
::std::string TMVStreamImpl::
	getId(const ::Ice::Current& c) const
{
	RLock sync(*this);
	//IceUtil::Mutex::Lock lock(*this);
	return ident.name;
}

bool TMVStreamImpl::
	getInfo(::Ice::Int mask,
	::TianShanIce::ValueMap& varOut,
	const ::Ice::Current& c)
{
	RLock sync(*this);
	return true;
}

::Ice::Int TMVStreamImpl::
	insert(::Ice::Int userCtrlNum, 
	const ::TianShanIce::Streamer::PlaylistItemSetupInfo& newItemInfo,
	::Ice::Int whereUserCtrlNum,
	const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter, 
	::TianShanIce::ServerError, 
	::TianShanIce::InvalidStateOfArt)
{
	WLock wLock(*this);
	return 1;
}


::Ice::Int TMVStreamImpl::
	pushBack(::Ice::Int userCtrlNum,
	const ::TianShanIce::Streamer::PlaylistItemSetupInfo& newItemInfo,
	const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter,
	::TianShanIce::ServerError,
	::TianShanIce::InvalidStateOfArt)
{
	WLock wLock(*this);
	return 1;
}

::Ice::Int TMVStreamImpl::
	size(const ::Ice::Current& c) const
{
	RLock sync(*this);
	return 1;
}

::Ice::Int TMVStreamImpl::
	left(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	return 1;
}

bool TMVStreamImpl::
	empty(const ::Ice::Current& c) const
	throw (::TianShanIce::ServerError,
	::TianShanIce::InvalidStateOfArt)
{
	RLock sync(*this);
	return true;
}

::Ice::Int TMVStreamImpl::
	current(const ::Ice::Current& c) const
	throw (::TianShanIce::ServerError,
	::TianShanIce::InvalidStateOfArt)
{
	RLock rLock(*this);
	return 1;
}

void TMVStreamImpl::
	erase(::Ice::Int whereUserCtrlNum,
	const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter,
	::TianShanIce::ServerError,
	::TianShanIce::InvalidStateOfArt)
{
	RLock rLock(*this);
}

::Ice::Int TMVStreamImpl::
	flushExpired(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError,
	::TianShanIce::InvalidStateOfArt)
{
	RLock rLock(*this);
	return 1;
}

bool TMVStreamImpl::
	clearPending(bool includeInitedNext,
	const ::Ice::Current& c)
	throw (::TianShanIce::ServerError,
	::TianShanIce::InvalidStateOfArt)
{
	RLock rLock(*this);
	return true;
}

bool TMVStreamImpl::
	isCompleted(const ::Ice::Current& c)
	throw (::TianShanIce::ServerError,
	::TianShanIce::InvalidStateOfArt)
{
	RLock rLock(*this);
	return true;
}

::TianShanIce::IValues TMVStreamImpl::
	getSequence(const ::Ice::Current& c) const
{
	RLock rLock(*this);
	::TianShanIce::IValues tmpValues;
	return tmpValues;
}

::Ice::Int TMVStreamImpl::
	findItem(::Ice::Int userCtrlNum,
	::Ice::Int from,
	const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter,
	::TianShanIce::ServerError,
	::TianShanIce::InvalidStateOfArt)
{
	RLock rLock(*this);
	return 0;
}

bool TMVStreamImpl::
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

void TMVStreamImpl::
	skipToItem_async(const ::TianShanIce::Streamer::AMD_Playlist_skipToItemPtr& amd,
	::Ice::Int where, 
	bool bPlay,
	const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter, 
	::TianShanIce::ServerError,
	::TianShanIce::InvalidStateOfArt)
{
	WLock wLock(*this);
	amd->ice_response(true);
}

void TMVStreamImpl::
	seekToPosition_async(const ::TianShanIce::Streamer::AMD_Playlist_seekToPositionPtr& amd,
	::Ice::Int UserCtrlNum,
	::Ice::Int timeOffset,
	::Ice::Int startPos,
	const ::Ice::Current& c)
	throw (::TianShanIce::InvalidParameter,
	::TianShanIce::ServerError,
	::TianShanIce::InvalidStateOfArt)
{
	WLock wLock(*this);
	amd->ice_response(true);
}

void TMVStreamImpl::
	enableEoT(bool enable,
	const ::Ice::Current& c)
	throw (::TianShanIce::ServerError,
	::TianShanIce::InvalidStateOfArt)
{
	WLock wLock(*this);
}

void TMVStreamImpl::
	playItem_async(const ::TianShanIce::Streamer::AMD_Playlist_playItemPtr& amd,
	::Ice::Int UserCtrlNum,
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
	amd->ice_response(streamInfo);
}

::TianShanIce::Properties TMVStreamImpl::getProperties(const ::Ice::Current&) const
{
	RLock rLock(*this);
	return props;
}

void TMVStreamImpl::setProperties(const ::TianShanIce::Properties&, const ::Ice::Current&)
{
	WLock wLock(*this);
}

}//namespace TM
}//namespace VSS
}//namespace ZQTianShan