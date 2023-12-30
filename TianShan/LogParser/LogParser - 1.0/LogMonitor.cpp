#include "LogMonitor.h"
#include "LogParserManager.h"
#include "LogMessageHandler.h"
#include "TimeConv.h"


LogMonitorRequest::LogMonitorRequest(std::string filepath,LogParserManager& logParserMgr)
			:_log(logParserMgr.getLog())
 			,ThreadRequest(logParserMgr.getThreadPool())
 			,_logParserMgr(logParserMgr)
			,_filepath(filepath)
			,_busyTime(200)
			,_idleTime(1000)
{

}

int LogMonitorRequest::run()
{
	using namespace ZQ::common;
	DWORD tid = ::GetCurrentThreadId();
	_log(Log::L_INFO, CLOGFMT(LogMonitorRequest, "Enter monitoring thread. TID=%d, FILE=%s, CurrentTime=%llu msec."), tid, _filepath.c_str(), ZQ::common::now());
	IRawMessageSource* pMsgSrc = _logParserMgr.getMessageSource(_filepath); 
	if (pMsgSrc == NULL)
	{
		_log(Log::L_ERROR, CLOGFMT(LogMonitorRequest, "Can't find the monitor file. TID=%d, FILE=%s"), tid, _filepath.c_str());
		return -1;
	}
	
	MessageIdentity &mid = pMsgSrc->getMessageIdentity();
	std::vector<char> _buf;
	_buf.resize(4096);
    try {

		if (!pMsgSrc->open(mid))
		{
			_log(Log::L_ERROR, CLOGFMT(LogMonitorRequest, "Open the monitor file error. TID=%d, FILE=%s"), tid, _filepath.c_str());
			_idleTime = 10000;
			_logParserMgr.add(_filepath,(ZQ::common::now()+_idleTime));
			_log(Log::L_INFO, CLOGFMT(LogMonitorRequest, "Leave monitoring thread. TID=%d, FILE=%s, CurrentTime=%llu msec,expiredTime=%llu msec."), tid, _filepath.c_str(), ZQ::common::now(),ZQ::common::now()+_idleTime);
			return -1;
		}

		MessageIdentity openMid = mid;

		int64 timeLimit = ZQ::common::now() + _busyTime; // run at most BUSTYME msec

		while(ZQ::common::now() < timeLimit)
		{
			MessageIdentity newMid;
			char* buf = &(_buf[0]);
			int len = _buf.size();
			int ret = pMsgSrc->fetchNext(buf, &len, newMid);

			if (ret <0)
			{ // error occur
				_log(Log::L_ERROR, CLOGFMT(LogMonitorRequest, "Can't fetch next log message. TID=%d, FILE=%s"), tid, _filepath.c_str());
				break;
			}

			if (ret > 0)
			{
				int64 stampStart = ZQ::common::now();
				_logParserMgr.handle(pMsgSrc->getKey(),buf, ret, newMid);
				//_log(Log::L_DEBUG, CLOGFMT(LogMonitorRequest, "file[%s] processed [%d]bytes took [%d]msec: %lld[%lld] => %lld[%lld]"), _filepath.c_str(), ret, (int) (ZQ::common::now() - stampStart), mid.position, mid.stamp, newMid.position, newMid.stamp);
				//mid = newMid;
				pMsgSrc->SetMessageIdentity(newMid);
				continue;
			}

			// no more new data read in this round
			if (len <= 0) // no data || blank line, ignore
				break;

			if (_buf.size() < len)
			{ // grow buffer and retry
				_log(Log::L_DEBUG, CLOGFMT(LogMonitorRequest, "Need more buffer (%d) to get next log message. current buffer size is %d. TID=%d, FILE=%s"), len, _buf.size(), tid, _filepath.c_str());
				_buf.resize(len);
				buf = &(_buf[0]);
				ret = pMsgSrc->fetchNext(buf, &len, newMid);
				if (ret > 0)
				{
					_logParserMgr.handle(pMsgSrc->getKey(),buf, ret, newMid);
					//mid = newMid;
					pMsgSrc->SetMessageIdentity(newMid);
				}
				else
				{ // error occur
					_log(Log::L_ERROR, CLOGFMT(LogMonitorRequest, "Can't fetch next log message after grow buffer. TID=%d, FILE=%s"), tid, _filepath.c_str());
					break;
				}
			}
		}
		pMsgSrc->close();
		if(openMid.stamp != mid.stamp || openMid.position != mid.position)
		{
			// log the progress
			std::string stampStr = time2utc(mid.stamp);
			_log(Log::L_DEBUG, CLOGFMT(LogMonitorRequest, "Progress: position [%lld] stamp[%s]. TID=%d, FILE=%s"), mid.position, stampStr.c_str(), tid, _filepath.c_str());
		}
		else
		{
			_idleTime = 4000;
			std::string stampStr = time2utc(mid.stamp);
			_log(Log::L_DEBUG, CLOGFMT(LogMonitorRequest, "No more message from: position [%lld] stamp[%s]. TID=%d, FILE=%s"), mid.position, stampStr.c_str(), tid, _filepath.c_str());
		}
		
	}
	catch (...)
	{
		_log(Log::L_ERROR, CLOGFMT(LogMonitor, "Got unexpected exception in the monitoring. TID=%d, FILE=%s"), tid, _filepath.c_str());
	}
	_logParserMgr.add(_filepath,(ZQ::common::now()+_idleTime));
	_log(Log::L_INFO, CLOGFMT(LogMonitorRequest, "Leave monitoring thread. TID=%d, FILE=%s, CurrentTime=%llu msec,expiredTime=%llu msec."), tid, _filepath.c_str(), ZQ::common::now(),ZQ::common::now()+_idleTime);
	return 0;
}

