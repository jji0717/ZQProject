#ifndef __EventSink_LogMonitor_H__
#define __EventSink_LogMonitor_H__
#include "EventSink.h"
#include <NativeThread.h>
#include <Log.h>
#include <Locks.h>
#ifdef ZQ_OS_LINUX
extern "C"
{
#include <semaphore.h>
}
#endif
class RecoverPointCache {
public:
    bool get(MessageIdentity& rp, const std::string& src);
    void set(const MessageIdentity& rp);
public:
    ZQ::common::Mutex lock_;
    std::map<std::string, MessageIdentity> data_;
};
class LogMonitor: public ZQ::common::NativeThread
{
public:
    LogMonitor(ZQ::common::Log& log, IRawMessageSource* msgSrc, IRawMessageHandler* msgHandler, const MessageIdentity& recoverPoint, RecoverPointCache& rpCache);
    ~LogMonitor();
    void setParsingLoad(int32 idleTime, int32 busyTime);
    virtual int run();
private:
    ZQ::common::Log& _log;
#ifdef ZQ_OS_MSWIN
    HANDLE _hQuit;
#else
	sem_t _hQuit;
#endif
    IRawMessageSource* _msgSrc;
    IRawMessageHandler* _msgHandler;
    MessageIdentity _recoverPoint;
    std::string _filePath;
    RecoverPointCache& _rpCache;

    int32 _idleTime;
    int32 _busyTime;
};
#endif

