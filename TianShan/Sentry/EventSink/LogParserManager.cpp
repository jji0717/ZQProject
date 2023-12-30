#include "LogParserManager.h"
#include "RegExHandler.h"
#include "SyntaxConfig.h"
#include "EventSinkCfg.h"
#include "MsgSenderPump.h"
#include <TimeUtil.h>
#include <FileLog.h>
#include "TimeConv.h"
#include <boost/shared_ptr.hpp>


class EventSender: public IEventSender
{
public:
    struct Handler {
        std::string type;
        OnNewMessage onMessage;
        PositionRecordPtr pos;
        Handler():onMessage(NULL), pos(NULL) {}
    };

    typedef std::vector<Handler> Handlers;
    EventSender(const Handlers& handlers, ZQ::common::Log& log)
        :_handlers(handlers), _log(log)
    {
    }
    virtual void post(const Event& evnt, const MessageIdentity& mid)
    {
        MSGSTRUCT msg;
        msg.id = evnt.eventId;
        msg.category = evnt.category;
        msg.timestamp = evnt.stampUTC;
        msg.eventName = evnt.eventName;
        msg.sourceNetId = evnt.sourceNetId;
        msg.property = evnt.params;

        for(size_t i = 0; i < _handlers.size(); ++i)
        {
            Handler& handler = _handlers[i];
            int64 position = 0;
            int64 stamp = 0;
            handler.pos->get(position, stamp);
            if((mid.stamp < stamp) ||
               (mid.stamp == stamp && mid.position <= position)) {
                // message too old
                continue;
            }
            try {
                handler.onMessage(msg, mid, handler.pos.get());
            } catch (...) {
                _log(ZQ::common::Log::L_WARNING, CLOGFMT(EventSender, "post() Unexpected exception when dispatch message to handler(%s). message:Category(%s), Name(%s)"), handler.type.c_str(), msg.category.c_str(), msg.eventName.c_str());
            }
        }
    }
private:
    Handlers _handlers;
	ZQ::common::Log& _log;
};

class LogMessageHandler:public IRawMessageHandler
{
public:
    LogMessageHandler(ZQ::common::Log& log, const SyntaxConf::Rules& rules, const EventSender::Handlers& handlers, const Literal::Library& lib)
        :_log(log)
    {
        using namespace ZQ::common;
        for(SyntaxConf::Rules::const_iterator itRule = rules.begin(); itRule != rules.end(); ++itRule)
        {
            if(!itRule->enabled)
                continue;
            EventTemplate tmpl;
            tmpl.category = itRule->evnt.category;
            tmpl.eventId = itRule->evnt.eventId;
            tmpl.eventName = itRule->evnt.eventName;
            tmpl.stampUTC = itRule->evnt.stampUTC;
            tmpl.sourceNetId = itRule->evnt.sourceNetId;
            tmpl.params = itRule->evnt.params;

            std::vector<std::string> targets;
            std::string strTargets = itRule->evnt.strTargets;
            // uniform the target name
            std::transform(strTargets.begin(), strTargets.end(), strTargets.begin(), tolower);
            ZQ::common::stringHelper::SplitString(strTargets, targets, ";", "; ");
            // create the event sender
            EventSender::Handlers targetHandlers;
            if(targets.empty()) {
                targetHandlers = handlers;
            } else {
                for(size_t iT = 0; iT < targets.size(); ++iT) {
                    std::string &target = targets[iT];
                    // got the handler of the target
                    bool bGot = false;
                    for(size_t iH = 0; iH < handlers.size(); ++iH) {
                        const EventSender::Handler &handler = handlers[iH];
                        if(target == handler.type) {
                            targetHandlers.push_back(handler);
                            bGot = true;
                            break;
                        }
                    }
                    if(!bGot) {
                        _log(Log::L_WARNING, CLOGFMT(LogMessageHandler, "No handler found for target(%s)"), target.c_str());
                    }
                }
            }
            if(targetHandlers.empty()) {
                _log(Log::L_WARNING, CLOGFMT(LogMessageHandler, "No handler found for all targets(%s)"), strTargets.c_str());
                continue;
            }
            EventSender* sender = new EventSender(targetHandlers, _log);
            try
            {
                RegExpHandler* reHandler = new RegExpHandler(_log, itRule->regex, tmpl, sender, lib);
                SubhandlerData subhandler;
                subhandler.sender = sender;
                subhandler.reHandler = reHandler;
                _subhandlers.push_back(subhandler);
            }
            catch(const ZQ::common::Exception& e)
            {
                _log(Log::L_WARNING, CLOGFMT(LogMessageHandler, "Caught [%s] during creating regex message handler"), e.getString());
                delete sender;
            }
        }
        if(_subhandlers.empty())
        { // no handler
            _log(Log::L_WARNING, CLOGFMT(LogMessageHandler, "No message handler created successfully"));
            throw ZQ::common::Exception("No message handler created successfully");
        }
    }
    ~LogMessageHandler() {
        clear();
    }
    virtual void handle(const char* msg, int len, const MessageIdentity& mid) {
        for(size_t i = 0; i < _subhandlers.size(); ++i) {
            _subhandlers[i].reHandler->handle(msg, len, mid);
        }
    }
private:
    void clear() {
        for(size_t i = 0; i < _subhandlers.size(); ++i) {
            delete _subhandlers[i].reHandler;
            delete _subhandlers[i].sender;
        }
        _subhandlers.clear();
    }
private:
    ZQ::common::Log& _log;
    struct SubhandlerData {
        SubhandlerData():sender(NULL), reHandler(NULL) {}
        EventSender* sender;
        RegExpHandler* reHandler;
    };
    typedef std::vector<SubhandlerData> Subhandlers;
    Subhandlers _subhandlers;
};

