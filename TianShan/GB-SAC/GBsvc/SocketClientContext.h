#if !defined(__ZQTIANSHAN_GB_SOCKET_CLIENT_CONTEXT_H__)
#define __ZQTIANSHAN_GB_SOCKET_CLIENT_CONTEXT_H__

#include "NativeThreadPool.h"
#include "NativeThread.h"

namespace ZQTianShan {	  
namespace GBServerNS { 

const int MAX_RECV_BUFF  = 1500;
const int IOCP_EXIT_CODE = NULL;

enum OPE_COMPORT
{
	RECV,
	WREATE,
	TOTAL
};

struct ClientSocketContext
{
	ClientSocketContext(SOCKET serverSocket, SOCKET clientSocket, sockaddr clientAddr, HANDLE completePort);

	HANDLE              _completePort;
	OVERLAPPED			_overlapped;
	WSABUF              _wsaBuf; 
	char		        _buf[MAX_RECV_BUFF];
	int                 _bufUsed;

	sockaddr            _clientAddr;
	SOCKET				_clientSocket;
	SOCKET				_serverSocket;
	int                 _comPortType;
};

class CreateSvcSocket
{
public:
	explicit CreateSvcSocket(ZQ::common::Log& log);
	~CreateSvcSocket();

	bool createSocket( int family, int type , int protocol);
	bool bind( const std::string& localIp , const std::string& localPort);
	bool listen( int backLog  = 100);
	ClientSocketContext * accept();

private:
	ZQ::common::Log &  _log;
	SOCKET             _mAcceptSocket;
	HANDLE             _hIOCompletionPort;

	ZQ::common::NativeThreadPool  _connectPool;	 //DEFAULT_THRPOOL_SZ = 10
	ZQ::common::NativeThreadPool  _processPool;
	ZQ::common::NativeThreadPool  _releasePool;
};


}//GBServerNS
}//	ZQTianShan

#endif// __ZQTIANSHAN_GB_SOCKET_CLIENT_CONTEXT_H__