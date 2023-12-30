#ifndef __SIMPLESERVER_H__
#define __SIMPLESERVER_H__

#pragma once
#include "ZQ_common_conf.h"
#include "NativeThread.h"
#include "NativeThreadPool.h"
#include "Log.h"
#include <string>
#include <time.h>

#ifdef ZQ_OS_MSWIN
#  include <io.h>
#  include <fcntl.h>
# ifdef WITH_IPV6
#  include <winsock2.h> // Visual Studio 2005 users: you must install the Platform SDK (R2)
#  include <ws2tcpip.h>
#  include <wspiapi.h>
# else
//#  include <winsock.h> // Visual Studio 2005 users: you must install the Platform SDK (R2)
# include <winsock2.h>   //Alternative: use winsock2 (not available with eVC)
# endif
#else
#	include <sys/socket.h>
#   include <netinet/in.h>
#   include <unistd.h>
#   include <fcntl.h>
#   include <netinet/tcp.h>          // TCP_NODELAY
#	include <arpa/inet.h>
#endif

#ifndef SOCKET_T
# ifdef ZQ_OS_MSWIN
#  define SOCKET_T SOCKET
#  define _closesocket(n) closesocket(n)
# else
#  define SOCKET_T int
#  define _closesocket(n) close(n)
# endif
#endif

#define MSG_INTERNAL_ERR (-1)
#define MSG_OK           (0)
#define MSG_UNKNOWN_PAID (1)

//message type

//#define TYPE_TRANSFER = "TransferStatus";
//#define TYPE_CHECKSUM = "ContentChecksum";


namespace {
	const char* TYPE_TRANSFER = "TransferStatus";
	const char* TYPE_CHECKSUM = "ContentChecksum";
}

class SimpleServer : public ZQ::common::NativeThread
{
public:

	SimpleServer(const char* hostIP, const int& port, ZQ::common::Log* pLog=0);
	virtual ~SimpleServer(void);

	bool init(void);
	void close(void);
	
	inline void setTimeout(int timeOut) {
		_nTimeOut = timeOut;
	}

	virtual int handleMsg(const std::string& type, const std::string& msg); 
	bool bExit(){ return _bExit == true;}

protected:	
	virtual int run(void);	

private:
	ZQ::common::NativeThreadPool _pool;

	std::string _strIP;
	int			_port;
	SOCKET_T	_sock;
	ZQ::common::Log* _pLog;
	bool		_bExit;
	int			_nTimeOut;//it is receive and send timeout unit is second,0 is no timeout
};


#ifdef ZQ_OS_MSWIN
class SimpleSerWSA
{
public:
	SimpleSerWSA();
	~SimpleSerWSA();
};
static SimpleSerWSA simpleSerwsa;
#endif

#endif //__SIMPLESERVER_H__
