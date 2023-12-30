// File Name: SystemClock.cpp
// Date: 2009-01
// Description: implement of system clock class.
//              This class can be used to adjust system time

#include "SystemClock.h"
#include "TianShanDefines.h"
#include <string>
#ifdef ZQ_OS_LINUX
extern "C"
{
#include <sys/time.h>
#include <errno.h>
}
#endif

#define MOLOG if(_log) (*_log)

using namespace ZQ::common;

namespace NTPSync
{
#ifdef ZQ_OS_MSWIN
	SystemClock::SystemClock()
	:_hExitEvent(NULL),_dwOrigTick(0), _dwTimesyncThreshold(50), _dwInterval(2*60*1000), _dwMaxTickPercentage(160), 
	_dwMinTickPercentage(20), _log(NULL), _bAdjusting(false), _dwMaxOffset(0)
	{
		_hExitEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		::ResetEvent(_hExitEvent);
	}

	SystemClock::~SystemClock()
	{
		if (_hExitEvent)
		{
			CloseHandle(_hExitEvent);
		}
		enableTimeDaemon();
	}

	bool SystemClock::getSystemPrivilege()
	{
		// Get a token for this process.
		int nErrorCode = 0;
		HANDLE hToken;  
		int nReCode = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
		if (0 == nReCode)
		{
			nErrorCode = GetLastError();
			MOLOG(Log::L_ERROR, CLOGFMT(NTPClient, "Fail to get a token for this process. Error code[%d]"), nErrorCode);
			return false;
		}

		// Get the LUID for the set system privilege. 
		TOKEN_PRIVILEGES tkp; 
		nReCode = LookupPrivilegeValue(NULL, SE_SYSTEMTIME_NAME, &tkp.Privileges[0].Luid); 
		if (0 == nReCode)
		{
			nErrorCode = GetLastError();
			MOLOG(Log::L_ERROR, CLOGFMT(NTPClient, "Fail to get a token privileges. Error code[%d]"), nErrorCode);
			return false;
		}
		tkp.PrivilegeCount = 1;  // one privilege to set    
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
		
		// Get the set system time privilege for this process. 
		nReCode = AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
		nErrorCode = GetLastError();
		if (0 == nReCode || nErrorCode != ERROR_SUCCESS)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(NTPClient, "Fail to get privilege to set system time. Error code[%d]"), nErrorCode);
			return false;
		}
		return true;
	}

	bool SystemClock::disTimeDaemon()
	{
		// system clock tick wont change, so we only need to get it once
		int nErrorCode = 0;
		DWORD dwTimeAdjustment = 0;
		BOOL bTimeAdjustmentDisabled = TRUE;
		if (GetSystemTimeAdjustment(
			&dwTimeAdjustment,	// size, in 100-nanosecond units, of a periodic time adjustment 
			(PDWORD)&_dwOrigTick,			// time, in 100-nanosecond units, between periodic time adjustments 
			&bTimeAdjustmentDisabled) == 0) // whether periodic time adjustment is disabled or enabled 
		{
			nErrorCode = GetLastError();
			MOLOG(Log::L_ERROR, CLOGFMT(NTPClient, "Fail to get system clock tick.Error code[%d]"), nErrorCode);
			return false;
		}
		// MOLOG(Log::L_DEBUG, CLOGFMT(NTPClient,"Adjustment %d, tick %d, disabled %d"), dwTimeAdjustment, _dwOrigTick, bTimeAdjustmentDisabled);

		// set adjustment
		if (SetSystemTimeAdjustment(_dwOrigTick, FALSE) == 0)
		{
			nErrorCode = GetLastError();
			MOLOG(Log::L_ERROR, CLOGFMT(NTPClient, "Fail to set system clock.Error code[%d]"), nErrorCode);
			return false;
		}
		return true;
	}

	bool SystemClock::enableTimeDaemon()
	{
		int nErrorCode = 0;
		// restore clock tick speed to before we made the adjustment
		if (0 == SetSystemTimeAdjustment(_dwOrigTick, FALSE))
		{
			nErrorCode = GetLastError();
			MOLOG(Log::L_ERROR, CLOGFMT(NTPClient, "Fail to set system clock.Error code[%d]"), nErrorCode);
			return false;
		}
		// update real time clock
		SYSTEMTIME st;
		GetSystemTime(&st);
		if (SetSystemTime(&st) == 0) //SE_SYSTEMTIME_NAME 
		{
			nErrorCode = GetLastError();
			MOLOG(Log::L_ERROR, CLOGFMT(NTPClient, "Fail to set system time. Error code[%d]"), nErrorCode);
			return false;
		}
		return true;
	}

	bool SystemClock::adjustClock(int64 nOffSet)
	{
		int nErrorCode = 0;
		std::string strTips = "";
		int64 iDifference = -nOffSet;
		int64 iabsDiff = iDifference;
		if (iDifference < 0) // our time is behind
		{
			iabsDiff = -iDifference;
			strTips = "behind";
		}
		else
		{
			strTips = "ahead";
		}

		// we are within range to be considered time accurate
		if (iabsDiff < _dwTimesyncThreshold)
		{
			MOLOG(Log::L_INFO, CLOGFMT(NTPClient,"adjustClock: [%lld] milliseconds, within threshold [%u] milliseconds"), iabsDiff, _dwTimesyncThreshold);
			return true;
		}

		if (iabsDiff > _dwMaxOffset) // adjust clock directly
		{
			MOLOG(Log::L_WARNING, CLOGFMT(NTPClient, 
				"adjust clock directly, time diff ["FMT64"] max allowed ["FMT64"]"),
				iabsDiff, _dwMaxOffset);

			FILETIME ft;
			GetSystemTimeAsFileTime(&ft);

			ULONGLONG T0;
			FILETIME_to_UNIT64(ft, T0); // current time 

			ULONGLONG T1, T2;
			T2 = iabsDiff;
			T2 = T2 * 10000; // to nanoseconds
			if(iDifference < 0) // our time is behind 
			{
				T1 = T0 + T2;	
			}
			else
			{
				T1 = T0 - T2;	
			}

			FILETIME ft2;
			UINT64_to_FILTETIME(T1, ft2);

			SYSTEMTIME st;
			FileTimeToSystemTime(&ft2, &st);
			if (!SetSystemTime(&st))
			{
				MOLOG(Log::L_INFO, CLOGFMT(NTPClient, "Fail to set clock time directly "));
				return false;
			}
			MOLOG(Log::L_INFO, CLOGFMT(NTPClient, "Success to set clock time directly"));
		}
		else // adjust clock bit by bit
		{
			// set adjust tick
			DWORD dwSleep = 0; // in milliseconds
			DWORD dwAdjustment = 0; // in nanoseconds
			if (iDifference > 0) // our time is ahead
			{
				dwSleep = _dwInterval;
				if (((double)iDifference / _dwInterval) > (1. - (_dwMinTickPercentage / 100.)))
				{
					// don't set clock tick too slow or still, no slower than 
					// MinTickSpeedPercentage, by default 20%
					// iDifference = (long)(_dwInterval * (1. - (_dwMinTickPercentage / 100.)));
					iDifference = _dwInterval * (1. - (_dwMinTickPercentage / 100.));
				}
				dwAdjustment = (DWORD) (_dwOrigTick * (1. - (double)iDifference / _dwInterval));
			}
			else // our time is behind
			{
				dwSleep = _dwInterval;
				if (((double)(-iDifference) / _dwInterval) > ((_dwMaxTickPercentage / 100.) - 1))
				{
					// don't set clock tick too fast, no faster than 
					// MaxTickSpeedPercentage, by default 160%
					// iDifference = (long)(_dwInterval * (1. - (_dwMaxTickPercentage / 100.)));
					iDifference = _dwInterval * (1. - (_dwMaxTickPercentage / 100.));
				}
				dwAdjustment = (DWORD) (_dwOrigTick * (1. - (double)iDifference / _dwInterval));
			}
			MOLOG(Log::L_INFO, CLOGFMT(NTPClient, "Our time is [%lld] milliseconds [%s],Success adjust clock [%lld] in milliseconds one time"), iabsDiff, strTips.c_str(), iDifference);

			// adjust time 
			if (0 == SetSystemTimeAdjustment(dwAdjustment, FALSE))
			{
				nErrorCode = GetLastError();
				MOLOG(Log::L_ERROR, CLOGFMT(NTPClient, "Fail to set system clock.Error code[%lu]"), nErrorCode);
				return true;
			}
			_bAdjusting = true;
			DWORD dwWait = ::WaitForSingleObject(_hExitEvent, dwSleep); // sleep for the time adjustment period
			_bAdjusting = false;
		}

		// enable system time daemon
		return enableTimeDaemon();
	}
	void SystemClock::abortCurrentAdjusting()
	{
		if (_bAdjusting)
		{
			SetEvent(_hExitEvent);
			SYS::sleep(1000);
		}
	}
