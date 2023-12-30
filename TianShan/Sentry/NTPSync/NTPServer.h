// File Name: NTPServer.h
// Date: 2009-01
// Description: Definition of ntp server class. 
// This class privide time synchronize service and it is support ntp version3 but not support NTP authentication mechanism

#ifndef __NTPSERVER_H__
#define __NTPSERVER_H__

#include "Log.h"
#include "NativeThread.h"
#include "NTPUDPSocket.h"
#include "NTPUtils.h"

namespace NTPSync
{
	class NTPServer : public ZQ::common::NativeThread
	{
	public:
		///constructor
		///@param Pool : thread pool size
		///@param strBindAddr : local address bind with this service, default is 0.0.0.0 which mean any valid address
		///@param sBindPort : local port bind with this service, default is 1230
		NTPServer(ZQ::common::Log* log, const std::string strBindAddr = "0.0.0.0", const short sBindPort = 123, int stacksize = 0);
		~NTPServer(void);
	public:
		///set bind address and port with this service, this method shoud be call before initial if call it
		///@param strBindAddr : local address bind with this service, default is 0.0.0.0 which mean any valid address
		///@param sBindPort : local port bind with this service, default is 1230
		void setBindAddr(const std::string strBindAddr = "0.0.0.0", const short sBindPort = 123);

		void stopNTPServer();
	protected:
		///accept ntp request and send ntp reply
		///@return the return value will also be passed as the thread exit code
		virtual int run();
	private:
		///create log file and udp socket binded with special address
		///@return return true if success, else return false
		bool initial();

		///log ntp reply packet
		///@param ntpPacket
		void logNTPPacket(NTP_Packet ntpPacket);
	private:
		NTPServer(const NTPServer& oriNTPServer);
		NTPServer& operator=(const NTPServer& oriNTPServer);
	private:
		NTPSync::NTPUDPSocket *_pServerSocket; // udp socket object
		ZQ::common::Log* _log; // log pointer
		ZQ::common::InetHostAddress _recPeerAddress; // peer address
		int _recPeerPort; // peer port
	private:
		NTP_Packet _ntpRequest; // ntp request packet
		NTP_Packet _ntpReply; // ntp reply packet
		NTP_Time _ntpRecTime; // receive timestamp
		NTP_Time _ntpTransmitTime; // transmit timestamp
		NTP_Time _ntpRefTime; // reference timestamp
	private:
		std::string _strBindAddr; // local address to bind
		short _sBindPort; // local port to bind
		bool _bQuit;
		uint32 _errorNum;
#ifdef ZQ_OS_MSWIN
		HANDLE _quitHandle;
#else
		sem_t _quitSem;
#endif

	};
}

#endif

