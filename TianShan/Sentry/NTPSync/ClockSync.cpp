// File Name: ClockSync.h
// Date: 2009-01
// Description: implement of client clock synchronize class.
//              This class can be used to synchronize time with ntp server

#include <algorithm>

#include "FileLog.h"
#include "ClockSync.h"
#include "SentryConfig.h"
#include "SystemUtils.h"
#ifdef ZQ_OS_LINUX
extern "C"
{
#include <time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
}
#endif

#define MOLOG if(_log) (*_log)

using namespace ZQ::common;

namespace NTPSync
{
	/*ClockSync::ClockSync(NativeThreadPool& Pool, Log* log, const std::string strNTPServer, const uint32 dwTimeoutOfRun, const uint32 dwMaxOffset, const short sNTPServerPort)
	:ThreadRequest(Pool), _T1(0), _T2(0), _T3(0), _T4(0), _strNTPServer(strNTPServer), _sNTPServerPort(sNTPServerPort), 
	_log(log), _dwInterval(dwTimeoutOfRun), _pool(Pool), _dwMaxOffset(dwMaxOffset)*/

	ClockSync::ClockSync(NativeThreadPool& Pool, Log* log, const uint32 dwTimeoutOfRun, const uint32 dwMaxOffset)
		:ThreadRequest(Pool), _T1(0), _T2(0), _T3(0), _T4(0), _log(log), _dwInterval(dwTimeoutOfRun), 
		_pool(Pool), _dwMaxOffset(dwMaxOffset)
	{
		_ntpSystemClock.setLog(_log);
		setAdjustClockParams();
	}

	ClockSync::~ClockSync()
	{
        MOLOG(Log::L_DEBUG, CLOGFMT(ClockSync,"~ClockSync() stop the current adjusting"));
		_ntpSystemClock.abortCurrentAdjusting();
        while(isRunning())
        {
		SYS::sleep(1000);
        }
        MOLOG(Log::L_DEBUG, CLOGFMT(ClockSync,"~ClockSync() quit the sync thread"));
	}

	void ClockSync::final(int retcode, bool bCancelled)
	{
	}

	void ClockSync::setAdjustClockParams(uint32 dwTimesyncThreshold, uint32 dwMaxTickPercentage, uint32 dwMinTickPercentage)
	{
		_ntpSystemClock.SetAdjustClockParams(_dwInterval, dwTimesyncThreshold, dwMaxTickPercentage, dwMinTickPercentage, _dwMaxOffset);
	}

	bool ClockSync::initial()
	{
		if (gSentryCfg.ntpClientCfg.timeServerDatas.empty())
		{
			MOLOG(Log::L_ERROR, CLOGFMT(ClockSync, "NTP server address list is empty"));
			return false;
		}
		if (gSentryCfg.ntpClientCfg.timeServerIndex >= gSentryCfg.ntpClientCfg.timeServerDatas.size())
		{
			gSentryCfg.ntpClientCfg.timeServerIndex = 0;
		}
		_strNTPServer = gSentryCfg.ntpClientCfg.timeServerDatas[gSentryCfg.ntpClientCfg.timeServerIndex].timeServerAddress;
		_sNTPServerPort = gSentryCfg.ntpClientCfg.timeServerDatas[gSentryCfg.ntpClientCfg.timeServerIndex].timeServerPort;
		if (_strNTPServer.empty())
		{
			gSentryCfg.ntpClientCfg.timeServerIndex++;
			MOLOG(Log::L_ERROR, CLOGFMT(ClockSync, "NTP server address is empty"));
			return false;
		}
		//get local host ip
		std::vector<uint32> localIPList;
		if (!getLocalIPs(localIPList))
		{
			MOLOG(Log::L_ERROR, CLOGFMT(ClockSync, "Fail to get local host IP."));
			return false;
		}
		// get ntp server ip
		uint32 dwNTPServerIP = 0;
		getIP(_strNTPServer, dwNTPServerIP);
		if((INADDR_NONE == dwNTPServerIP) || INADDR_ANY == dwNTPServerIP)
		{
			gSentryCfg.ntpClientCfg.timeServerIndex++;
			MOLOG(Log::L_ERROR, CLOGFMT(ClockSync, "Fail to get Server IP."));
			return false;
		}

		// determine if local host ip is same as ntp server ip
		if (localIPList.end() != std::find(localIPList.begin(), localIPList.end(), dwNTPServerIP))
		{
			gSentryCfg.ntpClientCfg.timeServerIndex++;
			MOLOG(Log::L_DEBUG, CLOGFMT(ClockSync,"NTP server is same as local host, it doesn't need synchronize time"));
			return false;
		}
		MOLOG(Log::L_DEBUG, CLOGFMT(ClockSync, "Sync with NTP Server at [%s : %d]"), _strNTPServer.c_str(), _sNTPServerPort);
#ifdef ZQ_OS_MSWIN
		// get privilege to set system time
		if (!_ntpSystemClock.getSystemPrivilege())
		{
			MOLOG(Log::L_ERROR, CLOGFMT(ClockSync, "Fail to get privilege to adjust system time"));
			return false;
		}
		// disabel sytem time daemon
		if (!_ntpSystemClock.disTimeDaemon())
		{
			MOLOG(Log::L_ERROR, CLOGFMT(ClockSync, "Fail to disable system time daemon"));
			return false;
		}
#else
		if (!_ntpSystemClock.checkAccess())
		{
			MOLOG(Log::L_ERROR, CLOGFMT(ClockSync, "check access failed,set system time need superuser"));
			return false;	
		}
#endif

		_ntpSocket.setCompletion(false); // set as un-block socket
		return true;
	}

