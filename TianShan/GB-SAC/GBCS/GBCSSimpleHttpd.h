#ifndef __SimpleHttpd_H__
#define __SimpleHttpd_H__

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
#  define _closesocket(n) ::close(n)
# endif
#endif

#define MSG_INTERNAL_ERR (-1)
#define MSG_OK           (0)
#define MSG_UNKNOWN_PAID (1)


// -----------------------------
// class SimpleHttpd
// -----------------------------
class SimpleHttpd : public ZQ::common::NativeThread
{
public:

	SimpleHttpd(const char* hostIP, const int& port, ZQ::common::Log* pLog=0);
	virtual ~SimpleHttpd(void);

	bool init(void);
	void close(void);
	
	//set http socket send and receive time out, unit second, less or equal zero is block
	void setTimeout(int timeOut);
	
	typedef struct _MsgInfo
	{
		std::string uri;
		std::string ip;
		int  port;
		std::string msgBody;

		_MsgInfo():port(0){};
		
	}MsgInfo;

	virtual int OnHttpMessage(const MsgInfo& info, std::string& respHttpMessageBody); 
	bool bExit(){ return _bExit;}

protected:	
	virtual int run(void);	

private:
	ZQ::common::NativeThreadPool _pool;

	std::string _strIP;
	int			_port;
	SOCKET_T	_sock;
	ZQ::common::Log* _pLog;
	bool		_bExit;
	int			_nTimeOut;//it is receive and send timeout unit is second
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

#endif //__SimpleHttpd_H__
