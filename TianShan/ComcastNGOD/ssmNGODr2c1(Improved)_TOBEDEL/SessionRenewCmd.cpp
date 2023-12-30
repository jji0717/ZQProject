#include "SessionRenewCmd.h"
#include "ssmNGODr2c1.h"
#include "ContextImpl.h"
#include "ssmNGODr2c1.h"


SessionRenewCmd::RequestMap SessionRenewCmd::_sReqMap;
ZQ::common::Mutex SessionRenewCmd::_sLockReqMap;
const int SessionRenewCmd::_sBasePriority = DEFAULT_REQUEST_PRIO +1;

SessionRenewCmd::SessionRenewCmd(ssmNGODr2c1& env, const std::string& cltSessID, const std::string& srvrSessID, const ::Ice::Long newExpiration, const ::Ice::Long oldExpiration)
	: _env(env), _newExpiration(newExpiration), _oldExpiration(oldExpiration), _srvrSessID(srvrSessID), _cltSessID(cltSessID)
	, ThreadRequest(*(env._pThreadPool))
{
	int thisPri = _sBasePriority + (int) ((oldExpiration - ZQTianShan::now())/1000);
	if (thisPri < 0)
		thisPri = 0;
	if (thisPri > 255)
		thisPri = 255;
	setPriority(thisPri);

	_env._fileLog(ZQ::common::Log::L_DEBUG, CLOGFMT(SessionRenewCmd, "Sess(%s), WeiwooSession(%s) renew required, priority(%d)"), 
		_cltSessID.c_str(), _srvrSessID.c_str(), thisPri);

	{
		ZQ::common::MutexGuard gd(_sLockReqMap);
		SessionRenewCmd::RequestMap::iterator it = _sReqMap.find(_cltSessID);
		_sReqMap[_cltSessID] = this;
	}
}

SessionRenewCmd::~SessionRenewCmd()
{
	ZQ::common::MutexGuard gd(_sLockReqMap);
	SessionRenewCmd::RequestMap::iterator it = _sReqMap.find(_cltSessID);
	if (_sReqMap.end() != it && this == it->second)
		_sReqMap.erase(it);
}

bool SessionRenewCmd::init()
{
	return (false == _srvrSessID.empty() && NULL != _env._pCommunicator);
}

void SessionRenewCmd::final(int retcode, bool bCancelled)
{
	 delete this;
}

int SessionRenewCmd::run(void)
{
	try
	{
		{
			ZQ::common::MutexGuard gd(_sLockReqMap);
			SessionRenewCmd::RequestMap::iterator it = _sReqMap.find(_cltSessID);
			if (_sReqMap.end() == it  || this != it->second)
				return 1; // already deleted or overwritten by some later renew command
			
			_sReqMap.erase(it);
		}

		::Ice::Long ttl = _newExpiration - ZQTianShan::now();
		if (ttl < 10)
			ttl = 10;
		
		// DO: gain weiwoo session proxy
		::TianShanIce::SRM::SessionPrx srvrSessPrx = NULL;
		try
		{
			srvrSessPrx = ::TianShanIce::SRM::SessionPrx::uncheckedCast(_env._pCommunicator->stringToProxy(_srvrSessID));
			srvrSessPrx->renew(ttl);
		}
		catch (const Ice::Exception& ex)
		{
			_env._fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(SessionRenewCmd, "renew WeiwooSessionPrx(%s) caught(%s)")
				, _srvrSessID.c_str(), ex.ice_name().c_str());
			return 1;
		}
	
		_env._fileLog(ZQ::common::Log::L_INFO, CLOGFMT(SessionRenewCmd, "Sess(%s), WeiwooSession(%s) renewed(%lldms)")
			, _cltSessID.c_str(), _srvrSessID.c_str(), ttl);
	}
	catch(...)
	{
		_env._fileLog(ZQ::common::Log::L_ERROR, CLOGFMT(SessionRenewCmd, "WeiwooSession(%s) renew caught unexpect exception"), _srvrSessID.c_str());
		return 1;
	}
	
	return 0;
}

