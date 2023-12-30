#include "LogMonitor.h"
#include "TimeConv.h"
#include <vector>

bool RecoverPointCache::get(MessageIdentity& rp, const std::string& src) {
    ZQ::common::MutexGuard guard(lock_);
    std::map<std::string, MessageIdentity>::const_iterator it = data_.find(src);
    if(it != data_.end()) { // found
        rp = it->second;
        return true;
    } else {
        return false;
    }
}
void RecoverPointCache::set(const MessageIdentity& rp) {
    ZQ::common::MutexGuard guard(lock_);
    data_[rp.source] = rp;
}

void LogMonitor::setParsingLoad(int32 idleTime, int32 busyTime) {
    if(idleTime > 0) {
        _idleTime = idleTime;
    }
    if(busyTime > 0) {
        _busyTime = busyTime;
    }
}

#ifdef ZQ_OS_MSWIN
LogMonitor::LogMonitor(ZQ::common::Log& log, IRawMessageSource* msgSrc, IRawMessageHandler* msgHandler, const MessageIdentity& recoverPoint, RecoverPointCache& rpCache)
    :_log(log), _msgSrc(msgSrc), _msgHandler(msgHandler), _recoverPoint(recoverPoint), _rpCache(rpCache)
{
    _filePath = _recoverPoint.source;
    _hQuit = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    _idleTime = 1000;
    _busyTime = 200;
}
LogMonitor::~LogMonitor()
{
    ::SetEvent(_hQuit);
    _log(ZQ::common::Log::L_INFO, CLOGFMT(LogMonitor, "~LogMonitor() Request to quit the monitoring thread. FILE=%s"), _filePath.c_str());
    waitHandle(-1); // may block here
    ::CloseHandle(_hQuit);
}

