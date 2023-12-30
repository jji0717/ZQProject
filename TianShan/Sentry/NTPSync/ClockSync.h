// File Name: ClockSync.h
// Date: 2009-01
// Description: definition of client clock synchronize class.
//              This class can be used to synchronize time with ntp 

#ifndef __CLOCKSYNC_H__
#define __CLOCKSYNC_H__

#include "Log.h"
#include "NativeThreadPool.h"
#include "NTPUDPSocket.h"

#include "NTPUtils.h"
#include "SystemClock.h"

namespace NTPSync
{
	class ClockSync : public ZQ::common::ThreadRequest
	{
	public:

		///constructor
		///@param Pool           : thread pool size
		///@param strNTPServer   : the hostname or IP address of the NTP server
		///@param dwTimeoutOfRun : in milliseconds of this clock sync execution,default is 2 minutes
		///@param nNTPServerPort : the server-side port of the NTP server, default is 123
		/*ClockSync(ZQ::common::NativeThreadPool& Pool, ZQ::common::Log* log, const std::string strNTPServer, 
			const DWORD dwTimeoutOfRun = 2 * 60 * 1000, const DWORD dwMaxOffset = 60 * 60 * 1000, const short sNTPServerPort = 123);*/
		ClockSync(ZQ::common::NativeThreadPool& Pool, ZQ::common::Log* log,
			const uint32 dwTimeoutOfRun = 2 * 60 * 1000, 
			const uint32 dwMaxOffset = 60 * 60 * 1000);
		~ClockSync();
	public:
		///set params to adjust system clock, this method must be call 
		///before call start() if want to set adjust clock params
		///@param dwTimesyncThreshold : in milliseconds,default is 50 milliseconds
		///@param dwMaxTickPercentage : default is 160
		///@param dwMinTickPercentage : default is 20
		///@return void
		void setAdjustClockParams(uint32 dwTimesyncThreshold = 50, uint32 dwMaxTickPercentage = 160, uint32 dwMinTickPercentage = 20);
	protected:
		///synchronize time with ntp server one time
		///@return the return value will also be passed as the thread exit code
		virtual int run(void);

		/// A thread that is self terminating, either by invoking exit() or leaving its run(),
		/// will have this method called. It can be used to self delete the current object assuming
		/// the object was created with new on the heap rather than stack local. You can safe
		/// delete thread via "delete this"in final, but should exit ASAP
		///@param retcode the return value of run()
		///@param bTerminated  true if the request was cancelled by the execution thread
		virtual void final(int retcode =0, bool bCancelled =false);
	private:
		///create log file, determine if local host is same as ntp server,get privilege to set system time, 
		///disable system time daemon
		///@return return true if all above operations success, else return false
		bool initial();

		///send ntp request and get response, store response in _ntpReply
		///@return return true if success, else return false
		bool queryTime();

		///get the time offset in milliseconds between the local clock and the NTP server
		///nOffset = NTP Server clock - local clock
		///@return return the difference in milliseconds, Negative if the local is ahead that of NTP server
		int64 getOffset();

		///get the round trip transmit time in milliseconds
		///@return return round trip transmit time between client and server
		int64 getRoundTrip();

		///adjust system time
		///@param nOffset : time offset in milliseconds to adjust
		///@return return true if success, else return false
		bool adjustClock(int64 nOffset);

		///@return return true if have some error,else return false
		bool checkPacketError();

		///get IP address from string to uint32
		///@param strAddress £ºaddress in form ip or domain
		///@param dwIP : store ip address
		///@return void
		void getIP(const std::string strAddress, uint32& dwIP);

		///get local ip address,mutiple networkinterface will have mutiple ip address
		///@param localIPList : store local ip list
		///@param dwError : error code
		///@return return true if success, else return false
		bool getLocalIPs(std::vector<uint32>& localIPList);

#ifdef ZQ_OS_MSWIN
		///log ntp reply packet
		///@param ntpPacket
		void logNTPPacket(NTP_Packet& ntpPacket, FILETIME& recPacketTime);
#else
		void logNTPPacket(NTP_Packet& ntpPacket,  struct timeval& tval);
#endif
	private:
		ClockSync(const ClockSync& oriClockSync);
		ClockSync& operator=(const ClockSync& oriClockSync);
	private:
		NTPUDPSocket _ntpSocket; // socket object
		SystemClock _ntpSystemClock; // system clock object
		NTP_Packet _ntpReply; // ntp reply packet
#ifdef ZQ_OS_MSWIN
		FILETIME _recFileTime; // the time when ntp reply arrive
#else
#endif
		uint64 _T1, _T2, _T3, _T4;
	private:
		std::string _strNTPServer; // ntp server address
		short _sNTPServerPort; // ntp server listen port
	private:
		ZQ::common::Log* _log; // log pointer
		uint32 _dwInterval; // timeslice to adjust clock
		ZQ::common::NativeThreadPool& _pool;
		uint32 _dwMaxOffset;
	};
} // end for namespa

#endif
