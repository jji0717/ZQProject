#include "HammerMonitor.h"

TimeoutCmd::TimeoutCmd(HammerMonitor& monitor, Hammer& hammer, uint seqIdx)
:ThreadRequest(monitor._pool), _hammer(hammer), _seq(seqIdx) {
}

int TimeoutCmd::run() {
	_hammer.OnTimer(_seq);
	return 0;
}

HammerMonitor::HammerMonitor(ZQ::common::Log& log, ZQ::common::NativeThreadPool& pool, Hammer& hammer)
:_log(log), _pool(pool), _hammer(hammer), _nextWakeup(0), _quit(false) {
}

HammerMonitor::~HammerMonitor() {
	_quit = true;
	wakeup();
	SYS::sleep(1000);
}

int HammerMonitor::run() {

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HammerMonitor, "watchdog start running"));

	while(!_quit) {

		int64 now = ZQ::common::now();
		std::vector<uint> timedoutSessions;
		{	
			ZQ::common::MutexGuard gd(_lock);
			{
				static size_t lastSize = 0;
				size_t watchSize = _expires.size();
				if (watchSize != lastSize) {
					_log(ZQ::common::Log::L_DEBUG, CLOGFMT(HammerMonitor, "[%d] object(s) under watching"), watchSize);
				}
				lastSize = watchSize;
			}

			_nextWakeup = now + 5000;

			ExpireSessions::const_iterator iter = _expires.begin();
			for(; iter != _expires.end(); ++iter) {
				if(iter->second <= now) {
					timedoutSessions.push_back(iter->first);
				}				
				else {
					_nextWakeup = (_nextWakeup > iter->second) ? iter->second : _nextWakeup;	
				}
			}

			std::vector<uint>::const_iterator iter2 = timedoutSessions.begin();
			for(; iter2 != timedoutSessions.end(); ++iter2) {
				_expires.erase(*iter2);
			}
		}

		if(_quit) {
			break;
		}

		std::vector<uint>::const_iterator iter = timedoutSessions.begin();
		for(; iter != timedoutSessions.end(); ++iter) {
			(new TimeoutCmd(*this, _hammer, *iter))->start();
		}

		if(_quit) {
			break;
		}

		int64 sleep = _nextWakeup - ZQ::common::now();
		if(sleep <= 0) {
			continue;
		}

		_handle.wait(sleep); 
	}

	_log(ZQ::common::Log::L_WARNING, CLOGFMT(TimerWatchDog, "watchdog stops watching"));

	return (0);
}

void HammerMonitor::wakeup() {
	_handle.signal();
}

void HammerMonitor::watch(size_t seqIdx, int64 timeout) {

	ZQ::common::MutexGuard gd(_lock);
	_expires[seqIdx] = timeout;
	
	if (timeout < _nextWakeup) {
		wakeup();
	}
}


// vim: ts=4 sw=4 nu bg=dark
