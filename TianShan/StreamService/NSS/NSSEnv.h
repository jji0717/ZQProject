#ifndef __ZQTianShan_NSSEnv_H__
#define __ZQTianShan_NSSEnv_H__

#include "TianShanDefines.h"
#include "SsEnvironment.h"
#include "NGODSession.h"

#include "InetAddr.h"
#include "SystemUtils.h"

#ifdef _DEBUG
#  define UNATTEND_TIMEOUT			(20*1000) // 20 sec
#  define DEFAULT_SCH_ERROR_WIN		(60000) // 1 min
#  define MAX_START_DELAY			(60*1000) //1 min
#  define STOP_REMAIN_TIMEOUT		(5*1000) // 5 sec
#  define MIN_PROGRESS_INTERVAL		(10*1000) // 10 sec
#else
#  define UNATTEND_TIMEOUT			(48*60*60*1000) // 48 hours
#  define DEFAULT_SCH_ERROR_WIN		(5000) // 5 sec
#  define MAX_START_DELAY			(5*60*1000) // 5 min
#  define STOP_REMAIN_TIMEOUT		(60*1000) // 1 min
#  define MIN_PROGRESS_INTERVAL		(30*1000) // 30 sec
#endif // _DEBUG

#define MAX_IDLE                    (60*60*1000) // 1hour
#define DEFAULT_IDLE                (5* 60*1000) // 5sec

extern bool bDecimalNpt;

#define C1Require (bDecimalNpt? "com.comcast.ngod.c1.decimal_npts" :"com.comcast.ngod.c1")
#define R2Require (bDecimalNpt? "com.comcast.ngod.r2.decimal_npts" :"com.comcast.ngod.r2")

namespace ZQTianShan {
namespace NGODSS {

class NSSEnv;

// -----------------------------
// class WatchDog
// -----------------------------
class WatchDog : public ZQ::common::NativeThread
{
	friend class NGODSessionGroup;
	typedef SYS::SingleObject Event; // for the stupid naming of SingleObject

public:
	WatchDog(NSSEnv& env);
	virtual ~WatchDog();

public:
	///@param[in] contentIdent identity of the object
	///@param[in] timeout the timeout to wake up timer to check the specified object
	void watch(NGODSessionGroup::Ptr group, ::Ice::Long syncInterval =0);

	//quit watching
	void quit();

protected:

	int		run();
	//used for third party to stop this thread
	int		terminate(int code /* = 0 */);

private:

	NSSEnv&             _env;

	typedef std::multimap <NGODSessionGroup::Ptr, Ice::Long > SyncMap; // sessId to expiration map
	ZQ::common::Mutex   _lockGroups;
	SyncMap				_groupsToSync;
	Event	            _event;
};

// -----------------------------
// class NSSEnv
// -----------------------------
class NSSEnv : public ZQ::StreamService::SsEnvironment
{
	friend class NGODSessionGroup;

public:
	//constructor
	NSSEnv(ZQ::common::Log& mainLog, ZQ::common::Log& sessLog, ZQ::common::NativeThreadPool& rtspThpool, ZQ::common::NativeThreadPool& ssThpool);
	virtual ~NSSEnv();

public:
	void init();
	void uninit();

	void start();

	bool isRunning() const { return _bRun; }

	void setNetId(std::string netId) { _netId = netId; };

public:
	ZQ::common::FileLog   _logger;	
	std::string _netId, _userAgent;

	ZQ::common::InetHostAddress _bindAddr;
	ZQ::common::Log::loglevel_t _rtspTraceLevel;
	ZQ::common::NativeThreadPool& _rtspThpool;


	int32  _sessTimeout; // in msec

protected:

	WatchDog* _watchDog;
	bool      _bRun;
};

}

}

#endif //__ZQTianShan_NSSEnv_H__
