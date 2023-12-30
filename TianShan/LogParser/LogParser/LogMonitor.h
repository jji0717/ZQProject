#ifndef __LogMonitor_H__
#define __LogMonitor_H__


#include <NativeThread.h>
#include <Log.h>
#include <Locks.h>
#include "LogParserManager.h"

#ifdef ZQ_OS_LINUX
extern "C"
{
#include <semaphore.h>
}
#endif


class LogMonitorRequest:public ZQ::common::ThreadRequest
{
public:
	LogMonitorRequest(std::string filepath,LogParserManager& logParserMgr);

	virtual int run();
	void setParsingLoad(int32 idleTime, int32 busyTime);
	virtual void final(int retcode =0, bool bCancelled =false);

private:
	ZQ::common::Log& _log;
	std::string _filepath;
	LogParserManager& _logParserMgr;

	int64 _idleTime;
	int64 _busyTime;
};

/*
class watchDog:public ZQ::common::NativeThread
{
public:
	watchDog(LogParserManager& logParserMgr);
	~watchDog();
	virtual int run();
	void add(const std::string& filepath,int64 expiredTime);
	void remove(std::string filepath);

private:
	ZQ::common::Log& _log;
	ZQ::common::Mutex _logfilesLock;
	std::map<std::string,int64> _logfiles;

	LogParserManager&              _logParserMgr;

#ifdef ZQ_OS_MSWIN
	HANDLE _hQuit;
#else
	sem_t _hQuit;
#endif

};
*/
#endif
