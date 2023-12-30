#ifndef __NTP_CLIENT_H__
#define __NTP_CLIENT_H__

#include "ClockSync.h"

namespace NTPSync
{
	class NTPClient
	{
	public:
		NTPClient(ZQ::common::Log* log, 
			ZQ::common::NativeThreadPool& pool, 
			int32 dwTimeoutOfRun = 60 * 1000, 
			int32 syncInterval = 600 * 1000, 
			int32 dwMaxOffset = 60 * 60 * 1000);

		//NTPClient(ZQ::common::Log* log, 
		//	ZQ::common::NativeThreadPool& pool, 
		//	const std::string& strNTPServer, 
		//	short sNTPServerPort = 123, 
		//	uint32 dwTimeoutOfRun = 60 * 1000, 
		//	int syncInterval = 600 * 1000, 
		//	uint32 dwMaxOffset = 60 * 60 * 1000);
		~NTPClient();
	public:
		bool start();
		bool keepWorking();
		void stop();
	private:
		NTPSync::ClockSync* _ntpClockSync;

		ZQ::common::NativeThreadPool& _pool;
		ZQ::common::Log* _log;
		uint32 _dwTimeoutOfRun;

        int _syncInterval;
        int64 _nextBookedSync;
		uint32 _dwMaxOffset;
	};
}

#endif