int LogMonitor::run()
{
    using namespace ZQ::common;
    DWORD tid = ::GetCurrentThreadId();
    _log(Log::L_INFO, CLOGFMT(LogMonitor, "Enter monitoring thread. TID=%d, FILE=%s, IdleTime=%d msec, BusyTime=%d msec"), tid, _filePath.c_str(), _idleTime, _busyTime);
    std::vector<char> _buf;
    _buf.resize(4096);

//#define WorkIntervalUnit 1000
    int32 WorkIntervalUnit = _idleTime;
    int workInterval = 1 * WorkIntervalUnit;
    unsigned int nFailCount = 0;
    MessageIdentity &mid = _recoverPoint;
    unsigned int nNoMoreMessage = 0;
    while(true)
    {
    try {
        workInterval = ((nFailCount / 2) + 1) * WorkIntervalUnit;

        if(WAIT_OBJECT_0 == ::WaitForSingleObject(_hQuit, workInterval))
        { // need quit the thread
            break;
        }

        if (!_msgSrc->open(mid))
        {
            nFailCount++;
			continue;
        }

		MessageIdentity openMid = mid;
		nFailCount = 0;
		DWORD timeLimit = ::GetTickCount() + _busyTime; // run at most BUSTYME msec

		while(::GetTickCount() < timeLimit)
		{
			MessageIdentity newMid;
			char* buf = &(_buf[0]);
			int len = _buf.size();
			int ret = _msgSrc->fetchNext(buf, &len, newMid);

			if (ret <0)
			{ // error occur
				_log(Log::L_ERROR, CLOGFMT(LogMonitor, "Can't fetch next log message. TID=%d, FILE=%s"), tid, _filePath.c_str());
				break;
			}

			if (ret > 0)
			{
				int64 stampStart = ZQ::common::now();
				_msgHandler->handle(buf, ret, newMid);
				_log(Log::L_DEBUG, CLOGFMT(LogMonitor, "file[%s] processed [%d]bytes took [%d]msec: %lld[%lld] => %lld[%lld]"), _filePath.c_str(), ret, (int) (ZQ::common::now() - stampStart), mid.position, mid.stamp, newMid.position, newMid.stamp);
				mid = newMid;
				continue;
			}

			// no more new data read in this round
			if (len <= 0) // no data || blank line, ignore
				break;

			if (_buf.size() < len)
			{ // grow buffer and retry
				_log(Log::L_DEBUG, CLOGFMT(LogMonitor, "Need more buffer (%d) to get next log message. current buffer size is %d. TID=%d, FILE=%s"), len, _buf.size(), tid, _filePath.c_str());
				_buf.resize(len);
				buf = &(_buf[0]);
				ret = _msgSrc->fetchNext(buf, &len, newMid);
				if (ret > 0)
				{
					_msgHandler->handle(buf, ret, newMid);
					mid = newMid;
				}
				else
				{ // error occur
					_log(Log::L_ERROR, CLOGFMT(LogMonitor, "Can't fetch next log message after grow buffer. TID=%d, FILE=%s"), tid, _filePath.c_str());
					break;
				}
			}
		}

		_msgSrc->close();
		if(openMid.stamp != mid.stamp || openMid.position != mid.position)
		{
			nNoMoreMessage = 0;
			// log the progress
			std::string stampStr = time2utc(mid.stamp);
			_log(Log::L_DEBUG, CLOGFMT(LogMonitor, "Progress: position [%lld] stamp[%s]. TID=%d, FILE=%s"), mid.position, stampStr.c_str(), tid, _filePath.c_str());
		}
		else
		{
			nNoMoreMessage += 1;
			if(nNoMoreMessage % (nNoMoreMessage > 1000 ? 1000 : 100) == 1)
			{
				std::string stampStr = time2utc(mid.stamp);
				_log(Log::L_DEBUG, CLOGFMT(LogMonitor, "No more message from: position [%lld] stamp[%s]. TID=%d, FILE=%s"), mid.position, stampStr.c_str(), tid, _filePath.c_str());
			}
		}
    } catch (...)
	{
        _log(Log::L_ERROR, CLOGFMT(LogMonitor, "Got unexpected exception in the monitoring. TID=%d, FILE=%s"), tid, _filePath.c_str());
    }
    } // while

    _log(Log::L_INFO, CLOGFMT(LogMonitor, "Leave monitoring thread. TID=%d, FILE=%s"), tid, _filePath.c_str());
    mid.source = _filePath; // keep the same as the input
    _rpCache.set(mid);
    return 0;
}

#else
extern "C"
{
#include <sys/time.h>
#include <errno.h>
}
LogMonitor::LogMonitor(ZQ::common::Log& log, IRawMessageSource* msgSrc, IRawMessageHandler* msgHandler, const MessageIdentity& recoverPoint, RecoverPointCache& rpCache)
    :_log(log), _msgSrc(msgSrc), _msgHandler(msgHandler), _recoverPoint(recoverPoint), _rpCache(rpCache)
{
    _filePath = _recoverPoint.source;
    sem_init(&_hQuit,0,0);
	_idleTime = 1000;
	_busyTime = 200;
}

LogMonitor::~LogMonitor()
{
    sem_post(&_hQuit);
    _log(ZQ::common::Log::L_INFO, CLOGFMT(LogMonitor, "~LogMonitor() Request to quit the monitoring thread. FILE=%s"), _filePath.c_str());
    waitHandle(5000);
    sem_destroy(&_hQuit);
}

