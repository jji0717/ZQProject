// File Name: SystemClock.h
// Date: 2009-01
// Description: definition of system clock class.
//              This class can be used to adjust system time

#ifndef __SYSTEMCLOCK_H__
#define __SYSTEMCLOCK_H__

#include "Log.h"
#include "NTPUtils.h"
#include "SystemUtils.h"

namespace NTPSync
{
	class SystemClock
	{
	public:
		SystemClock();
		~SystemClock();
	public:
#ifdef ZQ_OS_MSWIN
		///get the privilege to set system time
		///@return return true if success, else return false
		bool getSystemPrivilege();

		///disable system time daemon
		///@return return true if success, else return false
		bool disTimeDaemon();

		///enable system time daemon
		///@return return true if success, else return false
		bool enableTimeDaemon();
#else
		//check be superuser,return true is 
		bool checkAccess();
#endif
		///adjust clock,  -nOffSet = client - ntp server
		///@param nOffset[int] : in milliseconds unit
		///@return return true if success, else return false
		bool adjustClock(int64 nOffset);

		///set log object, this method must be called first after create object
		///@param pLog [in]: pointer to log object
		///@return void   
		void setLog(ZQ::common::Log* pLog);

		///@Function  set params to adjust system clock, this method must be call before call adjustClock()
		///@param dwInterval (in milliseconds unit) [in]: interval to adjust clock each time
		///@param dwTimesyncThreshold (in milliseconds unit)[in] : if time offset lower than
		///dwTimesyncThreshold, it doesn't need to sync clock with ntp server.
		///@param dwMaxTickPercentage[in]
		///@param dwMinTickPercentage[in]
		///@return void  
		void SetAdjustClockParams(uint32 dwInterval, uint32 dwTimesyncThreshold, uint32 dwMaxTickPercentage, uint32 dwMinTickPercentage, uint32 dwMaxOffset);

		///
		void abortCurrentAdjusting();
	public:
#ifdef ZQ_OS_MSWIN
		HANDLE _hExitEvent;
#else
		SYS::SingleObject _hExitEvent;
#endif
	private:
		uint32 _dwOrigTick; // in 100-nansecond, clock interupt interval
		uint32 _dwTimesyncThreshold; // default is 50 milliseconds
		uint32 _dwInterval; // default is 2 * 60 * 1000 milliseconds
		uint32 _dwMinTickPercentage; // default is 20
		uint32 _dwMaxTickPercentage; // default is 160
		ZQ::common::Log* _log; // log pointer
		bool _bAdjusting;
		uint32 _dwMaxOffset;
	};
}

#endif