static std::string fileLogTimeToUTC(const Literal::Arguments& args);
static std::string scLogTimeToUTC(const Literal::Arguments& args);
static std::string isoTimeToLoaclNomal(const Literal::Arguments& args);
static std::string sysLogTimeToUTC(const Literal::Arguments& args);
static std::string sysNormalTimeToUTC(const Literal::Arguments& args);
// helper marcro for clean field
#define ClearField(f) if(f) { delete f; f = NULL; }
#define ClearAllFields() {\
    ClearField(_pMsgSrcFac);\
    ClearField(_posDb);\
    ClearField(_pSenderPump);\
    ClearField(_pSinkConf);\
    ClearField(_pLog);\
    }
#define ResetAllFields() {\
    _pLog = NULL;\
    _pSinkConf = NULL;\
    _pSenderPump = NULL;\
    _posDb = NULL;\
    _pMsgSrcFac = NULL;\
    }
LogParserManager::LogParserManager(ZQ::common::Log& log, Ice::CommunicatorPtr comm, const std::string& confPath)
{
    ResetAllFields();

    using namespace ZQ::common;
    // init the literal function lib
    _lib.clear();
    _lib["FILELOGTIME2UTC"] = fileLogTimeToUTC;
    _lib["SCLOGTIME2UTC"] = scLogTimeToUTC;
	_lib["ISOTIME2LOCALTIME"] = isoTimeToLoaclNomal;
#ifdef ZQ_OS_LINUX
	_lib["SYSLOGTIME2UTC"] = sysLogTimeToUTC;
	_lib["SYSNORMALTIME2UTC"] = sysNormalTimeToUTC;
#endif
    { // parse the config
        _pSinkConf = new Config::Loader< EventSinkConf >("");
        _pSinkConf->setLogger(&log);
        if(!_pSinkConf->load(confPath.c_str()))
        {
            ClearAllFields();
            throw ZQ::common::Exception(std::string("Failed to load config file: ") + confPath);
        }
        // init the event sink log
        try
        {
			_pLog = new FileLog(_pSinkConf->logPath.c_str(),_pSinkConf->logLevel, _pSinkConf->logCount, _pSinkConf->logSize);
        }
        catch(const ZQ::common::FileLogException& e)
        {
            log(Log::L_ERROR, CLOGFMT(LogParserManager, "Caught [%s] during creating FileLog. FILE=%s"), e.getString(), _pSinkConf->logPath.c_str());
            ClearAllFields();
            throw e;
        }
#define LOG (*_pLog)
        LOG(Log::L_INFO, CLOGFMT(LogParserManager, "Starting the log parsing function..."));
    }
    // init the message sender pump
    _pSenderPump = new MsgSenderPump(LOG);
    if(!_pSenderPump->init(_pSinkConf)) {
        LOG(Log::L_ERROR, CLOGFMT(LogParserManager, "Failed to init MessageSenderPump."));
        ClearAllFields();
        throw ZQ::common::Exception("Failed to init MessageSenderPump.");
    }

    // init the position db
    _posDb = new LogPositionDb(LOG);
    if(!_posDb->init(comm, _pSinkConf->posDbPath, _pSinkConf->posDbEvictorSize)) {
        LOG(Log::L_ERROR, CLOGFMT(LogParserManager, "Failed to init the PositionDb at %s with evictor"), _pSinkConf->posDbPath.c_str());
        ClearAllFields();
        throw ZQ::common::Exception("Failed to init LogPositionDb.");
    }
    // init the message source factory
    _pMsgSrcFac = new MessageSourceFactory(LOG);
    _directSender.init(_pSenderPump->query());
    LOG(Log::L_INFO, CLOGFMT(LogParserManager, "The log parsing function is OK."));
}
LogParserManager::~LogParserManager()
{
    LOG(ZQ::common::Log::L_INFO, CLOGFMT(LogParserManager, "Stopping the log parsing function..."));
    {
        ZQ::common::MutexGuard sync(_lockMgmt);
        for(ManagementData::iterator it = _mgmtData.begin(); it != _mgmtData.end(); ++it)
        {
            delete it->second.monitor;
            _pMsgSrcFac->destroy(it->second.msgSrc);
            delete it->second.msgHandler;
        }
        _mgmtData.clear();
    }
    ClearField(_pMsgSrcFac);
    ClearField(_pSenderPump);
    _posDb->uninit();
    ClearField(_posDb);
    ClearField(_pSinkConf);
    LOG(ZQ::common::Log::L_INFO, CLOGFMT(LogParserManager, "The log parsing function is disabled."));
    ClearField(_pLog);
}
void LogParserManager::startInitialMonitoring()
{
    std::vector<MonitoringLog>::const_iterator it;
    for(it = _pSinkConf->initialMonitoringLogs.begin(); it != _pSinkConf->initialMonitoringLogs.end(); ++it)
    {
        LoggerInfo logger;
        logger.logFile = it->path;
        logger.logType = it->type;
        logger.syntaxFile = it->syntax;
        logger.syntaxKey = it->key;
        logger.ctx = it->ctx;
        addMonitoring(logger);
    }
}