	void ClockSync::getIP(const std::string strAddress, uint32& dwIP)
	{
		struct hostent* pHostent = gethostbyname(strAddress.c_str());
		if (NULL == pHostent)
		{
			dwIP = inet_addr(strAddress.c_str());
		}
		else
		{
			dwIP = *((unsigned long*)(pHostent->h_addr));
		}
	}

	bool ClockSync::getLocalIPs(std::vector<uint32>& localIPList)
	{
		// get host name
		char szLocal[MAX_PATH];
		memset(szLocal, 0, sizeof(szLocal));
		if (0 != gethostname(szLocal, MAX_PATH))
		{
			return false;
		}

		// get local ip addresses
		struct hostent* pHostent = gethostbyname(szLocal); // need to be replaced with...
		if (NULL == pHostent)
		{
			return false;
		}

		// stroe local ip address into localIPList
		uint32 dwLocalIP;
		char** pAddress = pHostent->h_addr_list;
		while (*pAddress != NULL)
		{
			dwLocalIP = *((unsigned long*)(*pAddress));
			localIPList.push_back(dwLocalIP);
			pAddress++;
		}
		return true;
	}

#ifdef ZQ_OS_MSWIN
	void ClockSync::logNTPPacket(NTP_Packet& ntpPacket, FILETIME& recPacketTime)
	{
		
		// translate to local represent
		NTP_Time nt_o, nt_rx, nt_tx;
		nt_o.dwIntSec = ntohl(ntpPacket.originate_timestamp_seconds);
		nt_o.dwFracSec=	ntohl(ntpPacket.originate_timestamp_fractions);
		nt_rx.dwIntSec = ntohl(ntpPacket.receive_timestamp_seconds);
		nt_rx.dwFracSec = ntohl(ntpPacket.receive_timestamp_fractions);
		nt_tx.dwIntSec = ntohl(ntpPacket.transmit_timestamp_seconds);
		nt_tx.dwFracSec = ntohl(ntpPacket.transmit_timestamp_fractions);
		
		// get int64 from ntp time, in nano seconds
	//	ULONGLONG T1, T2, T3;
		getInt64FromNTPTime(nt_o, _T1);
		getInt64FromNTPTime(nt_rx, _T2);
		getInt64FromNTPTime(nt_tx, _T3);
		FILETIME_to_UNIT64(recPacketTime, _T4);

		// get system file time
		FILETIME ft1, ft2, ft3;
		UINT64_to_FILTETIME(_T1, ft1);
		UINT64_to_FILTETIME(_T2, ft2);
		UINT64_to_FILTETIME(_T3, ft3);

		// get local file time
		FILETIME localft1, localft2, localft3, localft4;
		FileTimeToLocalFileTime(&ft1, &localft1);
		FileTimeToLocalFileTime(&ft2, &localft2);
		FileTimeToLocalFileTime(&ft3, &localft3);
		FileTimeToLocalFileTime(&recPacketTime, &localft4);

		// get system time
		SYSTEMTIME st1, st2, st3, st4;
		FileTimeToSystemTime(&localft1, &st1);
		FileTimeToSystemTime(&localft2, &st2);
		FileTimeToSystemTime(&localft3, &st3);
		FileTimeToSystemTime(&localft4, &st4);
		long nRoundTrip = (long)(getRoundTrip());
		MOLOG(Log::L_DEBUG, CLOGFMT(ClockSync, "Success to query time from [%s : %d], Round Trip delay is %d, Request orginated [%d/%d/%d %d:%d:%d.%d],Request received [%d/%d/%d %d:%d:%d.%d], Reply originated [%d/%d/%d %d:%d:%d.%d],  Reply received [%d/%d/%d %d:%d:%d.%d]"),
			_strNTPServer.c_str(), _sNTPServerPort, nRoundTrip,
			st1.wMonth, st1.wDay, st1.wYear, st1.wHour, st1.wMinute, st1.wSecond, st1.wMilliseconds,
			st2.wMonth, st2.wDay, st2.wYear, st2.wHour, st2.wMinute, st2.wSecond, st2.wMilliseconds,
			st3.wMonth, st3.wDay, st3.wYear, st3.wHour, st3.wMinute, st3.wSecond, st3.wMilliseconds,
			st4.wMonth, st4.wDay, st4.wYear, st4.wHour, st4.wMinute, st4.wSecond, st4.wMilliseconds);
	}

#else
	void ClockSync::logNTPPacket(NTP_Packet& ntpPacket, struct timeval& tval)
	{
		// translate to local represent
		NTP_Time nt_o, nt_rx, nt_tx;
		nt_o.dwIntSec = ntpPacket.originate_timestamp_seconds;
		nt_o.dwFracSec=	ntpPacket.originate_timestamp_fractions;
		nt_rx.dwIntSec = ntpPacket.receive_timestamp_seconds;
		nt_rx.dwFracSec = ntpPacket.receive_timestamp_fractions;
		nt_tx.dwIntSec = ntpPacket.transmit_timestamp_seconds;
		nt_tx.dwFracSec = ntpPacket.transmit_timestamp_fractions;

		//convert ntptime to uint64
		getInt64FromNTPTime(nt_o, _T1);
		getInt64FromNTPTime(nt_rx, _T2);
		getInt64FromNTPTime(nt_tx, _T3);
		getInt64FromTimeval(tval, _T4);

		//convert uint64 to timvval struct
		struct timeval tvalOrig,tvalRecv,tvalTras,tvalGet;
		getTimevalFromInt64(_T1,tvalOrig);	
		getTimevalFromInt64(_T2,tvalRecv);	
		getTimevalFromInt64(_T3,tvalTras);	
		getTimevalFromInt64(_T4,tvalGet);	
		
		struct tm tm1,tm2,tm3,tm4;
		localtime_r(&tvalOrig.tv_sec,&tm1);
		localtime_r(&tvalRecv.tv_sec,&tm2);
		localtime_r(&tvalTras.tv_sec,&tm3);
		localtime_r(&tvalGet.tv_sec,&tm4);

		long nRoundTrip = (long)(getRoundTrip());
		MOLOG(Log::L_DEBUG, CLOGFMT(ClockSync, "Success to query time from [%s : %d], Round Trip delay is %d, Request orginated [%d/%d/%d %02d:%02d:%02d.%03d,%lld],Request received [%d/%d/%d %02d:%02d:%02d.%03d,%lld], Reply originated [%d/%d/%d %02d:%02d:%02d.%03d,%lld],  Reply received [%d/%d/%d %02d:%02d:%02d.%03d,%lld]"),
			_strNTPServer.c_str(), _sNTPServerPort, nRoundTrip,
			tm1.tm_mon+1, tm1.tm_mday, tm1.tm_year+1900, tm1.tm_hour, tm1.tm_min, tm1.tm_sec, tvalOrig.tv_usec/1000,_T1,
			tm2.tm_mon+1, tm2.tm_mday, tm2.tm_year+1900, tm2.tm_hour, tm2.tm_min, tm2.tm_sec, tvalRecv.tv_usec/1000,_T2,
			tm3.tm_mon+1, tm3.tm_mday, tm3.tm_year+1900, tm3.tm_hour, tm3.tm_min, tm3.tm_sec, tvalTras.tv_usec/1000,_T3,
			tm4.tm_mon+1, tm4.tm_mday, tm4.tm_year+1900, tm4.tm_hour, tm4.tm_min, tm4.tm_sec, tvalGet.tv_usec/1000,_T4);
	
	}
#endif

