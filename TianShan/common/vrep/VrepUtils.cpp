#include "VrepUtils.h"
#include <TimeUtil.h>

namespace ZQ {
namespace Vrep {
// helper class
MessageHelper::MessageHelper(byte* buf, size_t len)
    :buf_(buf), len_(len) {
}
byte* MessageHelper::buffer(size_t offset) {
    if(offset <= len_)
        return (buf_ + offset);
    else
        return NULL;
}

size_t MessageHelper::length(size_t offset) {
    if(offset <= len_)
        return (len_ - offset);
    else
        return 0;
}

int MessageHelper::putByte(byte b, size_t offset) {
    if(offset + 1 <= len_) {
        buf_[offset] = b;
        return 1;
    } else {
        return -1;
    }
}
int MessageHelper::putWord(word w, size_t offset) {
    if(offset + 2 <= len_) {
        buf_[offset] = (byte)((0xFF00 & w) >> 8); // byte 0
        buf_[offset + 1] = (byte)(0x00FF & w); // byte 1
        return 2;
    } else {
        return -1;
    }
}
int MessageHelper::putDword(dword dw, size_t offset) {
    if(offset + 4 <= len_) {
        buf_[offset] = (byte)((0xFF000000 & dw) >> 24); // byte 0
        buf_[offset + 1] = (byte)((0x00FF0000 & dw) >> 16); // byte 1
        buf_[offset + 2] = (byte)((0x0000FF00 & dw) >> 8); // byte 2
        buf_[offset + 3] = (byte)(0x000000FF & dw); // byte 3
        return 4;
    } else {
        return -1;
    }
}
int MessageHelper::put(const byte* data, size_t size, size_t offset) {
    if(offset + size <= len_) {
        if(size == 0)
            return 0;
        memcpy(buf_ + offset, data, size);
        return size;
    } else {
        return -1;
    }
}
int MessageHelper::put(const bytes& bs, size_t offset) {
    const byte* data = (!bs.empty()) ? (&bs[0]) : NULL;
    size_t size = bs.size();
    return put(data, size, offset);
}

int MessageHelper::getByte(byte& b, size_t offset) const {
    if(offset + 1 <= len_) {
        b = buf_[offset];
        return 1;
    } else {
        return -1;
    }
}

int MessageHelper::getWord(word& w, size_t offset) const {
    if(offset + 2 <= len_) {
        w = 0;
        w += ((word)buf_[offset]) << 8; // byte 0
        w += ((word)buf_[offset + 1]); // byte 1

        return 2;
    } else {
        return -1;
    }
}
int MessageHelper::getDword(dword& dw, size_t offset) const {
    if(offset + 4 <= len_) {
        dw = 0;
        dw += ((word)buf_[offset]) << 24; // byte 0
        dw += ((word)buf_[offset + 1]) << 16; // byte 1
        dw += ((word)buf_[offset + 2]) << 8; // byte 2
        dw += ((word)buf_[offset + 3]); // byte 3
        return 4;
    } else {
        return -1;
    }
}
int MessageHelper::get(byte* data, size_t size, size_t offset) const {
    if(offset + size <= len_) {
        if(size == 0)
            return 0;

        memcpy(data, buf_ + offset, size);
        return size;
    } else {
        return -1;
    }
}

int MessageHelper::get(bytes& bs, size_t size, size_t offset) const {
    bs.resize(size); // allocate buffer
    byte* data = (!bs.empty()) ? &bs[0] : NULL;
    return get(data, size, offset);
}

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



Timer::Timer(Watchdog& watchdog, TimeoutObjectPtr obj)
    :watchdog_(watchdog), id_(-1), timeout_(-1) {
    id_ = watchdog.add(obj);
}
Timer::~Timer() {
    watchdog_.cancel(id_);
    watchdog_.remove(id_);
}

void Timer::start(size_t t) {
    timeout_ = t;
    watchdog_.cancel(id_);
    watchdog_.watch(id_, timeout_);
}

void Timer::restart() {
    if(timeout_ >= 0) {
        watchdog_.cancel(id_);
        watchdog_.watch(id_, timeout_);
    }
}

void Timer::clear() {
    watchdog_.cancel(id_);
}

}} // namespace ZQ::Vrep
