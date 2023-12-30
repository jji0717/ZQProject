
#include <boost/thread.hpp>
#include "LogPaserManagement.h"

namespace ZQTianShan {
namespace Sentry {

// auxiliary class
struct LoggerInfoEqual
{
    LoggerInfoEqual(const ::ZqSentryIce::LoggerInfo& loggerInfo):_info(loggerInfo){}
    bool operator() (const ::ZqSentryIce::LoggerInfo& o)
    {
        return (
            _info.logFile == o.logFile &&
            _info.logType == o.logType &&
            _info.syntaxFile == o.syntaxFile &&
            _info.syntaxKey == o.syntaxKey &&
            _info.ctx.size() == o.ctx.size() &&
            std::equal(_info.ctx.begin(), _info.ctx.end(), o.ctx.begin())
            );
    }
private:
    ::ZqSentryIce::LoggerInfo _info;
};
LogParserManagement::LogParserManagement(ZQ::common::Log& log, Ice::CommunicatorPtr comm, const std::string& confPath)
:_log(log)
{
    _parserMgr = new LogParserManager(log, comm, confPath);
    _parserMgr->startInitialMonitoring();
}
LogParserManagement::~LogParserManagement()
{
    if(_parserMgr)
    {
        delete _parserMgr;
        _parserMgr = NULL;
    }
}

void LogParserManagement::UpdateProcessLogInfo(int processId, const ::ZqSentryIce::LoggerInfos &loggerInfos)
{
	ZQ::common::MutexGuard gd(_lockMgmt);
    {
        std::ostringstream buf;
        for(::ZqSentryIce::LoggerInfos::const_iterator it_log = loggerInfos.begin(); it_log != loggerInfos.end(); ++it_log)
        {
            buf << "(";
            buf << it_log->logFile;
            buf << " , ";
            buf << it_log->syntaxFile;
            buf << ", ";
            buf << it_log->syntaxKey;
            buf << ") ";
        }
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(LogParserManagement, "UpdateProcessLogInfo() for process [%d]. logs: %s"), processId, buf.str().c_str());

    }
	//update local service log informations
	int iUpdatedProcessID = processId;
	LoggersMap::iterator it = _loggersMap.find(iUpdatedProcessID);
	if ( it == _loggersMap.end() ) // add
	{
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(LogParserManagement, "no log info for process [%d], add new record."), iUpdatedProcessID);

		//no log info for current process,update it
		_loggersMap.insert(LoggersMap::value_type(iUpdatedProcessID,loggerInfos));

		//notify log parser
        ::ZqSentryIce::LoggerInfos::const_iterator itLogger;
		for(itLogger = loggerInfos.begin() ; itLogger != loggerInfos.end() ; ++itLogger )
		{
            // convert ZQSentryIce::LoggerInfo to LoggerInfo
            LoggerInfo logger;
            logger.logFile = itLogger->logFile;
            logger.logType = itLogger->logType;
            logger.syntaxFile = itLogger->syntaxFile;
            logger.syntaxKey = itLogger->syntaxKey;
            logger.ctx = itLogger->ctx;
            // monitor
            if(_parserMgr->addMonitoring(logger))
            {
                _log(ZQ::common::Log::L_DEBUG, CLOGFMT(LogParserManagement, "Monitor logger (%s, %s, %s) for process [%d]"), logger.logFile.c_str(), logger.syntaxFile.c_str(), logger.syntaxKey.c_str(), iUpdatedProcessID);
            }
            else
            {
                _log(ZQ::common::Log::L_ERROR, CLOGFMT(LogParserManagement, "Failed to monitor logger (%s, %s, %s) for process [%d]"), logger.logFile.c_str(), logger.syntaxFile.c_str(), logger.syntaxKey.c_str(), iUpdatedProcessID);
            }
		}
	}
	else // update
	{
        _log(ZQ::common::Log::L_DEBUG, CLOGFMT(LogParserManagement, "find log info for process [%d], update it."), iUpdatedProcessID);
        const ::ZqSentryIce::LoggerInfos& oldLogInfo = it->second;
		const ::ZqSentryIce::LoggerInfos& newLogInfo = loggerInfos;
		//detect which log file is updated or deleted or modified
        // we should find the intersection of the two set of loggers
		::ZqSentryIce::LoggerInfos::const_iterator itOld = oldLogInfo.begin();
		for( ; itOld != oldLogInfo.end() ; itOld++ )
		{
            if(newLogInfo.end() == std::find_if(newLogInfo.begin(), newLogInfo.end(), LoggerInfoEqual(*itOld)))
            {
                _log(ZQ::common::Log::L_DEBUG, CLOGFMT(LogParserManagement, "Stop monitoring logger (%s, %s, %s) due to the process info updating. PID=[%d]"), itOld->logFile.c_str(), itOld->syntaxFile.c_str(), itOld->syntaxKey.c_str(), iUpdatedProcessID);
                // convert ZQSentryIce::LoggerInfo to LoggerInfo
                LoggerInfo logger;
                logger.logFile = itOld->logFile;
                logger.logType = itOld->logType;
                logger.syntaxFile = itOld->syntaxFile;
                logger.syntaxKey = itOld->syntaxKey;
                logger.ctx = itOld->ctx;
                // remove monitoring
                _parserMgr->removeMonitoring(logger);
            }
        }
        ::ZqSentryIce::LoggerInfos::const_iterator itNew = newLogInfo.begin();
		for( ; itNew != newLogInfo.end() ; itNew++ )
		{
            // convert ZQSentryIce::LoggerInfo to LoggerInfo
            LoggerInfo logger;
            logger.logFile = itNew->logFile;
            logger.logType = itNew->logType;
            logger.syntaxFile = itNew->syntaxFile;
            logger.syntaxKey = itNew->syntaxKey;
            logger.ctx = itNew->ctx;
            // monitor
            if(_parserMgr->addMonitoring(logger))
            {
                _log(ZQ::common::Log::L_DEBUG, CLOGFMT(LogParserManagement, "Monitor logger (%s, %s, %s) for process [%d]"), logger.logFile.c_str(), logger.syntaxFile.c_str(), logger.syntaxKey.c_str(), iUpdatedProcessID);
            }
            else
            {
                _log(ZQ::common::Log::L_ERROR, CLOGFMT(LogParserManagement, "Failed to monitor logger (%s, %s, %s) for process [%d]"), logger.logFile.c_str(), logger.syntaxFile.c_str(), logger.syntaxKey.c_str(), iUpdatedProcessID);
            }
		}
        // save the new logger info
        it->second = loggerInfos;
	}
}

DirectSender& LogParserManagement::directSender(){
    return _parserMgr->directSender();
}
}}//namespace ZQTianShan::Sentry

