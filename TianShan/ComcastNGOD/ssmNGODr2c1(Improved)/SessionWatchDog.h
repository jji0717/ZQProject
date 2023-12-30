#ifndef __SessionWatchDog_H__
#define __SessionWatchDog_H__

#include "NativeThreadPool.h"
#include "../../common/TianShanDefines.h"

typedef ::std::vector<Ice::Identity> IdentCollection;
typedef ::std::map<::Ice::Identity, ::Ice::Long> ExpirationMap;

class ssmNGODr2c1;

class SessionWatchDog : public ZQ::common::ThreadRequest
{
public:
	/// constructor
    SessionWatchDog(ssmNGODr2c1& env);
	virtual ~SessionWatchDog();

	///@param[in] sessIdent identity of session
	///@param[in] timeout the timeout to wake up timer to check the specified session
	void watchSession(const ::Ice::Identity& sessIdent, long timeout);
	
	//quit watching
	void quit();

protected: // impls of ThreadRequest

	virtual bool init(void);
	virtual int run(void);
	
	// no more overwrite-able
	void final(int retcode =0, bool bCancelled =false);

	void wakeup();

protected:

	typedef std::map <::Ice::Identity, ::Ice::Long > ExpirationMap; // sessId to expiration map
	ZQ::common::Mutex   _lockExpirations;
	ExpirationMap		_expirations;
	::Ice::Long			_nextWakeup;

	ssmNGODr2c1& _env;
	bool		  _bQuit;
	HANDLE		  _hWakeupEvent;
};

#endif // __SessionWatchDog_H__

