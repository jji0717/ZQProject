// File Name: NTPServer.cpp
// Date: 2009-01
// Description: implement of ntp server class.
// This class privide time synchronize service and it is support ntp version3 but not support NTP authentication mechanism

#include "FileLog.h"
#include "NTPServer.h"
#ifdef ZQ_OS_LINUX
extern "C"
{
#include <errno.h>
#include <sys/time.h>
}
#endif

#define MOLOG if(_log) (*_log)

using namespace ZQ::common;

namespace NTPSync
{
	NTPServer::NTPServer(Log* log, const std::string strBindAddr, const short sBindPort, int stacksize)
	:NativeThread(stacksize), _pServerSocket(NULL), _log(log), _recPeerPort(0), _strBindAddr(strBindAddr), 
	_sBindPort(sBindPort), _bQuit(false), _errorNum(0)
	{
	}

	NTPServer::~NTPServer(void)
	{
        MOLOG(Log::L_INFO, CLOGFMT(NTPServer, "~NTPServer() Quiting NTP Server..."));
		if(_pServerSocket)
		{
			delete _pServerSocket;
			_pServerSocket = NULL;
		}
#ifdef ZQ_OS_MSWIN
		SetEvent(_quitHandle);
		waitHandle(INFINITE);
		CloseHandle(_quitHandle);
#else
		sem_post(&_quitSem);
		waitHandle(3000);
		try{
			sem_destroy(&_quitSem);
		}catch(...){}
#endif
        MOLOG(Log::L_INFO, CLOGFMT(NTPServer, "~NTPServer() NTP Server uninitialized."));
	}

	void NTPServer::setBindAddr(const std::string strBindAddr, const short sBindPort)
	{
		_strBindAddr = strBindAddr;
		_sBindPort = sBindPort;
	}

	bool NTPServer::initial()
	{
#ifdef ZQ_OS_MSWIN
		_quitHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (!_quitHandle)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(NTPServer, "initial() Fail to create quit event handle"));
			return false;
		}
#else
		sem_init(&_quitSem, 0, 0);
#endif
		// create udp socket bind with special address
		InetAddress bindInetAddress;
		bindInetAddress.setAddress(_strBindAddr.c_str());
		_pServerSocket = new NTPUDPSocket(bindInetAddress, _sBindPort);
		if (NULL == _pServerSocket)
		{
			MOLOG(Log::L_ERROR, CLOGFMT(NTPServer, "Fail to create udp socket bind with %s:%d"), _strBindAddr.c_str(), _sBindPort);
			return false;
		}
		MOLOG(Log::L_INFO, CLOGFMT(NTPServer, "NTP server is listening for request at %s:%d"), _strBindAddr.c_str(), _sBindPort);
		return true;
	}

#ifdef ZQ_OS_MSWIN
	void NTPServer::logNTPPacket(NTP_Packet ntpPacket)
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
		ULONGLONG T1, T2, T3;
		getInt64FromNTPTime(nt_o, T1);
		getInt64FromNTPTime(nt_rx, T2);
		getInt64FromNTPTime(nt_tx, T3);
		FILETIME ft1, ft2, ft3;
		UINT64_to_FILTETIME(T1, ft1);
		UINT64_to_FILTETIME(T2, ft2);
		UINT64_to_FILTETIME(T3, ft3);

		FILETIME localft1, localft2, localft3;
		FileTimeToLocalFileTime(&ft1, &localft1);
		FileTimeToLocalFileTime(&ft2, &localft2);
		FileTimeToLocalFileTime(&ft3, &localft3);

		SYSTEMTIME st1, st2, st3;
		FileTimeToSystemTime(&localft1, &st1);
		FileTimeToSystemTime(&localft2, &st2);
		FileTimeToSystemTime(&localft3, &st3);
		MOLOG(Log::L_DEBUG, CLOGFMT(NTPServer, "Peer address[%s : %d],Request orginated [%d/%d/%d %d:%d:%d.%d],Request received [%d/%d/%d %d:%d:%d.%d], Reply originated [%d/%d/%d %d:%d:%d.%d]"),
			_recPeerAddress.getHostAddress(), _recPeerPort,
			st1.wMonth, st1.wDay, st1.wYear, st1.wHour, st1.wMinute, st1.wSecond, st1.wMilliseconds,
			st2.wMonth, st2.wDay, st2.wYear, st2.wHour, st2.wMinute, st2.wSecond, st2.wMilliseconds,
			st3.wMonth, st3.wDay, st3.wYear, st3.wHour, st3.wMinute, st3.wSecond, st3.wMilliseconds);
	}

