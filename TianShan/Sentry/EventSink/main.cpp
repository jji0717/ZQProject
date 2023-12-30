#include "LogParserManager.h"
#include <FileLog.h>
#include <TimeUtil.h>

Literal::Library gLiteralFuncLib;
ZQ::common::Log* log = NULL;

int
main(int argc, char* argv[])
{
    try{
        log = new ZQ::common::FileLog("C:\\test.log", ZQ::common::Log::L_DEBUG);

		ZQ::common::setGlogger(log);
    } catch (...) {
        return -1;
    }

    LogParserManager* parserMgr = NULL;
    try
    {
        parserMgr = new LogParserManager(glog, "C:\\TianShan\\etc\\Sentry.xml");
    }
    catch(const ZQ::common::Exception& e)
    {
        glog(ZQ::common::Log::L_ERROR, "Exception: %s", e.getString());
        glog.flush();
        return -1;
    }

    LoggerInfo logger;
    logger.logFile = "C:\\sentry.log";
    logger.logType = "filelog";
    logger.syntaxFile = "c:\\syntax.xml";
    logger.syntaxKey = "test";
    parserMgr->addMonitoring(logger);

    LoggerInfo logger2;
    logger2.logFile = "C:\\TianShan\\logs\\EventChannel.log";
    logger2.logType = "filelog";
    logger2.syntaxFile = "c:\\syntax.xml";
    logger2.syntaxKey = "test2";
    parserMgr->addMonitoring(logger2);

    LoggerInfo logger3;
    logger3.logFile = "C:\\TianShan\\logs\\EventChannel_shell.log";
    logger3.logType = "sclog";
    logger3.syntaxFile = "c:\\syntax.xml";
    logger3.syntaxKey = "test_sclog";
    parserMgr->addMonitoring(logger3);

    system("pause");
    delete parserMgr;

	ZQ::common::setGlogger();
	if (log)
		delete log;

    return 0;
}