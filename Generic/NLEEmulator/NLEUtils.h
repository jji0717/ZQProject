#ifndef __ZQ_NLE_Utils_H__
#define __ZQ_NLE_Utils_H__
#include <NativeThreadPool.h>
#include <SystemUtils.h>
#include <boost/shared_ptr.hpp>
namespace NLE {

class TimeoutObject
{
public:
    virtual ~TimeoutObject(){}
    virtual void onTimer() = 0;
};
typedef boost::shared_ptr<TimeoutObject> TimeoutObjectPtr;

class Watchdog: public ZQ::common::NativeThread
{
public:
    Watchdog(ZQ::common::Log& log, ZQ::common::NativeThreadPool& pool);
    ~Watchdog();
    virtual int run();

    // watching interface
    void watch(int id , size_t timeout);
    void cancel(int id);
    // timeout object registration management
    int add(TimeoutObjectPtr obj);
    TimeoutObjectPtr remove(int id);
    TimeoutObjectPtr get(int id);
    void clear();
    void stop();
private:
    ZQ::common::Log& log_;
    ZQ::common::NativeThreadPool& pool_;
    bool quit_;
    SYS::SingleObject hWakeup_;

    // watching management
    struct WatchItem {
        WatchItem(int i, int64 exp):id(i), expiration(exp) {}
        bool operator<(const WatchItem& o) const { return (expiration < o.expiration); }

        int id;
        int64 expiration;
    };
    typedef std::list<WatchItem> WatchingList;
    WatchingList watchingList_;
    ZQ::common::Mutex lockWatching_;
    // watching list manipulating function
    static void addWatching(WatchingList& wl, const WatchItem& item);
    static void removeWatching(WatchingList& wl, int id);
    static WatchingList removeExpired(WatchingList& wl, int64 t);

    // timeout object management
    typedef std::map<int, TimeoutObjectPtr> Objects;
    Objects objs_;
    int nextId_;
    ZQ::common::Mutex lockObjs_;
};
} // namespace
#endif