#else
	SystemClock::SystemClock()
	:_dwOrigTick(0), _dwTimesyncThreshold(50), _dwInterval(2*60*1000), _dwMinTickPercentage(20), 
	_dwMaxTickPercentage(160),  _log(NULL), _bAdjusting(false), _dwMaxOffset(0)
	{
	}

	SystemClock::~SystemClock()
	{
	}
	bool SystemClock::checkAccess()
	{
		if (0 != getuid() && 0 != geteuid())
			return false;
		return true;
	}
	bool SystemClock::adjustClock(int64 nOffSet)
	{
		std::string strTips = "";
		int64 iDifference = -nOffSet;
		int64 iabsDiff = iDifference;
		if (iDifference < 0) // our time is behind
		{
			iabsDiff = -iDifference;
			strTips = "behind";
		}
		else
		{
			strTips = "ahead";
		}

		// we are within range to be considered time accurate
		if (iabsDiff < _dwTimesyncThreshold)
		{
			MOLOG(Log::L_DEBUG, CLOGFMT(NTPClient,"adjustClock: %d milliseconds, within threshold %d milliseconds"),
				 iabsDiff, _dwTimesyncThreshold);
			return true;
		}

		if (iabsDiff > _dwMaxOffset) // adjust clock directly
		{
			MOLOG(Log::L_WARNING, CLOGFMT(NTPClient, 
					"adjust clock directly, time diff ["FMT64"] max allowed ["FMT64"]"),
					iabsDiff, _dwMaxOffset);

			struct timeval tval;
			int64 nowT = ZQTianShan::now();
			if(nowT == 0)
			{
				MOLOG(Log::L_ERROR,CLOGFMT(NTPClient,"adjustClock() gettimeofday failed error code [%d]"),errno);
				return false;	
			}
			nowT += nOffSet;
			tval.tv_sec = nowT/1000;
			tval.tv_usec = (nowT%1000)*1000;
			int rt = settimeofday(&tval,NULL);
			if(rt != 0)
			{
				MOLOG(Log::L_ERROR,CLOGFMT(NTPClient,"adjustClock() settimeofday failed error code[%d]"),errno);
				return false;
			}
		}
		else
		{
			struct timeval adjtval, oldtval;
			adjtval.tv_sec = nOffSet/1000;
			adjtval.tv_usec = (nOffSet%1000)*1000l;
			int rt = adjtime(&adjtval, &oldtval);
			if(rt != 0)
			{	
				MOLOG(Log::L_ERROR,CLOGFMT(NTPClient,"adjustClock() adjtime call  failed error code[%d]"),errno);
				return false;	
			}
			MOLOG(Log::L_INFO,CLOGFMT(NTPClient,"adjustClock() will gradually adjusts system time about change  0.002s every second "));
			
		}
		MOLOG(Log::L_DEBUG, CLOGFMT(NTPClient, "adjustClock() Our time is %d milliseconds %s,success adjust clock"),
			iabsDiff,strTips.c_str());

		return true;;
	}

		void SystemClock::abortCurrentAdjusting()
	{
		if (_bAdjusting)
		{
			_hExitEvent.signal();
			SYS::sleep(1000);
		}
	}

#endif



	void SystemClock::SetAdjustClockParams(uint32 dwInterval, uint32 dwTimesyncThreshold, uint32 dwMaxTickPercentage, uint32 dwMinTickPercentage, uint32 dwMaxOffset)
	{
		_dwInterval = dwInterval;
		_dwTimesyncThreshold = dwTimesyncThreshold;
		_dwMaxTickPercentage = dwMaxTickPercentage;
		_dwMinTickPercentage = dwMinTickPercentage;
		_dwMaxOffset = dwMaxOffset;
	}

	void SystemClock::setLog(ZQ::common::Log* pLog)
	{
		_log = pLog;
	}
}