int LogMonitor::run()
{
    using namespace ZQ::common;
    pthread_t tid = pthread_self();
    _log(Log::L_INFO, CLOGFMT(LogMonitor, "Enter monitoring thread. TID=%d, FILE=%s, IdleTime=%d msec, BusyTime=%d msec"), tid, _filePath.c_str(), _idleTime, _busyTime);
    std::vector<char> _buf;
    _buf.resize(4096);
	int nIntervalT = _idleTime;
	uint32 nFcount = 0;
    uint32 waitTime = nIntervalT;
    MessageIdentity &mid = _recoverPoint;
    unsigned int nNoMoreMessage = 0;
    try {
    while(true)
    {
		waitTime = (nFcount/2 +1) * nIntervalT; 

		struct timeval tval;
        struct timespec tsp;
		gettimeofday(&tval, (struct timezone*)NULL);
		int64 micSec = waitTime*1000ll + tval.tv_usec;
		tsp.tv_nsec = (micSec%1000000)*1000;
		tsp.tv_sec = tval.tv_sec + micSec/1000000;	
		int re = sem_timedwait(&_hQuit,&tsp);

        if(re == 0)
        { // need quit the thread
            break;
        }
		if(re == -1 && errno == EINTR)
			continue;

        if(_msgSrc->open(mid))
        {
            MessageIdentity openMid = mid;
			nFcount = 0;
			gettimeofday(&tval,NULL);
            int64 timeLimit = tval.tv_sec*1000ll + _busyTime + tval.tv_usec/1000; // run at most BusyTime
			int64 timenow;
            do
            {
                MessageIdentity newMid;
                char* buf = &(_buf[0]);
                int len = _buf.size();
                int ret = _msgSrc->fetchNext(buf, &len, newMid);

                if(ret > 0)
                {
                    _msgHandler->handle(buf, ret, newMid);
                    mid = newMid;
                }
                else if(ret == 0)
                {
                    if(len == 0) // no data
                    {
                        break;
                    }
                    else if(0 < len && (int)_buf.size() < len)
                    { // grow buffer and retry
                        _log(Log::L_DEBUG, CLOGFMT(LogMonitor, "Need more buffer (%d) to get next log message. current buffer size is %d. TID=%d, FILE=%s"), len, _buf.size(), tid, _filePath.c_str());
                        _buf.resize(len);
                        buf = &(_buf[0]);
                        ret = _msgSrc->fetchNext(buf, &len, newMid);
                        if(ret > 0)
                        {
                            _msgHandler->handle(buf, ret, newMid);
                            mid = newMid;
                        }
                        else
                        { // error occur
                            _log(Log::L_ERROR, CLOGFMT(LogMonitor, "Can't fetch next log message after grow buffer. TID=%d, FILE=%s"), tid, _filePath.c_str());
                            break;
                        }
                    }
                    else
                    { // blank line, ignore
                    }
                }
                else
                { // error occur
                    _log(Log::L_ERROR, CLOGFMT(LogMonitor, "Can't fetch next log message. TID=%d, FILE=%s"), tid, _filePath.c_str());
                    break;
                }
			gettimeofday(&tval,NULL);
			timenow = tval.tv_sec*1000ll + tval.tv_usec/1000;
            }while(timenow < timeLimit);
            _msgSrc->close();
            if(openMid.stamp != mid.stamp || openMid.position != mid.position) {
                nNoMoreMessage = 0;
                // log the progress
                std::string stampStr = time2utc(mid.stamp);
                _log(Log::L_DEBUG, CLOGFMT(LogMonitor, "Progress: position [%lld] stamp[%s]. TID=%d, FILE=%s"), mid.position, stampStr.c_str(), tid, _filePath.c_str());
            } else {
                nNoMoreMessage += 1;
                if(nNoMoreMessage % (nNoMoreMessage > 1000 ? 1000 : 100) == 1) {
                    std::string stampStr = time2utc(mid.stamp);
                    _log(Log::L_DEBUG, CLOGFMT(LogMonitor, "No more message from: position [%lld] stamp[%s]. TID=%d, FILE=%s"), mid.position, stampStr.c_str(), tid, _filePath.c_str());
                }
            }
        }
		else
		{
			nFcount += 1;
		}
    }
    } catch (...) {
        _log(Log::L_ERROR, CLOGFMT(LogMonitor, "Got unexpected exception in the monitoring. TID=%d, FILE=%s"), tid, _filePath.c_str());
    }
    _log(Log::L_INFO, CLOGFMT(LogMonitor, "Leave monitoring thread. TID=%d, FILE=%s"), tid, _filePath.c_str());
    mid.source = _filePath; // keep the same as the input
    _rpCache.set(mid);
    return 0;
}

#endif