#else

	void NTPServer::logNTPPacket(NTP_Packet ntpPacket)
	{
		// translate to local represent
		NTP_Time nt_o, nt_rx, nt_tx;
		nt_o.dwIntSec = ntpPacket.originate_timestamp_seconds;
		nt_o.dwFracSec=	ntpPacket.originate_timestamp_fractions;
		nt_rx.dwIntSec = ntpPacket.receive_timestamp_seconds;
		nt_rx.dwFracSec = ntpPacket.receive_timestamp_fractions;
		nt_tx.dwIntSec = ntpPacket.transmit_timestamp_seconds;
		nt_tx.dwFracSec = ntpPacket.transmit_timestamp_fractions;
		
		uint64 T1, T2, T3;
		getInt64FromNTPTime(nt_o, T1);
		getInt64FromNTPTime(nt_rx, T2);
		getInt64FromNTPTime(nt_tx, T3);
		
		struct timeval tvalOrig,tvalRecv,tvalTras;
        getTimevalFromInt64(T1,tvalOrig);
        getTimevalFromInt64(T2,tvalRecv);
        getTimevalFromInt64(T3,tvalTras);
		
	 	struct tm tm1,tm2,tm3;
        localtime_r(&tvalOrig.tv_sec,&tm1);
        localtime_r(&tvalRecv.tv_sec,&tm2);
        localtime_r(&tvalTras.tv_sec,&tm3);
	
		MOLOG(Log::L_DEBUG, CLOGFMT(NTPServer, "Peer address[%s : %d],Request orginated [%02d/%02d/%04d %02d:%02d:%02d.%03d],Request received [%02d/%02d/%04d %02d:%02d:%02d.%03d], Reply originated [%02d/%02d/%04d %02d:%02d:%02d.%03d]"),
			_recPeerAddress.getHostAddress(), _recPeerPort,
            tm1.tm_mon, tm1.tm_mday, tm1.tm_year, tm1.tm_hour, tm1.tm_min, tm1.tm_sec, tvalOrig.tv_usec/1000,
            tm2.tm_mon, tm2.tm_mday, tm2.tm_year, tm2.tm_hour, tm2.tm_min, tm2.tm_sec, tvalRecv.tv_usec/1000,
            tm3.tm_mon, tm3.tm_mday, tm3.tm_year, tm3.tm_hour, tm3.tm_min, tm3.tm_sec, tvalTras.tv_usec/1000);

}
#endif
	int NTPServer::run()
	{
		if (!initial())
		{
			return 1;
		}
		int nReceive = 0;
		int nSend = 0;
		int32 nControlWord = 0;
		while (!_bQuit)
		{
			// accept request
			memset(&_ntpRequest, 0, sizeof(_ntpRequest));
			nReceive = _pServerSocket->receiveFrom(&_ntpRequest, sizeof(_ntpRequest), _recPeerAddress, _recPeerPort);
			if (_bQuit)
			{
				break;
			}
			if (nReceive != sizeof(_ntpRequest))
			{
#ifdef ZQ_OS_MSWIN
				if (SOCKET_ERROR == nReceive)
				{
					DWORD dwErrorCode = WSAGetLastError();
					_errorNum++;
					MOLOG(Log::L_INFO, CLOGFMT(NTPServer, "run() server will sleep %d milliseconds as socket error[%d]"), _errorNum, dwErrorCode);
					DWORD dwResult = WaitForSingleObject(_quitHandle, _errorNum);
					if (_bQuit || dwResult == WAIT_OBJECT_0)
					{
						break;
					}
				}
#else
				if (nReceive < 0)
				{
					_errorNum++;
					MOLOG(Log::L_INFO, CLOGFMT(NTPServer, "run() server will sleep %d milliseconds as socket error[%d]"), 
						_errorNum, errno);
							
					struct timespec ts;
					struct timeval tmval;
					gettimeofday(&tmval,(struct timezone*)NULL);
					
					int64 nMicro = _errorNum*1000ll + tmval.tv_usec;
					ts.tv_sec = tmval.tv_sec + nMicro/1000000;
					ts.tv_nsec = (nMicro%1000000) * 1000;
					int nRet = sem_timedwait(&_quitSem,&ts);
					if(_bQuit || nRet == 0)
						break;

				}
				
#endif
				continue;
			}
			_errorNum = 0;
			getSystemTimeAsNTPTime(_ntpRecTime);

			// set packet 
			nControlWord = ntohl(_ntpRequest.Control_Word) & (0x3800FF00); // LI = 00, VN=VN, poll = poll
			_ntpReply.Control_Word = htonl(nControlWord | 0x04010090); //mode = 4 , stratum = 1,  precision = -16 = 0x90
			_ntpReply.root_delay = htonl(0x00000000); // root_delay = 0
			_ntpReply.root_dispersion = htonl(0x00000000); // root_diespersion = 0
			_ntpReply.reference_identifier = htonl(0x47505300); // GPS = 0x47505300
			_ntpReply.reference_timestamp_seconds = _ntpRefTime.dwIntSec;
			_ntpReply.reference_timestamp_fractions = _ntpRefTime.dwFracSec;
			_ntpReply.originate_timestamp_seconds = _ntpRequest.originate_timestamp_seconds;
			_ntpReply.originate_timestamp_fractions = _ntpRequest.originate_timestamp_fractions;
			_ntpReply.receive_timestamp_seconds = _ntpRecTime.dwIntSec;
			_ntpReply.receive_timestamp_fractions = _ntpRecTime.dwFracSec;
			getSystemTimeAsNTPTime(_ntpTransmitTime);
			_ntpReply.transmit_timestamp_seconds = _ntpTransmitTime.dwIntSec;
			_ntpReply.transmit_timestamp_fractions = _ntpTransmitTime.dwFracSec;
			
			// send response 
			_pServerSocket->setPeer(_recPeerAddress, _recPeerPort);
			nSend = _pServerSocket->send(&_ntpReply, sizeof(_ntpReply));
			if (nSend != sizeof(_ntpReply))
			{
				MOLOG(Log::L_ERROR, CLOGFMT(NTPServer, "Fail to send ntp reply"));
				continue;
			}
			logNTPPacket(_ntpReply);
		}
		MOLOG(Log::L_DEBUG, CLOGFMT(NTPServer, "NTP server stop receive request and exit"));
		return 0;
	}

	void NTPServer::stopNTPServer()
	{
		_bQuit = true;
		_pServerSocket->closeSocket();
		MOLOG(Log::L_DEBUG, CLOGFMT(NTPServer, "Listen socket is closed"));
	}
}

