#include "NTPUDPSocket.h"

using namespace ZQ::common;

namespace NTPSync
{
	NTPUDPSocket::NTPUDPSocket() : UDPSocket()
	{
	}

	NTPUDPSocket::NTPUDPSocket(const InetAddress &bind, tpport_t port) : UDPSocket(bind, port)
	{
	}

	int NTPUDPSocket::receiveTimeout(void *buf, size_t len, uint32 dwTimeout)
	{
		fd_set setCheck;
		timeval timeout;
		timeout.tv_sec = dwTimeout/1000; 
		timeout.tv_usec = dwTimeout - dwTimeout/1000*1000;
		FD_ZERO(&setCheck);
		FD_SET( _so, &setCheck );
		select(_so+1, &setCheck, 0, 0, &timeout);
		return ::recv(_so, (char *)buf, len, 0);
	}

	void NTPUDPSocket::closeSocket()
	{
		endSocket();
	}
}

