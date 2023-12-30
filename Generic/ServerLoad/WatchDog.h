#ifndef __WatchDog_H__
#define __WatchDog_H__

// stl
#include <string>
#include <map>
#include <list>
#pragma warning(disable: 4503)
#include <vector>

// zq common and tianshan common
#include <NativeThread.h>
#include <NativeThreadPool.h>
#include <Locks.h>

#include <windows.h>

namespace SrvrLoad
{
	class SrvrLoadEnv;
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
		void setExpiration(const __int64& expiration);
		std::string getIdent() const;
		__int64 getExpiration() const;
		bool isFullEqual(const WatchObject& watchObj) const;
		
	protected: 
		std::string _ident;
		__int64 _expiration;
		
	}; // class WatchObject	
	
	//////////////////////////////////////////////////////////////////////////
	// class WatchDog
	//////////////////////////////////////////////////////////////////////////
	
	class WatchDog : public ZQ::common::NativeThread
	{
	public: 
		WatchDog(SrvrLoadEnv& env);
		virtual ~WatchDog();
		void watch(std::string ident, __int64 milliSec);
		void unwatch(std::string ident);
		void onTimer(const std::vector<std::string>& idents);
		int getWatchSize();

	public: 
		virtual void stop();

	protected: 
		virtual bool init(void);
		virtual int run();
		virtual void final(void);

		void buildList(const __int64& tmNow);
		void preSort(std::vector<WatchObject>& wos, const int& inFrom, const int& inLen, int& nSmallest);
		void QuickSort(std::vector<WatchObject>& wos, const int& inFrom, const int& inLen);
		
	protected: 
		SrvrLoadEnv& _env;
		std::list<WatchObject> _watchList;
		std::map<std::string, __int64> _watchMap;
		ZQ::common::Mutex _lock;
		HANDLE _event;
		bool _bExit;
		__int64 _nextWakeup;
		
	}; // class WatchDog

} // namespace SrvrLoad

#endif // __WatchDog_H__