	bool ClockSync::queryTime()
	{
		// get system time as ntp time
		NTP_Time nt;
		getSystemTimeAsNTPTime(nt);

		// set ntp request packet
		NTP_Packet ntpRequest; // ntp request packet
		memset(&ntpRequest, 0, sizeof(ntpRequest));
		ntpRequest.Control_Word = htonl(0x0B000000); // 00, 001, 011, 0x00, 0x00, 0x00
		ntpRequest.originate_timestamp_seconds = nt.dwIntSec;
		ntpRequest.originate_timestamp_fractions = nt.dwFracSec;

		// send ntp request packet
		InetAddress ntpInetAddress;
		ntpInetAddress.setAddress(_strNTPServer.c_str());
		_ntpSocket.setPeer(ntpInetAddress, _sNTPServerPort);
		int nSend = _ntpSocket.send((char*)&ntpRequest, sizeof(ntpRequest));
		if (nSend != sizeof(ntpRequest))
		{
			gSentryCfg.ntpClientCfg.timeServerIndex++;
			MOLOG(Log::L_ERROR, CLOGFMT(ClockSync, "Fail to send ntp request to %s"), _strNTPServer.c_str());
			return false;
		}

		// get ntp response packet
		memset(&_ntpReply, 0, sizeof(_ntpReply));

		int nReceive = _ntpSocket.receiveTimeout((char*)&_ntpReply, sizeof(_ntpReply), 5000);
		if (nReceive != sizeof(_ntpReply))
		{
			gSentryCfg.ntpClientCfg.timeServerIndex++;
			MOLOG(Log::L_ERROR, CLOGFMT(ClockSync, "Fail to receive ntp reply from %s"), _strNTPServer.c_str());
			return false;
		}

#ifdef ZQ_OS_MSWIN
		GetSystemTimeAsFileTime(&_recFileTime);
		_ntpReply.originate_timestamp_seconds = ntpRequest.originate_timestamp_seconds;
		_ntpReply.originate_timestamp_fractions = ntpRequest.originate_timestamp_fractions;
		logNTPPacket(_ntpReply, _recFileTime);
#else
		struct timeval tval;
		gettimeofday(&tval,NULL);
		_ntpReply.originate_timestamp_seconds = ntpRequest.originate_timestamp_seconds;
		_ntpReply.originate_timestamp_fractions = ntpRequest.originate_timestamp_fractions;
		logNTPPacket(_ntpReply, tval);	
#endif
		return true;
	}

