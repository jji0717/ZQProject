#include "VrepListener.h"
#include <FileLog.h>
using namespace ZQ::common;
class ConsoleLog: public Log {
public:
    ConsoleLog():Log(Log::L_DEBUG) {
    }
    virtual void writeMessage(const char *msg, int level /*=-1*/)
    {
        printf("%s\t%s\n", getVerbosityStr(level), msg);
    }
};
ConsoleLog CONSOLE;
class D6AttributesReceiver:public ZQ::Vrep::StateMachine::Monitor {
    virtual void onUpdateMessage(const ZQ::Vrep::UpdateMessage& msg) {
        std::string txt;
        msg.textDump(txt);
        printf("onUpdateMessage(): %s\n", txt.c_str());
    }
};
class D6Fac:public ZQ::Vrep::MonitorFactory{
    virtual ZQ::Vrep::StateMachine::Monitor* create() {
        return new D6AttributesReceiver();
    }
    virtual void destroy(ZQ::Vrep::StateMachine::Monitor* m) {
        delete m;
    }
};
int
main() {
    FileLog* pLog = new FileLog("C:\\VrepListenerTest.log", Log::L_DEBUG);
    NativeThreadPool thrdPool;
    ZQ::Vrep::Server svr(*pLog, thrdPool);
    svr.setBindAddress("127.0.0.1", 2234);
    D6Fac fac;
    svr.setMonitorFactory(fac);
    ZQ::Vrep::Configuration conf;
    conf.identifier = 1;
    conf.streamingZone = "ZQ";
    conf.componentName = "Test";
    conf.vendorString = "HaHa";
    conf.defaultHoldTimeSec = 60;
    conf.connectRetryTimeSec = 60;
    conf.connectTimeoutMsec = 2000;
    conf.keepAliveTimeSec = 10;
    conf.sendReceiveMode = VREP_ReceiveOnlyMode;
    svr.configure(conf);
    svr.start();
    printf("Press any key to stop the server:");
    getchar();
    CONSOLE(Log::L_INFO, "Stopping server...");
    svr.stop();
    CONSOLE(Log::L_INFO, "Server stopped");
    pLog->flush();
    delete pLog;
    return 0;
}