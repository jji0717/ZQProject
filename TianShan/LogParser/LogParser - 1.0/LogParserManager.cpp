#include "LogParserManager.h"

#include "SyntaxConfig.h"
#include "LogMessageHandler.h"
#include "TimeConv.h"
// #include <TimeUtil.h>
#include "LiteralFunc.h"
/*#include <FileLog.h>*/
#include <boost/shared_ptr.hpp>
#include "LogMonitor.h"
#include "MessageSenderPump.h"


LogParserManager::LogParserManager(ZQ::common::Log& log,ZQ::common::NativeThreadPool& thdPool, int idelTime, int busyTime)
					:_log(log)
					,LogMessageHandler(log)
					,_posDb(NULL)
					,_pSenderPump(NULL)
					,_pMsgSrcFac(NULL)
					,_thpool(thdPool)
					,_Quit(false)
{

}

LogParserManager::~LogParserManager()
{
	_Quit = true;
	_sysWakeUp.signal();

	_log(ZQ::common::Log::L_INFO, CLOGFMT(LogParserManager, "Stopping the log parsing function..."));
	{
		ZQ::common::MutexGuard sync(_lockLogMonitors);
		for(LogMonitorData::iterator it = _logMonitorData.begin(); it != _logMonitorData.end(); ++it)
		{
			delete it->second;
		}
		_logMonitorData.clear();
	}
	if(_pMsgSrcFac != NULL)
	{
		delete _pMsgSrcFac; 
		_pMsgSrcFac = NULL; 
	}

	if(_pSenderPump != NULL)
	{
		delete _pSenderPump; 
		_pSenderPump = NULL; 
	}

	_posDb->uninit();
	if(_posDb != NULL)
	{
		delete _posDb; 
		_posDb = NULL; 
	}

	_log(ZQ::common::Log::L_INFO, CLOGFMT(LogParserManager, "The log parsing function is disabled."));
}

bool LogParserManager::init(Ice::CommunicatorPtr comm,const std::string& posDbPath,int posDbEvictorSize)
{

	// init the literal function lib
	_lib.clear();
	_lib["FILELOGTIME2UTC"] = Literal::fileLogTimeToUTC;
	_lib["SCLOGTIME2UTC"] = Literal::scLogTimeToUTC;
	_lib["ISOTIME2LOCALTIME"] = Literal::isoTimeToLoaclNomal;
#ifdef ZQ_OS_LINUX
	_lib["SYSLOGTIME2UTC"] = Literal::sysLogTimeToUTC;
	_lib["SYSNORMALTIME2UTC"] = Literal::sysNormalTimeToUTC;
#endif

	// init the message sender pump
	_pSenderPump = new MessageSenderPump(_log);
	if(_pSenderPump == NULL || _pSenderPump->query().empty()) {
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(LogParserManager, "Failed to init MessageSenderPump."));
		return false;
	}

	// init the position db
	_posDb = new LogPositionDb(_log);
	if((_posDb == NULL) || !_posDb->init(comm,posDbPath,posDbEvictorSize)) {
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(LogParserManager, "Failed to init the PositionDb at %s with evictor"),posDbPath.c_str());
		return false;
	}
	// init the message source factory
	_pMsgSrcFac = new MessageSourceFactory(_log);

	_log(ZQ::common::Log::L_INFO, CLOGFMT(LogParserManager, "The log parsing function is OK."));

	return true;
}

