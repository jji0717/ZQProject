#ifndef __EventSink_LogParserManager_H__
#define __EventSink_LogParserManager_H__
#include "EventSink.h"
#include "LiteralFunc.h"
#include "MessageSource.h"
#include "LogMonitor.h"
#include "MsgSenderPump.h"
#include <Locks.h>
typedef std::map<std::string, std::string> Properties;
class DirectSender {
public:
    DirectSender();
    void init(const std::vector<MSGHANDLE>& senders);
    void send(const std::string& category, const std::string& name, int id, const Properties& properties, const std::string& targets);
public:
    std::string localhost_;
    std::vector<MSGHANDLE> senders_;
};
struct LoggerInfo
{
    std::string logFile;
    std::string logType;
    std::string syntaxFile;
    std::string syntaxKey;
    typedef std::map<std::string, std::string> Properties;
    Properties ctx;
};

class LogParserManager
{
public:
    LogParserManager(ZQ::common::Log& log, Ice::CommunicatorPtr comm, const std::string& confPath);
    ~LogParserManager();

    void startInitialMonitoring();
    bool addMonitoring(const LoggerInfo& logger);
    void removeMonitoring(const LoggerInfo& logger);
    DirectSender& directSender();
private:
    ZQ::common::Log* _pLog;
    ZQ::common::Config::Loader< EventSinkConf >* _pSinkConf;
    MsgSenderPump* _pSenderPump;
    LogPositionDb* _posDb;
    MessageSourceFactory* _pMsgSrcFac;
    Literal::Library _lib;

    struct ManagementItem
    {
        IRawMessageHandler* msgHandler;
        IRawMessageSource* msgSrc;
        LogMonitor* monitor;
        ManagementItem()
        {
            msgHandler = NULL;
            msgSrc = NULL;
            monitor = NULL;
        }
    };
    typedef std::map<std::string, ManagementItem> ManagementData;
    ManagementData _mgmtData;
    ZQ::common::Mutex _lockMgmt;

    // runtime recover point cache
    RecoverPointCache _rpCache;

    DirectSender _directSender;
};
#endif

