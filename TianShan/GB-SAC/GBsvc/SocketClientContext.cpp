#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <assert.h>

#include "GBsvcConfig.h"
#include "SocketClientContext.h"
#include "IOCPThreadPool.h"
#include "Log.h"


extern ZQ::common::Config::Loader<ZQTianShan::GBServerNS::GBServerConfig > gConfig;

namespace ZQTianShan {	  
namespace GBServerNS {	 

ClientSocketContext::ClientSocketContext(SOCKET serverSocket, SOCKET clientSocket, sockaddr clientAddr, HANDLE completePort)
  :_serverSocket(serverSocket), _clientSocket(clientSocket), _clientAddr(clientAddr), _completePort(completePort)
{
	_bufUsed     = 0;
	_wsaBuf.buf  = _buf;
	_wsaBuf.len  = sizeof(_buf);
	_comPortType = TOTAL;	
    memset(_buf, 0, sizeof(_buf));
	memset(&_overlapped, 0, sizeof(_overlapped));
}

CreateSvcSocket::CreateSvcSocket(ZQ::common::Log& log)
:_connectPool(gConfig._gbSvcBase._dispatchThreadSize),
_processPool(gConfig._gbSvcBase._processThreadSize),
_releasePool(gConfig._gbSvcBase._releaseThreadSize),
_log(log)
{
	_hIOCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == _hIOCompletionPort)
	{
	    _log(ZQ::common::Log::L_ERROR, CLOGFMT(CreateSvcSocket,"CreateSvcSocket create iocp failed, errorCode[%u]"), WSAGetLastError());	
		throw;
	}

	for(int threadNum = 0; threadNum < _connectPool.size(); ++threadNum)
	{
		ConnectDispatch *handleDisp = new ConnectDispatch(_connectPool, _processPool, _releasePool, _hIOCompletionPort, _log);
		handleDisp->start();
	}
	    
}

bool CreateSvcSocket::createSocket(int family, int type , int protocol )
{
	_mAcceptSocket	= WSASocket(family, type, protocol, NULL, 0, WSA_FLAG_OVERLAPPED);
	if( _mAcceptSocket == INVALID_SOCKET )
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CreateSvcSocket,"createSocket failed with family[%d] type[%d] protocol[%d] and errorCode[%u]"),
			family , type , protocol , WSAGetLastError() );		
		return false;
	}

	return true;
}


bool CreateSvcSocket::bind( const std::string& localIp , const std::string& localPort)
{
	assert( _mAcceptSocket != INVALID_SOCKET );
	addrinfo* adInfo = NULL;
	addrinfo  addrHint = {0};
	int family = (localIp.find(":") != std::string::npos) ? AF_INET6 :AF_INET;

	addrHint.ai_family		=	family;
	addrHint.ai_socktype	=	SOCK_STREAM;
	addrHint.ai_protocol	=	IPPROTO_TCP;
	addrHint.ai_flags		=	AI_CANONNAME | AI_PASSIVE;
	int rc = ::getaddrinfo( localIp.c_str() , localPort.c_str() , &addrHint , &adInfo);
	if (0 != rc || NULL == adInfo) 
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CreateSvcSocket,"bind can't get address information with peer[%s:%s] and errorCode[%u]"),
			localIp.c_str() , localPort.c_str()	,WSAGetLastError() );

		return false;
	}

	int iRet = ::bind(_mAcceptSocket, adInfo->ai_addr ,(int)adInfo->ai_addrlen );
	::freeaddrinfo(adInfo);

	if( iRet ==  SOCKET_ERROR )
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CreateSvcSocket , "failed to bind socket with peer[%s:%s] and errorCode[%u]" ),
			localIp.c_str() , localPort.c_str() , WSAGetLastError()	);
		return false;
	}

	return true;
}

bool CreateSvcSocket::listen( int backLog/*  = 100*/)
{
	assert( _mAcceptSocket != INVALID_SOCKET );
	int iRet = ::listen( _mAcceptSocket , backLog );
	if( iRet == SOCKET_ERROR )
	{
		_log(ZQ::common::Log::L_ERROR, CLOGFMT(CreateSvcSocket,"failed to invoke listen and errorCode[%u]"), WSAGetLastError());
		return false;
	}

	return true;
}


ClientSocketContext * CreateSvcSocket::accept()
{
	assert( _mAcceptSocket != INVALID_SOCKET );

	ClientSocketContext * pSocket = NULL;
	sockaddr clientAddr = {0};
	int addLen = sizeof(clientAddr);

	SOCKET clientSocket = ::accept(_mAcceptSocket, (sockaddr*)&clientAddr , &addLen );
	if(clientSocket == INVALID_SOCKET )
	{
		if( WSAGetLastError() !=  10004 )
			_log(ZQ::common::Log::L_ERROR, CLOGFMT(CreateSvcSocket,"accept failed and errorCode[%u]"), WSAGetLastError());
		else//listen socket is closed
			_log(ZQ::common::Log::L_INFO, CLOGFMT(CreateSvcSocket,"listen socket is closed. errorCode[%u]"), WSAGetLastError());
	}
	else
	{
		pSocket = new ClientSocketContext(_mAcceptSocket, clientSocket, clientAddr, _hIOCompletionPort);
		assert( pSocket != NULL );
		(new AssociateWithIOCP(_releasePool, pSocket, _log))->start();
	}

	return pSocket;
}

CreateSvcSocket::~CreateSvcSocket()
{
	for(int postTime = 0; postTime < _connectPool.size(); ++postTime)
		::PostQueuedCompletionStatus(_hIOCompletionPort, 0, (DWORD)IOCP_EXIT_CODE, NULL);

	::Sleep(1000);
	::CloseHandle(_hIOCompletionPort);
	::shutdown(_mAcceptSocket, SD_BOTH);
	::closesocket(_mAcceptSocket);
	::WSACleanup();

	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CreateSvcSocket,"CreateSvcSocket destructor"));
}

}//GBServerNS
}//	ZQTianShan