#include "ContextImpl.h"
#include "ssmNGODr2c1.h"
#include "SessionRenewCmd.h"

#include <time.h>

#ifdef _DEBUG
	#include <iostream>
	using namespace std;
#endif

namespace NGODr2c1
{

	ContextImpl::ContextImpl(ssmNGODr2c1& env) : _env(env)
	{
	#ifdef _DEBUG
		cout<<"create context"<<endl;
	#endif
	}

	ContextImpl::~ContextImpl()
	{
	#ifdef _DEBUG
		cout<<"delete context"<<endl;
	#endif
	}

	::NGODr2c1::ctxData ContextImpl::getState(const ::Ice::Current& c)
	{
		IceUtil::Mutex::Lock lk(*this);

		::NGODr2c1::ctxData state;
		
		state.ident = ident;
		state.weiwooFullID = weiwooFullID;
		state.onDemandID = onDemandID;
		state.streamFullID = streamFullID;
		state.streamShortID = streamShortID;
		state.normalURL = normalURL;
		state.resourceURL = resourceURL;
		state.connectID = connectID;
		state.groupID = groupID;
		state.expiration = expiration;
		state.announceSeq = announceSeq;

		return state;
	}

	void ContextImpl::renew(::Ice::Long ttl, const ::Ice::Current& c)
	{
		IceUtil::Mutex::Lock lk(*this);
		expiration = ZQTianShan::now() + ttl;		
	}

	void ContextImpl::increaseAnnSeq(const ::Ice::Current& c)
	{
		IceUtil::Mutex::Lock lk(*this);

		announceSeq++;
	}

	void ContextImpl::onTimer(const ::Ice::Current& c)
	{
		IceUtil::Mutex::Lock lk(*this);

		::Ice::Long timeout = expiration - ZQTianShan::now();
		_env._fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(ContextImpl, "onTimer() Session[%s]; timeout=%lld"), ident.name.c_str(), timeout);

		if (timeout > 0)			
		{			
			_env._pSessionWatchDog->watchSession(ident, timeout);			
			return;			
		}

		// increase announce sequence number
		if (false == _env._config._useGlobalSeq)
			announceSeq++;
		
		if (timeout > -_env._config._timeoutInterval * 1000 * 2) // 注意是负数
		{			
			_env.sessionInProgressAnnounce(this);
			_env._pSessionWatchDog->watchSession(ident, _env._config._timeoutInterval * 1000);
		}		
		else // 没有renew时间太久了，需要销毁并发送Session Terminated announcement
		{		
			_env.terminatAndAnnounce(this); //!! Rename this func to terminateAndAnnounce()			
		}		
	}
}