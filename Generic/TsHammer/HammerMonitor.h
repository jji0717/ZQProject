#ifndef __HAMMER_MONITOR__
#define __HAMMER_MONITOR__

#include "NativeThread.h"
#include "NativeThreadPool.h"
#include "SystemUtils.h"
#include "Hammer.h"

class HammerMonitor : public ZQ::common::NativeThread {

public:

	HammerMonitor(ZQ::common::Log& log, ZQ::common::NativeThreadPool& thrdPool, Hammer&);
	virtual ~HammerMonitor();

public:

	void watch(size_t seqIdx, int64 timeout=0);
	
	void quit();

	size_t numWatching() const {
		ZQ::common::MutexGuard gd(_lock);
		return _expires.size();
	}
	void rmWatch(size_t idx) {
		ZQ::common::MutexGuard gd(_lock);
		_expires.erase(idx);
	}
	
public: 
	
	virtual int run();
	
	void wakeup();
	
public:

	typedef std::map <size_t, int64> ExpireSessions; 

public:

	ZQ::common::Log& _log;
	ZQ::common::NativeThreadPool& _pool;
	
	ExpireSessions _expires;
	Hammer& _hammer;

	int64 _nextWakeup;
	bool _quit;

	ZQ::common::Mutex _lock;
	SYS::SingleObject _handle;
};

class TimeoutCmd : public ZQ::common::ThreadRequest {

public:

    TimeoutCmd(HammerMonitor& watchdog, Hammer& hammer, uint seqIdx);
	virtual ~TimeoutCmd() {}

public: 

	virtual bool init()	{ return true; };
	virtual int run();
	
	virtual void final(int retcode=0, bool bCancelled=false) { delete this; }

private:
	Hammer& _hammer;
	uint _seq;
};



#endif

// vim: ts=4 sw=4 nu

