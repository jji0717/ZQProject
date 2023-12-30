#ifndef __ZQ_VREP_Utils_H__
#define __ZQ_VREP_Utils_H__

#include <ZQ_common_conf.h>
#include <Locks.h>
#ifdef ZQ_OS_MSWIN
#include <winsock2.h>
#endif
#include <NativeThreadPool.h>
#include <boost/shared_ptr.hpp>
#include "SystemUtils.h"

namespace ZQ {
namespace Vrep {

typedef unsigned char byte;
typedef u_short word;
typedef u_long dword;
typedef std::vector<byte> bytes;

template < class EventT >
class EventQueue
{
public:
    EventQueue() {
#ifdef ZQ_OS_MSWIN
        hEvent_ = CreateEvent(NULL, TRUE, FALSE, NULL); // manual reset
#endif
    }
    ~EventQueue() {
#ifdef ZQ_OS_MSWIN
        CloseHandle(hEvent_);
#endif
    }
    // take a msec time out
    // return true for any event exists
    //        false for no event
    
    bool check(int timeout) {
			
#ifdef ZQ_OS_MSWIN
        return (WAIT_OBJECT_0 == WaitForSingleObject(hEvent_, timeout));
#else
		SYS::SingleObject::STATE st = hEvent_.wait(timeout);
        ZQ::common::MutexGuard guard(lock_);
        if(!q_.empty()) {
            hEvent_.signal();
            return true;
        } else {
            return false;
        }
#endif
}

    void issue(EventT evnt) {
        ZQ::common::MutexGuard guard(lock_);
        q_.push(evnt);
#ifdef ZQ_OS_MSWIN
        SetEvent(hEvent_);
#else
		hEvent_.signal();
#endif
    }

    // fetch the next event in the queue
    // return false for no event
    bool fetch(EventT& evnt) {
        bool got = false;
        ZQ::common::MutexGuard guard(lock_);
        if(!q_.empty()) {
            evnt = q_.front();
            q_.pop();
            got = true;
        }

#ifdef ZQ_OS_MSWIN
        if(q_.empty()) {
            ResetEvent(hEvent_);
        }

#endif
        return got;
    }
    void clear() {
        ZQ::common::MutexGuard guard(lock_);
        q_.clear();
#ifdef ZQ_OS_MSWIN
        SetEvent(hEvent_); // cancel the pending check
#else
		hEvent_.signal();
#endif
    }
private:
    ZQ::common::Mutex lock_;
    std::queue<EventT> q_;
#ifdef ZQ_OS_MSWIN
	HANDLE hEvent_;
#else
    SYS::SingleObject hEvent_;
#endif
};

// helper class
class MessageHelper
{
public:
    MessageHelper(byte* buf, size_t len);
    byte* buffer(size_t offset);
    size_t length(size_t offset);

    int putByte(byte b, size_t offset);
    int putWord(word w, size_t offset);
    int putDword(dword dw, size_t offset);
    int put(const byte* data, size_t size, size_t offset);
    int put(const bytes& bs, size_t offset);

    int getByte(byte& b, size_t offset) const;
    int getWord(word& w, size_t offset) const;
    int getDword(dword& dw, size_t offset) const;
    int get(byte* data, size_t size, size_t offset) const;
    int get(bytes& bs, size_t size, size_t offset) const;
private:
    byte* buf_;
    size_t len_;
};

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

class Timer
{
public:
    Timer(Watchdog& watchdog, TimeoutObjectPtr obj);
    ~Timer();
    void start(size_t t);
    void restart();
    void clear();
private:
    Timer(const Timer&);
    Timer& operator=(const Timer&);
private:
    Watchdog& watchdog_;
    int id_;
    int timeout_;
};

}} // namespace ZQ::Vrep
#endif
