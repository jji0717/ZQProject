#include "NLEUtils.h"
#include <TimeUtil.h>
namespace NLE {
Watchdog::Watchdog(ZQ::common::Log& log, ZQ::common::NativeThreadPool& pool)
:log_(log), pool_(pool), quit_(false), nextId_(1)
{
}
Watchdog::~Watchdog() {
    stop();
}
void Watchdog::stop()
{
    if( !quit_ )
    {
        quit_ = true;
        hWakeup_.signal();
        waitHandle(-1);
    }
}

class TimeoutCmd: public ZQ::common::ThreadRequest {
public:
    TimeoutCmd(ZQ::common::NativeThreadPool& pool, TimeoutObjectPtr obj)
        :ThreadRequest(pool), obj_(obj) {
    }
    virtual ~TimeoutCmd() {}

protected: // impl of ThreadRequest
    virtual bool init(void)	{ return true; };
    virtual int run(void) {
        try {
            if(obj_) {
                obj_->onTimer();
            }
        } catch (...) {
        }
        return 0;
    }
    // no more overwrite-able
    void final(int retcode =0, bool bCancelled =false) { delete this; }
private:
    TimeoutObjectPtr obj_;
};
#define WATCHING_IDLE_INTERVAL 10000 // 10 seconds
int Watchdog::run() {
    do {
        if(quit_)
            break;

        WatchingList expired;
        int64 nextWakeup = 0;
        { // access the watching list
            ZQ::common::MutexGuard guard(lockWatching_);
            expired = removeExpired(watchingList_, ZQ::common::now());
            nextWakeup = watchingList_.empty() ? ZQ::common::now() + WATCHING_IDLE_INTERVAL : watchingList_.front().expiration;
        }
        for(WatchingList::iterator it = expired.begin(); it != expired.end(); ++it) {
            // start a timeout command
            TimeoutObjectPtr obj = get(it->id);
            if(obj) {
                (new TimeoutCmd(pool_, obj))->start();
            }
        }

        int64 nowTime = ZQ::common::now(); // some ticks past
        if(nowTime < nextWakeup)
            hWakeup_.wait(nextWakeup - nowTime);
    } while (true);

    return 0;
}

// watching interface
void Watchdog::watch(int id , size_t timeout) {
    ZQ::common::MutexGuard guard(lockObjs_);
    removeWatching(watchingList_, id);
    addWatching(watchingList_, WatchItem(id, ZQ::common::now() + timeout));
    hWakeup_.signal();
}

void Watchdog::cancel(int id) {
    ZQ::common::MutexGuard guard(lockWatching_);
    removeWatching(watchingList_, id);
    hWakeup_.signal();
}

// timeout object registration management
int Watchdog::add(TimeoutObjectPtr obj) {
    ZQ::common::MutexGuard guard(lockObjs_);
    int id = nextId_++;
    if(nextId_ <= 0)
        nextId_ = 1;

    objs_[id] = obj;
    return id;
}

TimeoutObjectPtr Watchdog::remove(int id) {
    ZQ::common::MutexGuard guard(lockObjs_);
    Objects::iterator it = objs_.find(id);
    if(it != objs_.end()) {
        TimeoutObjectPtr obj = it->second;
        objs_.erase(it);
        return obj;
    } else {
        return TimeoutObjectPtr();
    }
}

TimeoutObjectPtr Watchdog::get(int id) {
    ZQ::common::MutexGuard guard(lockObjs_);
    Objects::iterator it = objs_.find(id);
    if(it != objs_.end()) {
        return it->second;
    } else {
        return TimeoutObjectPtr();
    }
}

void Watchdog::clear() {
    ZQ::common::MutexGuard guard(lockObjs_);
    objs_.clear();
}

// watching list manipulating function
void Watchdog::addWatching(WatchingList& wl, const WatchItem& item) {
    for(WatchingList::iterator it = wl.begin(); it != wl.end(); ++it) {
        if(item.expiration < it->expiration) {
            wl.insert(it, item);
            return;
        }
    }

    wl.push_back(item);
}
void Watchdog::removeWatching(WatchingList& wl, int id) {
    for(WatchingList::iterator it = wl.begin(); it != wl.end(); ++it) {
        if(id == it->id) {
            wl.erase(it);
            return;
        }
    }
}

Watchdog::WatchingList Watchdog::removeExpired(WatchingList& wl, int64 t) {
    WatchingList expired;
    for(WatchingList::iterator it = wl.begin(); it != wl.end(); ++it) {
        if(t < it->expiration) {
            expired.splice(expired.begin(), wl, wl.begin(), it);
            return expired;
        }
    }
    expired.splice(expired.begin(), wl);
    return expired;
}
}