bool LogParserManager::addMonitoring(const LoggerInfo& logger)
{
	using namespace ZQ::common;
	_log(Log::L_INFO, CLOGFMT(LogParserManager, "Enter addMonitoring with [log=%s], [type=%s], [syntax=%s], [key=%s]"), logger.logFile.c_str(), logger.logType.c_str(), logger.syntaxFile.c_str(), logger.syntaxKey.c_str());

	if(logger.logFile.empty() || logger.syntaxFile.empty() || logger.syntaxKey.empty())
	{
		_log(Log::L_ERROR, CLOGFMT(LogParserManager, "addMonitoring() Bad logger info. file=%s"), logger.logFile.c_str());
		return false;
	}
	if(!logger.logType.empty() && !_pMsgSrcFac->checkType(logger.logType))
	{
		_log(Log::L_ERROR, CLOGFMT(LogParserManager, "addMonitoring() Bad log type. type=%s"), logger.logType.c_str());
		return false;
	}
	
	if (findMonitoring(logger.logFile))
	{
			_log(Log::L_WARNING, CLOGFMT(LogParserManager, "addMonitoring() Logger is under monitoring. file=%s"), logger.logFile.c_str());
			return false;
	}

	// step 1: parse the syntax file and get the syntax definition
	ZQ::common::Config::Loader<SyntaxDefConf> syntaxDef("");
	syntaxDef.setLogger(&_log);
	// setup the context
/*	{
		// global context
		std::map<std::string, std::string>::const_iterator it;
		for(it = _pSinkConf->contextProps.begin(); it != _pSinkConf->contextProps.end(); ++it)
		{
			syntaxDef.PP().define(it->first, it->second);
		}
		// specific context
		for(it = logger.ctx.begin(); it != logger.ctx.end(); ++it)
		{
			syntaxDef.PP().define(it->first, it->second);
		}
	}*/
	if(!syntaxDef.load(logger.syntaxFile.c_str()))
	{
		_log(Log::L_ERROR, CLOGFMT(LogParserManager, "addMonitoring() Failed to load syntax definition file. file=%s"), logger.syntaxFile.c_str());
		return false;
	}

	if(syntaxDef.syntaxMap.end() == syntaxDef.syntaxMap.find(logger.syntaxKey))
	{ // no syntax found
		_log(Log::L_ERROR, CLOGFMT(LogParserManager, "addMonitoring() Can't find syntax definition. key=%s"), logger.syntaxKey.c_str());
		return false;
	}

	SyntaxConf syntax = syntaxDef.syntaxMap[logger.syntaxKey];
	if(!logger.logType.empty() && MessageSourceFactory::parseType(logger.logType).first != MessageSourceFactory::parseType(syntax.evntSrcType).first)
	{ // logger type not fit
		_log(Log::L_ERROR, CLOGFMT(LogParserManager, "addMonitoring() Logger type [%s] conflict with actual log type [%s] in the syntx definition"), logger.logType.c_str(), syntax.evntSrcType.c_str());
		return false;
	}

	MessageIdentity recoverPoint;
    bool rpGot = false; // if recover point got from cache
    if(getCache(recoverPoint, logger.logFile)) {
        rpGot = true;
    } else {
        rpGot = false;
        recoverPoint.source = logger.logFile;
        recoverPoint.position = -1;
        recoverPoint.stamp = -1;
    }
    EventSender::Handlers handlers;
	MessageSenderPump::vecMsgSender senders = _pSenderPump->query();

    for(size_t iSender = 0; iSender < senders.size(); ++iSender) {
        PositionRecordPtr record = _posDb->getPosition(logger.logFile, senders[iSender].strType);
        if(!record) {
            _log(Log::L_ERROR, CLOGFMT(LogParserManager, "addMonitoring() Failed to get the position of handler(%s), file(%s)"), senders[iSender].strType.c_str(), logger.logFile.c_str());
            continue;
        }
        /*
        if(ZQ::common::now() - record->lastUpdatedAt() > DidcardLimit) {
            record->set(0, 0);
        }
        */
        if(!rpGot) { // need calculate the recover point from db records
            int64 position = 0;
            int64 stamp = 0;
            record->get(position, stamp);
            if((recoverPoint.stamp == -1 && recoverPoint.position == -1) ||
               (recoverPoint.stamp > stamp) ||
               (recoverPoint.stamp == stamp && recoverPoint.position > position)) {
                recoverPoint.stamp = stamp;
                recoverPoint.position = position;
            }
        }

        EventSender::Handler handler;
        handler.type = senders[iSender].strType;
        handler.onMessage = senders[iSender].handle;
        handler.pos = record;
        handlers.push_back(handler);
    }
    if(!rpGot) { // update the recover point cache
        if(recoverPoint.stamp < 0 || recoverPoint.position < 0) {
            recoverPoint.stamp = 0;
            recoverPoint.position = 0;
        }
        setCache(recoverPoint);
    }

	// step 2: add rule
	addRules(logger.syntaxKey,syntax.rules, handlers, _lib);

	// step 3: create the message source base on the logger info
	std::string msgSrcType = (!logger.logType.empty() ? logger.logType : syntax.evntSrcType);
	IRawMessageSource* msgSrc = _pMsgSrcFac->create(logger.logFile, msgSrcType);
	if(NULL == msgSrc)
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(LogParserManager, "addMonitoring() Failed to create message sourcing object. Give up the monitoring. file=%s, type=%s"), logger.logFile.c_str(), msgSrcType.c_str());
		removeRules(logger.syntaxKey);
		return false;
	}
	msgSrc->SetKey(logger.syntaxKey);
	msgSrc->SetMessageIdentity(recoverPoint);
	// step 4: add the log monitor
	{
		ZQ::common::MutexGuard sync(_lockLogMonitors);
		_logMonitorData[logger.logFile] = msgSrc;
	}

	if (!add(logger.logFile,ZQ::common::now()))
	{
		return false;
	}
	std::string stampStr = time2utc(recoverPoint.stamp);
	printf("--------stamp = %lld\n",recoverPoint.stamp);
	_log(ZQ::common::Log::L_INFO, CLOGFMT(LogParserManager, "addMonitoring() Successful to monitor the log file. file = %s,stamp = %s,position = %llu."), logger.logFile.c_str(),stampStr.c_str(),recoverPoint.position);
	return true;
}

