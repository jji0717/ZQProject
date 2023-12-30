#ifndef _ZQ_EVENTSINK_LOGPOSITIONI_H_
#define _ZQ_EVENTSINK_LOGPOSITIONI_H_
#include <ZQ_common_conf.h>
#include <Log.h>
#include <FileSystemOp.h>
#include "LogPosition.h"
#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include <Locks.h>

class LogPositionFactory : public Ice::ObjectFactory
{
public:

    LogPositionFactory();

    //
    // Operations from ObjectFactory
    //
    virtual Ice::ObjectPtr create(const std::string&);
    virtual void destroy();
};
typedef IceUtil::Handle<LogPositionFactory> LogPositionFactoryPtr;

class LogPositionI : public EventSink::LogPosition, public IceUtil::AbstractMutexI<IceUtil::Mutex>
{
public:
	LogPositionI();
    virtual ::std::string getSourceName(const ::Ice::Current& = ::Ice::Current()) const;
    virtual ::std::string getHandlerName(const ::Ice::Current& = ::Ice::Current()) const;
    virtual ::Ice::Long getLastUpdatedTime(const ::Ice::Current& = ::Ice::Current()) const;
    virtual void getData(::Ice::Long&, ::Ice::Long&, const ::Ice::Current& = ::Ice::Current()) const;
    virtual void updateData(::Ice::Long, ::Ice::Long, const ::Ice::Current& = ::Ice::Current());
};

typedef IceUtil::Handle<LogPositionI> LogPositionIPtr;

class PositionRecord : virtual public IceUtil::Shared
{
public:
    PositionRecord(EventSink::LogPositionPrx pos)
        :pos_(pos) {
        source_ = pos_->getSourceName();
        handler_ = pos_->getHandlerName();
    }
    const std::string& source() const { return source_; }
    const std::string& handler() const { return handler_; }
    void get(int64& position, int64& stamp) const {
        position = 0;
        stamp = 0;
        pos_->getData(position, stamp);
    }
    void set(int64 position, int64 stamp) {
        pos_->updateData(position, stamp);
    }
private:
    EventSink::LogPositionPrx pos_;
    std::string source_;
    std::string handler_;
};
typedef IceUtil::Handle<PositionRecord> PositionRecordPtr;

class LogPositionDb
{
public:
    LogPositionDb(ZQ::common::Log& log);
    bool init(Ice::CommunicatorPtr communicator, const std::string& path, int32 evictorSize);
    void uninit();
    PositionRecordPtr getPosition(const std::string& sourceName, const std::string handlerName);
    void xmlDump(std::ostream&);
private:
    EventSink::LogPositionPrx query(const std::string& sourceName, const std::string handlerName);
private:
    ZQ::common::Log& log_;
    Ice::ObjectAdapterPtr adapter_;
    Freeze::EvictorPtr evictor_;
    typedef std::map<std::pair<std::string, std::string>, PositionRecordPtr> PositionRecordCache;
    PositionRecordCache posCache_;
    ZQ::common::Mutex lockPosCache_;
};
#endif //_ZQ_EVENTSINK_LOGPOSITIONI_H_