bool LogParserManager::addMonitoring(const LoggerInfo& logger)
{
    using namespace ZQ::common;
    if (logger.logFile.empty() || logger.syntaxFile.empty() || logger.syntaxKey.empty())
    {
		LOG(Log::L_ERROR, CLOGFMT(LogParserManager, "addMonitoring() bad logger info: file[%s] syntax[%s] key[%s]"), logger.logFile.c_str(), logger.syntaxFile.c_str(), logger.syntaxKey.c_str());
        return false;
    }

    if (!logger.logType.empty() && !_pMsgSrcFac->checkType(logger.logType))
    {
		LOG(Log::L_ERROR, CLOGFMT(LogParserManager, "addMonitoring() illegal logger type: file[%s] type[%s]"), logger.logFile.c_str(), logger.logType.c_str());
        return false;
    }

    LOG(Log::L_INFO, CLOGFMT(LogParserManager, "addMonitoring() with file[%s] type[%s] syntax[%s] key[%s]"), logger.logFile.c_str(), logger.logType.c_str(), logger.syntaxFile.c_str(), logger.syntaxKey.c_str());

	ZQ::common::MutexGuard sync(_lockMgmt);
    if(_mgmtData.find(logger.logFile) != _mgmtData.end())
    { 
		// under monitoring
        LOG(Log::L_WARNING, CLOGFMT(LogParserManager, "addMonitoring() file[%s] already monitored"), logger.logFile.c_str());
        return false;
    }

    // step 1: parse the syntax file and get the syntax definition
    ZQ::common::Config::Loader<SyntaxDefConf> syntaxDef("");
    syntaxDef.setLogger(&LOG);
    
	// setup the context
    {
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
    }

    if(!syntaxDef.load(logger.syntaxFile.c_str()))
    {
        LOG(Log::L_ERROR, CLOGFMT(LogParserManager, "addMonitoring() failed to load syntax definition[%s]"), logger.syntaxFile.c_str());
        return false;
    }

    if(syntaxDef.syntaxMap.end() == syntaxDef.syntaxMap.find(logger.syntaxKey))
    { // no syntax found
		LOG(Log::L_ERROR, CLOGFMT(LogParserManager, "addMonitoring() failed to find syntax definition: key[%s]"), logger.syntaxKey.c_str());
        return false;
    }

    SyntaxConf syntax = syntaxDef.syntaxMap[logger.syntaxKey];
    if (!logger.logType.empty() && MessageSourceFactory::parseType(logger.logType).first != MessageSourceFactory::parseType(syntax.evntSrcType).first)
    {
		// logger type not fit
        LOG(Log::L_ERROR, CLOGFMT(LogParserManager, "addMonitoring() logger type[%s] conflicts with actual log type[%s] in the syntx definition"), logger.logType.c_str(), syntax.evntSrcType.c_str());
        return false;
    }

    std::string msgSrcType = (!logger.logType.empty() ? logger.logType : syntax.evntSrcType);
    MessageIdentity recoverPoint;
    bool rpGot = false; // if recover point got from cache
    if (_rpCache.get(recoverPoint, logger.logFile))
        rpGot = true;
	else
	{
        rpGot = false;
        recoverPoint.source = logger.logFile;
        recoverPoint.position = -1;
        recoverPoint.stamp = -1;
    }

    EventSender::Handlers handlers;
    std::vector<MSGHANDLE> senders = _pSenderPump->query();
    for(size_t iSender = 0; iSender < senders.size(); ++iSender)
	{
        PositionRecordPtr record = _posDb->getPosition(logger.logFile, senders[iSender].strType);
        if(!record)
		{
			LOG(Log::L_ERROR, CLOGFMT(LogParserManager, "addMonitoring() failed to get the position: handler[%s] file[%s]"), senders[iSender].strType.c_str(), logger.logFile.c_str());
            continue;
        }
        /*
        if(ZQ::common::now() - record->lastUpdatedAt() > DidcardLimit) {
            record->set(0, 0);
        }
        */
        if(!rpGot)
		{ 
			// need calculate the recover point from db records
            int64 position = 0;
            int64 stamp = 0;
            record->get(position, stamp);
            if((recoverPoint.stamp == -1 && recoverPoint.position == -1) ||
               (recoverPoint.stamp > stamp) ||
               (recoverPoint.stamp == stamp && recoverPoint.position > position))
			{
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

    if(!rpGot)
	{ 
		// update the recover point cache
        if(recoverPoint.stamp < 0 || recoverPoint.position < 0)
		{
            recoverPoint.stamp = 0;
            recoverPoint.position = 0;
        }

        _rpCache.set(recoverPoint);
    }
    
	// step 3: create the message handler base on the syntax conf
    ManagementItem mgmtItem;
    try {
        mgmtItem.msgHandler = new LogMessageHandler(LOG, syntax.rules, handlers, _lib);
    } 
	catch (const ZQ::common::Exception& e)
	{
		LOG(Log::L_ERROR, CLOGFMT(LogParserManager, "addMonitoring() failed to create message handler object, giving up monitoring file[%s] type[%s]: %s"), logger.logFile.c_str(), msgSrcType.c_str(), e.getString());
        return false;
    }
    
	// step 3: create the message source base on the logger info
    mgmtItem.msgSrc = _pMsgSrcFac->create(logger.logFile, msgSrcType);
    if(NULL == mgmtItem.msgSrc)
    {
        LOG(Log::L_ERROR, CLOGFMT(LogParserManager, "addMonitoring() failed to create message sourcing object, give up the monitoring file[%s] type[%s]"), logger.logFile.c_str(), msgSrcType.c_str());
        delete mgmtItem.msgHandler;
        mgmtItem.msgHandler = NULL;
    
		return false;
    }
    
	// step 4: create the log monitor
    mgmtItem.monitor = new LogMonitor(LOG, mgmtItem.msgSrc, mgmtItem.msgHandler, recoverPoint, _rpCache);
    
	// step 5: start the monitoring
    _mgmtData[logger.logFile] = mgmtItem;
    mgmtItem.monitor->start();
    LOG(Log::L_INFO, CLOGFMT(LogParserManager, "addMonitoring() monitored file[%s]"), logger.logFile.c_str());
    return true;
}

void LogParserManager::removeMonitoring(const LoggerInfo& logger)
{
    using namespace ZQ::common;
    LOG(Log::L_INFO, CLOGFMT(LogParserManager, "Enter removeMonitoring with [log=%s]"), logger.logFile.c_str());
    if(logger.logFile.empty())
    {
        LOG(Log::L_WARNING, CLOGFMT(LogParserManager, "removeMonitoring() Bad logger info. file=%s"), logger.logFile.c_str());
        return;
    }
    ZQ::common::MutexGuard sync(_lockMgmt);
    // step 1: find out the log monitor
    ManagementData::iterator it = _mgmtData.find(logger.logFile);
    if(_mgmtData.end() == it)
    { // not under monitoring?
        LOG(Log::L_WARNING, CLOGFMT(LogParserManager, "removeMonitoring() Log not being monitored. file=%s]"), logger.logFile.c_str());
        return;
    }
    // step 2: stop the monitoring
    delete it->second.monitor;
    // step 3: clean the resource
    _pMsgSrcFac->destroy(it->second.msgSrc);
    delete it->second.msgHandler;
    _mgmtData.erase(it);
    LOG(Log::L_INFO, CLOGFMT(LogParserManager, "removeMonitoring() Successfully to remove the monitoring. file=%s"), logger.logFile.c_str());
}

DirectSender& LogParserManager::directSender() {
    return _directSender;
}
///////////////////////////////////////////////////////////////////////////////
/// literal function definition

static std::string fileLogTimeToUTC(const Literal::Arguments& args)
{
    if(args.size() != 1)
    {
        return "";
    }
    return time2utc(getFilelogTime(args[0].c_str()));
}

static std::string scLogTimeToUTC(const Literal::Arguments& args)
{
    if(args.size() != 1)
    {
        return "";
    }
    return time2utc(getSclogTime(args[0].c_str()));
}

#ifdef ZQ_OS_MSWIN
static std::string isoTimeToLoaclNomal(const Literal::Arguments& args)
{
	if(args.size() !=1 )
		return "";
	std::string utctime = args[0];
	std::string strTime;
	char Buf[64] ="";

	// conver utc time 2009-08-12T08:00:00.000Z to local time.
	//format: 2009-08-12 08:00:00.000
	time_t _time;
	struct tm* _tm;
	bool bret = ZQ::common::TimeUtil::Iso2TimeEx(utctime.c_str(), _time);
	if(!bret)
		return "";
	_tm = localtime(&_time);
	sprintf(Buf, "%04d-%02d-%02d %02d:%02d:%02d.000\0", 
		_tm->tm_year + 1900, _tm->tm_mon +1, _tm->tm_mday,
		_tm->tm_hour, _tm->tm_min, _tm->tm_sec);
	strTime = Buf;
	return strTime;
}
#else
static std::string isoTimeToLoaclNomal(const Literal::Arguments& args)
{
	if(args.size() !=1 )
		return "";
	std::string utctime = args[0];
	std::string strTime;
	char Buf[64] ={0};

	// conver utc time 2009-08-12T08:00:00.000Z to local time.
	//format: 2009-08-12 08:00:00.000
	time_t _time;
	bool bret = ZQ::common::TimeUtil::Iso2Time(utctime.c_str(), _time);
	if(!bret)
		return "";
	struct tm _tmstorage;
    struct tm* _tm = localtime_r(&_time, &_tmstorage);

	sprintf(Buf, "%04d-%02d-%02d %02d:%02d:%02d.000", 
		_tm->tm_year + 1900, _tm->tm_mon +1, _tm->tm_mday,
		_tm->tm_hour, _tm->tm_min, _tm->tm_sec);
	strTime = Buf;
	return strTime;
}
#endif

static std::string sysLogTimeToUTC(const Literal::Arguments& args)
{
	if(args.size() != 1)
		return "";
	
	return time2utc(getSyslogTime1(args[0].c_str()));
}

static std::string sysNormalTimeToUTC(const Literal::Arguments& args)
{
	if(args.size() != 1)
		return "";
	return time2utc(getSyslogTime2(args[0].c_str()));
}

DirectSender::DirectSender() {
    char buf[128];
    gethostname(buf, 127);
    buf[127] = '\0';
    localhost_ = buf;
}

void DirectSender::init(const std::vector<MSGHANDLE>& senders) {
    senders_ = senders;
}

void DirectSender::send(const std::string& category, const std::string& name, int id, const Properties& properties, const std::string& targets) {
    if(targets.empty()) {
        return;
    }
    MessageIdentity mid;
    mid.source = "Instant";
    mid.position = 0;
    mid.stamp = ZQ::common::now();
    MSGSTRUCT msg;
    msg.id = id;
    msg.category = category;
    msg.eventName = name;
    msg.sourceNetId = localhost_;
    msg.timestamp = time2utc(mid.stamp);
    msg.property = properties;
    for(size_t i = 0; i < senders_.size(); ++i) {
        MSGHANDLE& sender = senders_[i];
        if(sender.handle != NULL && (targets == "*" || std::string::npos != targets.find(sender.strType))) {
            try {
                sender.handle(msg, mid, NULL);
            } catch (...) {
            }
        }
    }
}
