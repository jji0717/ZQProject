#ifndef __NTPUDPSOCKET_H__
#define __NTPUDPSOCKET_H__

#include "ZQ_common_conf.h"
#include "UDPSocket.h"

namespace NTPSync
{
	class NTPUDPSocket : public ZQ::common::UDPSocket
	{
	public:
		/// Create an unbound UDP socket, mostly for internal use.
		NTPUDPSocket(void);

		/// Create a UDP socket and bind it to a specific interface and port
		/// address so that other UDP sockets may find and send UDP messages
		/// to it. On failure to bind, an exception is thrown.
		/// @param bind address to bind this socket to.
		/// @param port number to bind this socket to.
		NTPUDPSocket(const ZQ::common::InetAddress &bind, ZQ::common::tpport_t port);
	public:
		/// Receive a message from any host.
		/// @param pointer to packet buffer to receive.
		/// @param len of packet buffer to receive.
		/// @return number of bytes received.
		int receiveTimeout(void *buf, size_t len, uint32 dwTimeout);

		void closeSocket();
	};
}

#endif