	int64 ClockSync::getOffset()
	{
		int64 tx = _T2 - _T1;
		int64 ty = _T3 - _T4;
		int64 nOffset = (tx + ty) /2;
		nOffset /= 10000;
		return nOffset;
	}

	int64 ClockSync::getRoundTrip()
	{
		int64 nRoundTirp = (_T4 - _T1) - (_T3 - _T2);
		return (nRoundTirp / 10000);
	}

	bool ClockSync::checkPacketError()
	{
		int32 nControlWord = ntohl(_ntpReply.Control_Word);
		// check LI field
		int32 nLI = (nControlWord >> 30) & 0x00000003;
		if (3 == nLI)
		{
			return true;
		}
		// check stratum field
		int32 nStratum = (nControlWord >> 16) & 0x000000FF;
		if (nStratum < 1 || nStratum >15)
		{
			return true;
		}
		//check transmit field
		NTP_Time transmitTime;
		transmitTime.dwIntSec = ntohl(_ntpReply.transmit_timestamp_seconds);
		transmitTime.dwFracSec = ntohl(_ntpReply.transmit_timestamp_fractions);
		if (0 == transmitTime.dwIntSec && 0 == transmitTime.dwFracSec)
		{
			return true;
		}
		return false;
	}

	int ClockSync::run()
	{
		if (!initial())
		{
			return 1;
		}
		if (!queryTime())
		{
			MOLOG(Log::L_ERROR, CLOGFMT(ClockSync, "Fail to query time one time"));
			return 1;
		}
		if (checkPacketError())
		{
			MOLOG(Log::L_DEBUG, CLOGFMT(ClockSync, "the server has not synchronized to a valid timing source within the last 24 hours"));
			return 1;
		}
		if (!adjustClock(getOffset()))
		{
			MOLOG(Log::L_ERROR, CLOGFMT(ClockSync, "Fail to adjust time one time"));
			return 1;
		}
		return 0;
	}

	bool ClockSync::adjustClock(int64 nOffset)
	{
		return _ntpSystemClock.adjustClock(nOffset);
	}
} // end for ZQ
