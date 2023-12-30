
#include "EventSink/LogParserManager.h"
#include "SentryEnv.h"

namespace ZQTianShan {
namespace Sentry {

class LogParserManagement
{
public:
    LogParserManagement(ZQ::common::Log& log, Ice::CommunicatorPtr comm, const std::string& confPath);
	~LogParserManagement();
public:
    void UpdateProcessLogInfo(int processId, const ::ZqSentryIce::LoggerInfos &loggerInfos);
    DirectSender& directSender();
private:
    LogParserManager* _parserMgr;
    typedef std::map<int, ::ZqSentryIce::LoggerInfos> LoggersMap;
    LoggersMap _loggersMap;
	ZQ::common::Mutex _lockMgmt;
    ZQ::common::Log& _log;
};
}}//namespace ZQTianShan::Sentry


