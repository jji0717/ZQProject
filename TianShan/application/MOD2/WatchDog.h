#ifndef __ZQMODApplication_WatchDog_H__
#define __ZQMODApplication_WatchDog_H__

#pragma warning(disable:4503)

// stl
#include <string>
#include <map>
#include <list>
#include <vector>

// zq common and tianshan common
#include "NativeThread.h"
#include "NativeThreadPool.h"
#include "Locks.h"
#include "TianShanDefines.h"
#include "SystemUtils.h"

namespace ZQMODApplication
{

class ModEnv;	
//////////////////////////////////////////////////////////////////////////
// class WatchException
//////////////////////////////////////////////////////////////////////////
class WatchException
{
public: 
	WatchException(const std::string& msg);
	virtual ~ WatchException();
	std::string getMessage() const;

protected: 
	std::string _msg;
}; // class WatchException


//////////////////////////////////////////////////////////////////////////
// class WatchObject
//////////////////////////////////////////////////////////////////////////
class WatchManager;
class WatchObject
{
	friend class WatchDog;
public: 
	WatchObject();
	WatchObject(const WatchObject& watchObj);
	virtual ~WatchObject();
	virtual bool operator==(const WatchObject& watchObj) const;
	virtual bool operator!=(const WatchObject& watchObj) const;
	virtual bool operator>(const WatchObject& watchObj) const;
	virtual bool operator>=(const WatchObject& watchObj) const;
	virtual bool operator<(const WatchObject& watchObj) const;
	virtual bool operator<=(const WatchObject& watchObj) const;
	virtual void operator=(const WatchObject& watchObj);
	void setIdent(const std::string& ident);
	void setExpiration(const int64& expiration);
	std::string getIdent() const;
	int64 getExpiration() const;
	bool isFullEqual(const WatchObject& watchObj) const;

protected: 
	std::string _ident;
	int64 _expiration;

}; // class WatchObject	


//////////////////////////////////////////////////////////////////////////
// class PurchaseTimeout
//////////////////////////////////////////////////////////////////////////
class PurchaseTimeout : public ZQ::common::ThreadRequest
{
public:
	PurchaseTimeout(ZQ::common::NativeThreadPool& thrdPool, ModEnv& env, const std::string& sessID);
	virtual ~PurchaseTimeout();

protected: // impls of ScheduleTask
	virtual bool init();
	virtual int run(void);
	virtual void final(int retcode =0, bool bCancelled =false);

protected:
	ZQ::common::NativeThreadPool& _thrdPool;
	ModEnv& _env;
	std::string _sessID;

}; // class PurchaseTimeout


//////////////////////////////////////////////////////////////////////////
// class WatchDog
//////////////////////////////////////////////////////////////////////////
class WatchDog : public ZQ::common::NativeThread
{
public: 
	WatchDog(ModEnv& env);
	virtual ~WatchDog();
	void watch(std::string ident, int64 milliSec);
	void unwatch(std::string ident);
	void onTimer(const std::vector<std::string>& idents);
	int getWatchSize();
	void clear();

public: 
	virtual void stop();

protected: 
	virtual bool init(void);
	virtual int run();
	virtual void final(void);

	void buildList(const int64& tmNow);
	void preSort(std::vector<WatchObject>& wos, const int& inFrom, const int& inLen, int& nSmallest);
	void QuickSort(std::vector<WatchObject>& wos, const int& inFrom, const int& inLen);

protected: 
	ModEnv& _env;
	ZQ::common::NativeThreadPool* _pThreadPool;
	std::list<WatchObject> _watchList;
	std::map<std::string, int64> _watchMap;
	ZQ::common::Mutex _lock;
	SYS::SingleObject _event;
	bool _bExit;
	int64 _nextWakeup;

}; // class WatchDog

} // namespace ZQMODApplication

#endif // __ZQMODApplication_WatchDog_H__

