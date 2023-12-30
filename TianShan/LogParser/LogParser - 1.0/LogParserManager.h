#ifndef __LOG_PARSER_MANAGER_H_ 
#define __LOG_PARSER_MANAGER_H_

#include <Log.h>
#include <string>
#include <map>

#include "EventSink.h"
#include "NativeThreadPool.h"
#include "LiteralFunc.h"
#include "MessageSource.h"
#include "LogMessageHandler.h"
#include <Locks.h>
#include "SystemUtils.h"

#ifdef ZQ_OS_LINUX
extern "C"
{
#include <semaphore.h>
}
#endif

class MessageSenderPump;
class LogParserManager : public LogMessageHandler, public RecoverPointCache, public ZQ::common::NativeThread
{
public:
	LogParserManager(ZQ::common::Log& log,ZQ::common::NativeThreadPool& thdPool, int idleTime = 200, int busyTime = 1000);
	virtual ~LogParserManager();

	struct LoggerInfo
	{
		std::string logFile;
		std::string logType;
		std::string syntaxFile;
		std::string syntaxKey;
		typedef std::map<std::string, std::string> Properties;
		Properties ctx;
	};

	bool init(Ice::CommunicatorPtr comm,const std::string& posDbPath,int posDbEvictorSize);
	bool addMonitoring(const LoggerInfo& logger);
	void removeMonitoring(const LoggerInfo& logger);
	bool findMonitoring(const std::string& filepath);

private:
	ZQ::common::NativeThreadPool& _thpool;
	ZQ::common::Log& _log;
	MessageSenderPump* _pSenderPump;
	LogPositionDb* _posDb;
	MessageSourceFactory* _pMsgSrcFac;
	Literal::Library _lib;

	typedef std::map<std::string,IRawMessageSource*> LogMonitorData;
	LogMonitorData _logMonitorData;
	ZQ::common::Mutex _lockLogMonitors;

public:
	IRawMessageSource* getMessageSource(const std::string& filepath);
	ZQ::common::NativeThreadPool& getThreadPool();
	ZQ::common::Log& getLog();

//watchdog: monitor file expired
public:
	virtual int run();
	bool add(const std::string& filepath,int64 expiredTime);
	void remove(std::string filepath);
private:
	ZQ::common::Mutex _logfilesLock;
	std::map<std::string,int64> _logfiles;
	int64 _timeout;
	bool _Quit;

	SYS::SingleObject _sysWakeUp;

};
#endif