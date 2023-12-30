#ifndef __SessionRenewer_H__
#define __SessionRenewer_H__

#include "../../common/TianShanDefines.h"
#include "ContextImpl.h"
#include "NativeThreadPool.h"
#include <Ice/Ice.h>

class NGODEnv;

class SessionRenewCmd : public ZQ::common::ThreadRequest
{
public:
	/// constructor
	SessionRenewCmd(NGODEnv& env, const std::string& cltSessID, const std::string& srvrSessID, const ::Ice::Long newExpiration, const ::Ice::Long oldExpiration =0);
	virtual ~SessionRenewCmd();

protected: // impls of ThreadRequest
	virtual bool init();
	virtual int run(void);
	virtual void final(int retcode =0, bool bCancelled =false);
	
protected:
	std::string _srvrSessID;
	std::string _cltSessID;
	::Ice::Long	_newExpiration;
	::Ice::Long	_oldExpiration;

    typedef std::map <std::string, SessionRenewCmd*> RequestMap;
	static RequestMap  _sReqMap;
	static ZQ::common::Mutex _sLockReqMap;

	NGODEnv&		_env;

public:
	const static int	_sBasePriority;

};

#endif // __SessionRenewer_H__

