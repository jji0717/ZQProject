#include "NTPClient.h"
#include "TianShanDefines.h"

#define NTP_AdjustTimeout_Min 10000 // 10 sec
#define NTP_AdjustTimeout_Max 120000 // 2 min
#define NTP_SyncInterval_Min 300000 // 5 min
#define NTP_SyncInterval_Max 3600000 // 1 h
#define ADJUST_TO_RANGE(val, low, high) (val < low ? low : (val > high ? high : val))
namespace NTPSync
{
    /*NTPClient::NTPClient(ZQ::common::Log* log, ZQ::common::NativeThreadPool& pool, const std::string& strNTPServer, short sNTPServerPort, uint32 dwTimeoutOfRun, int syncInterval, uint32 dwMaxOffset)
        :_ntpClockSync(NULL), _pool(pool), _log(log), _strNTPServer(strNTPServer)*/
	NTPClient::NTPClient(ZQ::common::Log *log, ZQ::common::NativeThreadPool &pool, int32 dwTimeoutOfRun , int32 syncInterval, int32 dwMaxOffset)
		:_ntpClockSync(NULL), _pool(pool), _log(log)
	{
        _dwTimeoutOfRun = ADJUST_TO_RANGE(dwTimeoutOfRun, NTP_AdjustTimeout_Min,NTP_AdjustTimeout_Max);
        _syncInterval = ADJUST_TO_RANGE(syncInterval, NTP_SyncInterval_Min, NTP_SyncInterval_Max);

		if (dwMaxOffset < 300000) // adjust dwMaxOffset
		{
			_dwMaxOffset = 300000; // 5 minutes
		}
		else
		{
			_dwMaxOffset = dwMaxOffset;
		}

        _nextBookedSync = ZQTianShan::now();
        (*_log)(ZQ::common::Log::L_INFO, CLOGFMT(NTPClient, "NTPClient initialized. timeout=%d, interval=%d"), _dwTimeoutOfRun, _syncInterval);
	}

	NTPClient::~NTPClient()
	{
		stop();
	}
	
	bool NTPClient::start()
	{
		_ntpClockSync = new NTPSync::ClockSync(_pool, _log, _dwTimeoutOfRun, _dwMaxOffset);
		if (NULL == _ntpClockSync)
		{
			(*_log)(ZQ::common::Log::L_INFO, CLOGFMT(NTPClient, "start() Failed to create Clock Sync Object"));
			return false;
		}

        _nextBookedSync = ZQTianShan::now() + _syncInterval;
		return _ntpClockSync->start();
	}

	bool NTPClient::keepWorking()
	{
		if(NULL == _ntpClockSync)
        {
            return false;
        }

		if (!_ntpClockSync->isRunning())
		{
            if(_nextBookedSync <= ZQTianShan::now())
            { // time to sync
			    delete _ntpClockSync;
                _ntpClockSync = new NTPSync::ClockSync(_pool, _log, _dwTimeoutOfRun, _dwMaxOffset);
		        if (NULL == _ntpClockSync)
		        {
					(*_log)(ZQ::common::Log::L_INFO, CLOGFMT(NTPClient, "keepWorking() Failed to create Clock Sync Object"));
			        return false;
		        }
                if(_ntpClockSync->start())
                { // sync is started, book the next
                    _nextBookedSync = ZQTianShan::now() + _syncInterval;
                    return true;
                }
                else
                { // failed to start
                    return false;
                }
            }
            else
            { // no need to sync
                return true;
            }
		}
        else
        {
            return true;
        }
	}

	void NTPClient::stop()
	{
        if(_ntpClockSync)
        {
		    delete _ntpClockSync;
		    _ntpClockSync = NULL;
        }
	}
}