void LogParserManager::removeMonitoring(const LoggerInfo& logger)
{
	using namespace ZQ::common;
	_log(Log::L_INFO, CLOGFMT(LogParserManager, "Enter removeMonitoring with [log=%s]"), logger.logFile.c_str());
	if(logger.logFile.empty())
	{
		_log(Log::L_WARNING, CLOGFMT(LogParserManager, "removeMonitoring() Bad logger info. file=%s"), logger.logFile.c_str());
		return;
	}

	{
		ZQ::common::MutexGuard sync(_lockLogMonitors);
		LogMonitorData::iterator it = _logMonitorData.find(logger.logFile);
		if(_logMonitorData.end() == it)
		{ // not under monitoring?
			_log(Log::L_WARNING, CLOGFMT(LogParserManager, "removeMonitoring() Log not being monitored. file=%s]"), logger.logFile.c_str());
			return;
		}
		_pMsgSrcFac->destroy(it->second);
		_logMonitorData.erase(it);
	}

	remove(logger.logFile);
	_log(Log::L_INFO, CLOGFMT(LogParserManager, "removeMonitoring() Successfully to remove the monitoring. file=%s"), logger.logFile.c_str());
}

bool LogParserManager::findMonitoring(const std::string& filepath)
{
	ZQ::common::MutexGuard sync(_lockLogMonitors);
	LogMonitorData::iterator it = _logMonitorData.find(filepath);
	if(_logMonitorData.end() == it)
		return false;
	else
		return true;
}

IRawMessageSource* LogParserManager::getMessageSource(const std::string& filepath)//LogMessageHandler thread safe
{
	ZQ::common::MutexGuard sync(_lockLogMonitors);

	LogMonitorData::const_iterator it = _logMonitorData.find(filepath);
	if(_logMonitorData.end() == it)
	{ // not under monitoring?
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(LogParserManager, "Log not being monitored. file=%s]"),filepath.c_str());
		return NULL;
	}
	return it->second;
}


ZQ::common::NativeThreadPool& LogParserManager::getThreadPool()
{
	return _thpool;
}

ZQ::common::Log& LogParserManager::getLog()
{
	return _log;
}

bool LogParserManager::add(const std::string& filepath,int64 expiredTime)
{
	if (!findMonitoring(filepath))
	{
		_log(ZQ::common::Log::L_WARNING, CLOGFMT(LogParserManager, "Add monitoring failure. file=%s]"),filepath.c_str());
		return false;
	}

	{
		ZQ::common::MutexGuard guard(_logfilesLock);
		_logfiles[filepath] = expiredTime;
	}
	_sysWakeUp.signal();
	return true;
}

void LogParserManager::remove(std::string filepath)
{
	ZQ::common::MutexGuard guard(_logfilesLock);
	// step 1: find out the log monitor
	std::map<std::string,int64>::iterator it = _logfiles.find(filepath);
	if(_logfiles.end() == it)
	{ 
		return;
	}
	_logfiles.erase(it);
}

int LogParserManager::run()
{
	int64 timeLimit = 0,waitTime = 15000;
	while(!_Quit)
	{
		SYS::SingleObject::STATE state = _sysWakeUp.wait(waitTime);
		if (_Quit)
			break;
		waitTime = 5000;
		if(SYS::SingleObject::SIGNALED == state || SYS::SingleObject::TIMEDOUT == state)
		{ 
			ZQ::common::MutexGuard guard(_logfilesLock);
			for(std::map<std::string,int64>::iterator it = _logfiles.begin(); it != _logfiles.end();)
			{
				timeLimit = it->second - ZQ::common::now();
				//_log(ZQ::common::Log::L_DEBUG, CLOGFMT(LogParserManager, " filePath = %s,CurrentTime=%llu msec expiredTime = %llu msec, timeLimit = %lld msec."),it->first.c_str(),ZQ::common::now(),it->second,timeLimit);
				if (timeLimit <= 0)
				{
					LogMonitorRequest* cmd = new LogMonitorRequest (it->first,*this); 
					if (cmd != NULL)
						cmd->start(); 

					_logfiles.erase(it++);
				}
				else
				{
					if (waitTime > timeLimit)
					{
						waitTime = timeLimit;
					}
					it++;
				}
			}
			if (_logfiles.empty())
				waitTime = 15000;

			//_log(ZQ::common::Log::L_DEBUG, CLOGFMT(LogParserManager, "waitTime = %llu msec."),waitTime);
		}
	}
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(LogParserManager, "quit watchDog thread!"));
	return 0;
}