void LogMonitorRequest::final(int retcode,bool bCancelled)
{
	delete this;
}


/*
watchDog::watchDog(LogParserManager& logParserMgr)
		:_log(logParserMgr.getLog())
		,_logParserMgr(logParserMgr)

{
	 _hQuit = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

watchDog::~watchDog()
{
	::SetEvent(_hQuit);
//	_log(ZQ::common::Log::L_INFO, CLOGFMT(watchDog, "~LogMonitor() Request to quit the monitoring thread."));
	waitHandle(-1); // may block here
	::CloseHandle(_hQuit);
}

void watchDog::add(const std::string& filepath,int64 expiredTime)
{
	 ZQ::common::MutexGuard guard(_logfilesLock);
	_logfiles[filepath] = expiredTime;
}

void watchDog::remove(std::string filepath)
{
	ZQ::common::MutexGuard guard(_logfilesLock);
	// step 1: find out the log monitor
	std::map<std::string,int64>::iterator it = _logfiles.find(filepath);
	if(_logfiles.end() == it)
	{ 
		return;
	}
	_logfiles.erase(it);
}


int watchDog::run()
{
	DWORD timeLimit = 0,waitTime = 0;
	while(true)
	{
		if(WAIT_OBJECT_0 == ::WaitForSingleObject(_hQuit, waitTime))
		{ // need quit the thread
			break;
		}
		ZQ::common::MutexGuard guard(_logfilesLock);
		for(std::map<std::string,int64>::iterator it = _logfiles.begin(); it != _logfiles.end();)
		{
			timeLimit = ::GetTickCount() - it->second;
			if (timeLimit <= 0)
			{
// 				if (NULL == _logParserMgr.getLogMonitorItem(it->first))
// 				{
// 					return -1;
// 				}

				try
				{
// 					LogMonitorRequest* cmd = new LogMonitorRequest (_log,it->first,_logParserMgr); 
// 					if (cmd != NULL)
// 						cmd->start(); 
				}
				catch(...)
				{
				}

				_logfiles.erase(it++);
			}
			else
				it++;

			if (waitTime > timeLimit)
			{
				waitTime = timeLimit;
			}
		}

	}
	return 0;
}
*/