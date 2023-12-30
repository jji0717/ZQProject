#ifdef ZQ_OS_MSWIN
#include <WinSock2.h>
#else
#include <sys/socket.h>  
#include <sys/epoll.h>  
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#include <errno.h>
#endif

#include <algorithm>

#include "ICommunicator.hpp"
#include "IMgr.hpp"
#include "IAsyncClientHandler.hpp"
#include "IAsyncServer.hpp"

#ifdef ZQ_OS_MSWIN

AcceptCommuncator::AcceptCommuncator(IAsyncServer* asyncServer, IMgr* mgr, IAsyncClientFactory* asyncClientFactory, int poster)
    :_asyncServer(asyncServer), _mgr(mgr), _maxWorker(poster), _asyncClientFactory(asyncClientFactory), _log(*mgr->_log)
{
#if defined(DEBUG)
	_clientCount = 0;
#endif//DEBUG
	_quit         = false;
	_hAcceptEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL); 
	_listenSock   = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == _listenSock)
	{
		throw;
	}
	//the GUID for exporting AcceptEx and GetAcceptExSockaddrs pfn
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS; 

	// AcceptEx pfn
	DWORD dwBytes = 0;  
	if(SOCKET_ERROR == WSAIoctl(_listenSock, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx), 
		&_lpfnAcceptEx, sizeof(_lpfnAcceptEx), &dwBytes, NULL,NULL))  
	{  
		int lastError = WSAGetLastError();
		throw;
	}  

	// GetAcceptExSockAddrs pfn
	dwBytes = 0;
	if( SOCKET_ERROR == WSAIoctl(_listenSock, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidGetAcceptExSockAddrs, sizeof(GuidGetAcceptExSockAddrs),
		&_lpfnGetAcceptExSockAddrs, sizeof(_lpfnGetAcceptExSockAddrs), &dwBytes, NULL, NULL))  
	{  
		int lastError = WSAGetLastError();
	} 
}

int AcceptCommuncator::preAccept(const char* addr, unsigned int port)
{
	//1. bind socket to servent (IOCP)
	struct sockaddr_in ServerAddress;

	// generate Socket message
	Message* plistenMessage = new Message;
	plistenMessage->_sock = _listenSock;
	plistenMessage->_listenSock = _listenSock;
	plistenMessage->_msgType    = ERROR_PENDING;
	_asyncServer->addServent(_listenSock, this);
	_msgListen.push_back(plistenMessage);

	//2. fill address
	ZeroMemory((char *)&ServerAddress, sizeof(ServerAddress));
	ServerAddress.sin_family = AF_INET;

	//int port = 8080;
	//ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);                 
	ServerAddress.sin_addr.s_addr = inet_addr(addr);
	//ServerAddress.sin_addr.s_addr = inet_addr("192.168.81.108");         
	ServerAddress.sin_port = htons(port); 
	if (SOCKET_ERROR == bind(_listenSock, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress))) 
	{
		int lastErr = WSAGetLastError();
		return false;
	}
	if (SOCKET_ERROR == listen(_listenSock, SOMAXCONN))
	{
		return false;
	}

	// reg FD_ACCEPT, AcceptEx I/O not enough, more post
	WSAEventSelect(_listenSock, _hAcceptEvent, FD_ACCEPT);  

	//3. Prepare data for AcceptEx I/O request
	for(int worker = 0; worker < _maxWorker; ++worker)
	{
		Message* nextAcceptMsg = new (std::nothrow) Message;
		nextAcceptMsg->_sock   = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		_msgListen.push_back(nextAcceptMsg);
		this->postAccept(plistenMessage->_listenSock, nextAcceptMsg);
	}

	return true;
}

int  AcceptCommuncator::postAccept(int listenSock, Message* nextAcceptMsg)
{
	nextAcceptMsg->_listenSock = listenSock;
	nextAcceptMsg->_msgType = ACCEPT_PENDING;

	//1. Prepare socket is difference from traditional accept
	nextAcceptMsg->_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);  
	if(INVALID_SOCKET == nextAcceptMsg->_sock)  
	{  
		int lastErr = WSAGetLastError(); 
		return false;  
	} 
	int reuse = 1;
	::setsockopt(nextAcceptMsg->_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(int));
	
	int nNetTimeout=1000;//1s，
	::setsockopt(nextAcceptMsg->_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout, sizeof(int));

	SOCKET listenSocket = nextAcceptMsg->_listenSock;
	SOCKET acceptSocket = nextAcceptMsg->_sock;
	char*  outputBuffer = nextAcceptMsg->_buf;
	int    receiveDataLength     = nextAcceptMsg->_bufSize - ( (sizeof(SOCKADDR_IN) + 16 ) * 2 );
	int    localAddressLength    = sizeof(SOCKADDR_IN) + 16;
	int    remoteAddressLength   = sizeof(SOCKADDR_IN) + 16;
	unsigned long  bytesReceived = 0;

	nextAcceptMsg->_msgType = ACCEPT_PENDING;  
	memset(&nextAcceptMsg->_overlapped, 0, sizeof(OVERLAPPED));
	unsigned long u1 = 1;
	ioctlsocket(acceptSocket, FIONBIO, &u1);

	//2. post AcceptEx
	if(FALSE == _lpfnAcceptEx(listenSocket, acceptSocket, outputBuffer, receiveDataLength, 
		localAddressLength, remoteAddressLength, (LPDWORD)&bytesReceived, &nextAcceptMsg->_overlapped) )  
	{  
		if(WSA_IO_PENDING != WSAGetLastError())  
		{  
			int lastErr = WSAGetLastError();
			return false;  
		}  
	} 

	nextAcceptMsg->_ioSize = receiveDataLength;
	return true;
}

int  AcceptCommuncator::onAccept(Message* acceptMsg)
{
#ifdef DEBUG
	InterlockedIncrement(&_clientCount);
	if (!(_clientCount % 300))
	{
		std::cout<<"apt="<<_clientCount<<endl;
	}
#endif//DEBUG

	SOCKET       clientSock     = acceptMsg->_sock;
	SOCKET       listenSock     = acceptMsg->_listenSock;
	SOCKADDR_IN* remoteSockaddr = NULL;
	SOCKADDR_IN* localSockaddr  = NULL;  
	int remoteSockaddrLength = sizeof(SOCKADDR_IN);
	int localSockaddrLength  = sizeof(SOCKADDR_IN);  
	int receiveDataLength    = acceptMsg->_bufSize - (2 * (sizeof(SOCKADDR_IN) + 16));
	int localAddressLength   = sizeof(SOCKADDR_IN) + 16;
	int remoteAddressLength  = sizeof(SOCKADDR_IN) + 16;

	if (!clientSock)
	{
		acceptMsg->_ioSize = 0;
		postAccept(listenSock, acceptMsg);//prepare for next accept
		return true;
	}


	// 1. Get client info
	acceptMsg->_sock = NULL;
	::setsockopt(clientSock, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,(char*)&_listenSock, sizeof(_listenSock));
	this->_lpfnGetAcceptExSockAddrs(acceptMsg->_buf, receiveDataLength,  
		localAddressLength, remoteAddressLength, (LPSOCKADDR*)&localSockaddr, 
		&localSockaddrLength, (LPSOCKADDR*)&remoteSockaddr, &remoteSockaddrLength);  

	struct linger linger;
	memset((void*)&linger, 0, sizeof(linger));
	linger.l_onoff = 1; // (在closesocket()调用,但是还有数据没发送完毕的时候容许逗留)
	linger.l_linger = 0;// (容许逗留的时间为0秒)
	
	//2. insert into accept list
	{
		IAsyncClientHandler* servant = _asyncClientFactory->create();
		Message* newNextAcceptMsg  = new (std::nothrow) Message;
		strncpy(newNextAcceptMsg->_buf, acceptMsg->_buf, acceptMsg->_ioSize);
		memset(acceptMsg->_buf, 1, acceptMsg->_bufSize);
		newNextAcceptMsg->_ioSize = acceptMsg->_ioSize;
		newNextAcceptMsg->_sock   = clientSock;
		newNextAcceptMsg->_listenSock     = _listenSock;
		newNextAcceptMsg->_clientHandler  = servant;
		
		CommonTcpCommuncator::Ptr conn = new (std::nothrow) CommonTcpCommuncator(_asyncServer, _mgr, newNextAcceptMsg);
		setsockopt(newNextAcceptMsg->_sock, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(struct linger));
		servant->attachConn(conn);
		_mgr->add(conn, IAsyncClientHandler::Ptr(newNextAcceptMsg->_clientHandler));
		if (!conn->init())
		{
			conn->unInit();
		}
	}

	//3. re-post accept socket
	acceptMsg->_ioSize = 0;
	postAccept(listenSock, acceptMsg);//prepare for next accept

	return true;
}

int AcceptCommuncator::run(void)
{
	struct linger linger;
	memset((void*)&linger, 0, sizeof(linger));
	linger.l_onoff = 1; // (在closesocket()调用,但是还有数据没发送完毕的时候容许逗留)
	linger.l_linger = 0;// (容许逗留的时间为0秒)

	while (!_quit)
	{
		//1. if prepared accept enough
		if (WAIT_OBJECT_0 == WaitForSingleObject(_hAcceptEvent, 500))
		{
			//1.1 accept aqsp
			sockaddr_storage addr;
			int addLen = sizeof(addr);
			SOCKET sockAccept = ::accept(_listenSock, (sockaddr*)&addr , &addLen );
			if (sockAccept > 0)
			{
				IAsyncClientHandler* servant  = _asyncClientFactory->create();
				Message* activeMsg            = new (std::nothrow) Message;

				activeMsg->_listenSock     = _listenSock;
				activeMsg->_sock           = sockAccept;				
				activeMsg->_clientHandler  = servant;
				CommonTcpCommuncator::Ptr connPtr(new (std::nothrow) CommonTcpCommuncator(_asyncServer, _mgr, activeMsg)) ;
				activeMsg->_selfConn = connPtr;
				servant->attachConn(connPtr);
				setsockopt(activeMsg->_sock, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(struct linger));
				_mgr->add(connPtr, IAsyncClientHandler::Ptr(servant));
				if (!connPtr->init())
					connPtr->unInit();
			}
			//1.2 adding post accept
			if(_msgListen.size() < 500)
			{
				Message* nextAcceptMsg = new (std::nothrow) Message;
				nextAcceptMsg->_sock   = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				_msgListen.push_back(nextAcceptMsg);
				postAccept(_listenSock, nextAcceptMsg);
			}
		}	

		if (_quit)
			break;

		//2. check if there have connect time too long 
		int sockCount = 0;
		for (std::list<Message*>::iterator it = _msgListen.begin(); it != _msgListen.end(); ++it)
		{
			if(0 == (*it)->_sock || INVALID_SOCKET == (*it)->_sock)
			{
				++sockCount;
				continue;
			}

			int seconds = 0;
			int opt     = sizeof(seconds);
			int sock    = (*it)->_sock;
			int error   = ::getsockopt(sock, SOL_SOCKET, SO_CONNECT_TIME, (char*)&seconds, (int*)&opt);
			if (NO_ERROR != error )
			{
				int err = WSAGetLastError();
				if (WSAENOTSOCK == err)
				{
					::shutdown( sock, SD_BOTH);// trigger IOCP close event
					::closesocket(sock);
					(*it)->_sock = 0;
				}
				continue;
			}

			if (5 < seconds)
			{
				::setsockopt(sock, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(struct linger));
				::shutdown(sock, SD_BOTH);// trigger IOCP close event
				::closesocket(sock);
				(*it)->_sock = 0;
			}
		}
	}

	return true;
}
#else//ZQ_OS_LINUX

AcceptCommuncator::AcceptCommuncator(IAsyncServer* asyncServer, IMgr* mgr, IAsyncClientFactory* asyncClientFactory, int poster)
    :_asyncServer(asyncServer), _mgr(mgr), _maxWorker(poster), _asyncClientFactory(asyncClientFactory),_log(*mgr->_log)
{
	_quit        = false;
#if defined(DEBUG)
	_clientCount = 0; 
#endif
	_listenSock  = socket(AF_INET, SOCK_STREAM, 0);
	if (0 == _listenSock)
	{
		throw;
	}

	::fcntl(_listenSock, F_SETFL, O_NONBLOCK);
}

int AcceptCommuncator::preAccept(const char* addr, unsigned int port)
{
#define  SOMAXCONN  (500)
	//1. bind socket to servent
	struct sockaddr_in ServerAddress;

	// generate Socket message
	Message* plistenMessage = new Message;
	plistenMessage->_sock   = _listenSock;
	plistenMessage->_listenSock = _listenSock;
	plistenMessage->_msgType    = ERROR_PENDING;
	_msgListen.push_back(plistenMessage);

	//2. fill address
	memset((char *)&ServerAddress, 0, sizeof(ServerAddress));
	ServerAddress.sin_family = AF_INET;

	//int port = 8080;
	//ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);                      
	ServerAddress.sin_addr.s_addr   = inet_addr(addr);         
	//ServerAddress.sin_addr.s_addr = inet_addr("10.15.10.96");
	ServerAddress.sin_port = htons(port); 
	if (0 != bind(_listenSock, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress))) 
	{//log
		return false;
	}
	if (0 != listen(_listenSock, SOMAXCONN))
	{
		return false;
	}

	int reuse = 1;
	setsockopt(plistenMessage->_listenSock, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(int));
	plistenMessage->_listenSock = _listenSock;
	plistenMessage->_msgType    = ACCEPT_PENDING;

	//post Accept
	_asyncServer->addServent(_listenSock, (ICommuncator*) plistenMessage, EPOLLIN);
}

int  AcceptCommuncator::postAccept(int listenSock, Message* nextAcceptMsg)
{
	nextAcceptMsg->_listenSock = listenSock;
	nextAcceptMsg->_msgType    = ACCEPT_PENDING;

	//post Accept
	_asyncServer->activeServent(_listenSock, (ICommuncator*) nextAcceptMsg, EPOLLIN);

	return true;
}


int  AcceptCommuncator::onAccept(Message* acceptMsg)
{
#ifdef DEBUG
	if (!(++_clientCount % 300))
	{
		std::cout<<"apt="<<_clientCount<<endl;
	}
#endif//DEBUG

	int  sock       = 0;
	int  listenSock = acceptMsg->_listenSock;
	struct sockaddr_in  sin;  
	socklen_t           len        = sizeof(struct sockaddr_in); 

	if(-1 != (sock = accept(listenSock, (struct sockaddr*)&sin, &len)) )
	{  
		Message* newNextAcceptMsg    = new (std::nothrow) Message;
		IAsyncClientHandler* servant = _asyncClientFactory->create();
		struct linger linger;

		::fcntl(sock, F_SETFL, O_NONBLOCK);
		memset((void*)&linger, 0, sizeof(linger));
		linger.l_onoff  = 1;
		linger.l_linger = 30;
		setsockopt(sock, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(struct linger));

		newNextAcceptMsg->_sock          = sock;
		newNextAcceptMsg->_clientHandler = servant;
		CommonTcpCommuncator::Ptr conn   = new (std::nothrow) CommonTcpCommuncator(_asyncServer, _mgr, newNextAcceptMsg);

		servant->attachConn(conn);
		_mgr->add(conn, IAsyncClientHandler::Ptr(servant));
		if (!conn->init())
			conn->unInit();
	}
	else if(errno != EAGAIN && errno != EINTR) 
	{ 
		//log 
	} 

	//3. re-post accept socket
	postAccept(listenSock, acceptMsg);//prepare for next accept

	return true;
}


int AcceptCommuncator::run(void)
{
	int    status;
	struct timeval tv;
	struct timeval *tvp = &tv;
	fd_set grp;
	struct linger linger;

	memset((void*)&linger, 0, sizeof(linger));
	linger.l_onoff  = 1; // (在closesocket()调用,但是还有数据没发送完毕的时候容许逗留)
	linger.l_linger = 0;// (容许逗留的时间为0秒)

	while (!_quit)
	{
		int                 sock =   0;
		struct sockaddr_in  sin;  
		socklen_t           len = sizeof(struct sockaddr_in); 

		tv.tv_sec  = 2;
		tv.tv_usec = 0;
		fd_set grpRecv;
		FD_ZERO(&grpRecv);
		FD_SET(_listenSock, &grpRecv);
		status = select((int)_listenSock + 1, &grpRecv, NULL, NULL, tvp);
		if(status < 1 || !FD_ISSET(_listenSock, &grpRecv))
		{
			//log
			continue;
		}

		if(-1 != (sock = accept(_listenSock, (struct sockaddr*)&sin, &len)) )
		{  
			Message*   newNextAcceptMsg   = new (std::nothrow) Message;
			IAsyncClientHandler*  servant = _asyncClientFactory->create();
			struct linger linger;

			::fcntl(sock, F_SETFL, O_NONBLOCK);
			memset((void*)&linger, 0, sizeof(linger));
			linger.l_onoff = 1;
			linger.l_linger = 30;
			setsockopt(sock, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(struct linger));

			newNextAcceptMsg->_sock      = sock;
			newNextAcceptMsg->_clientHandler = servant;
			CommonTcpCommuncator::Ptr conn( new (std::nothrow) CommonTcpCommuncator(_asyncServer, _mgr, newNextAcceptMsg) );
			
			servant->attachConn(conn);
			_mgr->add(conn, IAsyncClientHandler::Ptr(servant));
			if (!conn->init())
			{
				conn->unInit();
			}
#ifdef DEBUG
			if (!(++_clientCount % 300))
			{
				std::cout<<"apt="<<_clientCount<<endl;
			}
#endif//DEBUG
		}
		else if(errno != EAGAIN && errno != EINTR) 
		{ 
			//log 
		} 
	}

	return true;
}

#endif//ZQ_OS_LINUX | ZQ_OS_MSWIN


int  AcceptCommuncator::onError(Message* errMsg)
{
	_mgr->onError(errMsg, this);
#ifdef ZQ_OS_MSWIN
	::shutdown(errMsg->_sock, SD_BOTH);
	::closesocket(errMsg->_sock);
#else// ZQ_OS_LINUX
	::shutdown(errMsg->_sock, SHUT_RDWR);
	::close(errMsg->_sock);
#endif

	// log alert
	postAccept(_listenSock, errMsg);//prepare for next accept
	return false;
};

int  AcceptCommuncator::onClose(Message* closeMsg)
{
	_mgr->remove(closeMsg->_selfConn);

	return true;
}


int  AcceptCommuncator::init(const char* addr, unsigned int port)
{
	this->preAccept(addr, port);
	ZQ::common::NativeThread::start();
	return true;
}

int  AcceptCommuncator::unInit()
{
	_quit = true;

	return true;
}

AcceptCommuncator::~AcceptCommuncator()
{
	_quit = true;
#ifdef ZQ_OS_MSWIN
	::shutdown(_listenSock, SD_BOTH);
	::closesocket(_listenSock);
#else//  ZQ_OS_LINUX
	::shutdown(_listenSock, SHUT_RDWR);
	::close(_listenSock);
#endif

	ZQ::common::NativeThread::waitHandle(3000);
	for (std::list<Message*>::iterator it = _msgListen.begin(); it != _msgListen.end(); ++it)
	{
		delete (*it);
	}

	_msgListen.clear();
	delete _asyncClientFactory;
	_asyncClientFactory = NULL;
};

CommonTcpCommuncator::CommonTcpCommuncator(IAsyncServer* asyncServer, IMgr* mgr, Message* msgHold)
:_asyncServer(asyncServer), _mgr(mgr), _log(*(_mgr->_log) ), _isManualShutdown(0)
{
	Message* sendTrigrer  = new Message(SEND_PENDING);
	Message* recvTrigger  = new Message(RECV_PENDING);
	
	_msgChain._defSock   = msgHold->_sock;
	_msgChain._defHandle = msgHold->_clientHandler;
	
	sendTrigrer->_sock    = msgHold->_sock;
	sendTrigrer->_clientHandler = _msgChain._defHandle;

	recvTrigger->_sock    = msgHold->_sock;
	recvTrigger->_clientHandler = _msgChain._defHandle;

	_msgChain._defASendTrigger  = sendTrigrer;
	_msgChain._defARecvTrigger  = recvTrigger;
	_msgChain._msgList.push_back(msgHold);
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CommonTcpCommuncator, "Constructor leave sock[%d]"), _msgChain._defSock);
}

int  CommonTcpCommuncator::init(const char* addr, unsigned int port)
{
	using namespace ZQ::common;
	Message* msgHold = *_msgChain._msgList.begin();

	_log(Log::L_DEBUG, CLOGFMT(CommonTcpCommuncator, "init() entry sock[%d]"), _msgChain._defSock);
	_msgChain._defASendTrigger->_clientHandler  = msgHold->_clientHandler;
	_msgChain._defARecvTrigger->_clientHandler  = msgHold->_clientHandler;
#ifdef ZQ_OS_LINUX
	msgHold->_msgType = RECV_PENDING;
	if(!_asyncServer->addServent(msgHold->_sock, (ICommuncator*)msgHold, EPOLLIN))
	{
		_log(Log::L_INFO, CLOGFMT(CommonTcpCommuncator, "init() error sock[%d], addTo asyncServer[EPOLLIN]"), _msgChain._defSock);
		return false;
	}
#else// ZQ_OS_MSWIN
	if(!_asyncServer->addServent(msgHold->_sock, this, 1))
	{
		_log(Log::L_WARNING, CLOGFMT(CommonTcpCommuncator, "init() error sock[%d], addTo asyncServer[IOCP Reg]"), _msgChain._defSock);
		return false;
	}

	msgHold->_msgType = RECV_PENDING;
	if (msgHold->_ioSize > 0)
	{
		_log(Log::L_INFO, CLOGFMT(CommonTcpCommuncator, "init() sock[%d], allready data, goto onRecvAsync data size[%d]"), _msgChain._defSock, msgHold->_ioSize);
		msgHold->_msgType = RECV_SUCCEED;
		if(!this->onRecvAsync(msgHold) )
		{
			_log(Log::L_WARNING, CLOGFMT(CommonTcpCommuncator, "init() sock[%d], goto onRecvAsync error, data size[%d]"), _msgChain._defSock, msgHold->_ioSize);
			return false;
		}
	}
	else if(!_asyncServer->doRecvAsync(msgHold))
	{
		_log(Log::L_WARNING, CLOGFMT(CommonTcpCommuncator, "init() sock[%d], goto doRecvAsync error"), _msgChain._defSock);
		return false;
		//log
	}
#endif

	return true;
}

int  CommonTcpCommuncator::unInit()
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CommonTcpCommuncator, "unInit, sock[%d], isManualShutdown[YES]"), _msgChain._defSock);
	_isManualShutdown = true;
#ifdef ZQ_OS_MSWIN
	::shutdown(_msgChain._defSock, SD_BOTH);
	::closesocket(_msgChain._defSock);
#else//  ZQ_OS_LINUX
	::shutdown(_msgChain._defSock, SHUT_RDWR);
	::close(_msgChain._defSock);
#endif

	_msgChain._defSock = 0;

	return true;
}

CommonTcpCommuncator::~CommonTcpCommuncator()
{
	ZQ::common::MutexGuard guard(_msgChain._msgChainMutex);
	
	(_log)(ZQ::common::Log::L_INFO, CLOGFMT(CommonTcpCommuncator, "~CommonTcpCommuncator() entry, sock[%d]"), _msgChain._defSock);
	_mgr     = NULL;
	_asyncServer = NULL;

	if (NULL != _msgChain._defARecvTrigger)
	{
		delete _msgChain._defARecvTrigger;
		_msgChain._defARecvTrigger = NULL;
	}
	
	if (NULL != _msgChain._defASendTrigger)
	{
		delete _msgChain._defASendTrigger;
		_msgChain._defASendTrigger = NULL;
	}
	
	std::list<Message*>::iterator it =  _msgChain._msgList.begin();
	for (; it != _msgChain._msgList.end(); ++it)
	{
		Message* msgTemp = (*it);
		IAsyncClientHandler* handler = msgTemp->_clientHandler;
		if(!_isManualShutdown && handler)
		{
			handler->onCloseEvent(msgTemp->_buf, 0, this);
			msgTemp->_clientHandler = NULL;
		}
#ifdef ZQ_OS_MSWIN
		::shutdown(msgTemp->_sock, SD_BOTH);
		::closesocket(msgTemp->_sock);
#else//  ZQ_OS_LINUX
		::shutdown(msgTemp->_sock, SHUT_RDWR);
		::close(msgTemp->_sock);
#endif
		delete (*it);
	}

	_msgChain._msgList.clear();
	_msgChain._defSock = 0;
	_msgChain._defARecvTrigger = 0;
	_msgChain._defASendTrigger = 0;
}

int CommonTcpCommuncator::onAsyncIn(Message* inMsg)
{
	int nRev = this->onRecvAsync(inMsg);
	if (true == _isManualShutdown)
	{
		ICommuncator::Ptr selfConn = inMsg->_selfConn;
		inMsg->_msgType = CLOSE_PENDING;
		_mgr->remove(inMsg->_selfConn);
		inMsg->_selfConn = NULL;
	}

	return nRev;

}

int CommonTcpCommuncator::onAsyncOut(Message* outMsg)
{
	int nRev = this->onSendAsync(outMsg);
	if (true == _isManualShutdown)
	{
		ICommuncator::Ptr selfConn = outMsg->_selfConn;
		outMsg->_msgType = CLOSE_PENDING;
		_mgr->remove(outMsg->_selfConn);
		outMsg->_selfConn = NULL;
	}

	return nRev;
}

int    CommonTcpCommuncator::onRecvSync (char* buffer, size_t bufSize)
{
	(_log)(ZQ::common::Log::L_DEBUG, CLOGFMT(CommonTcpCommuncator, "onRecvSync() entry, but not implement yet"));
	return false;
}

int    CommonTcpCommuncator::onRecvAsync(Message* recvMsg)
{
	int nRev = false;
	(_log)(ZQ::common::Log::L_DEBUG, CLOGFMT(CommonTcpCommuncator, "onRecvAsync() entry") );
	if (RECV_SUCCEED == recvMsg->_msgType)
	{
		ZQ::common::MutexGuard guard(_msgChain._msgChainMutex);
		IAsyncClientHandler* handler = NULL;
		if (NULL != _msgChain._defARecvTrigger && recvMsg == _msgChain._defARecvTrigger)
		{//trigger recv event
			handler = recvMsg->_clientHandler;
			if (handler)
				nRev = handler->onRecvEvent(recvMsg->_buf, recvMsg->_ioSize, this);
		}else{
			if (NULL == _msgChain._defARecvTrigger)
				return false;

			std::list<Message*>::iterator it =  std::find(_msgChain._msgList.begin(), _msgChain._msgList.end(), recvMsg);
			if (it == _msgChain._msgList.end())
				return false;//not exist, may cause memory leak

			it      = _msgChain._msgList.end();
			handler = recvMsg->_clientHandler;
			if (handler)
				nRev = handler->onRecvEvent(recvMsg->_buf, recvMsg->_ioSize, this);

			if (!_isManualShutdown)
			{//re post recv
				recvMsg->resetBuffer();
				recvMsg->_msgType = RECV_PENDING;
				nRev = _asyncServer->doRecvAsync(recvMsg);
			}
		}
	}

	return (ERROR_CODE_OPERATION_OK == nRev || ERROR_CODE_OPERATION_PENDING == nRev);
}

int    CommonTcpCommuncator::onSendSync (const char* buffer, size_t bufSize)
{
	(_log)(ZQ::common::Log::L_DEBUG, CLOGFMT(CommonTcpCommuncator, "onSendSync() entry, but not implement yet"));
	return true;
}

int    CommonTcpCommuncator::onSendAsync(Message* sendMsg)
{
	int nRev = false;
	(_log)(ZQ::common::Log::L_DEBUG, CLOGFMT(CommonTcpCommuncator, "onSendAsync() entry"));
	ZQ::common::MutexGuard guard(_msgChain._msgChainMutex);
	if (SEND_SUCCEED == sendMsg->_msgType)
	{
		IAsyncClientHandler* handler = NULL;
		if (sendMsg == _msgChain._defASendTrigger)
		{//trigger send event
			if (NULL != _msgChain._defASendTrigger && _msgChain._defASendTrigger->_clientHandler)
			{
				handler = _msgChain._defASendTrigger->_clientHandler;
				nRev    = handler->onSendEvent(sendMsg->_buf, sendMsg->_ioSize, this);
			}
		}else{
			
			if (NULL == _msgChain._defASendTrigger)
				return false;

			std::list<Message*>::iterator it =  std::find(_msgChain._msgList.begin(), _msgChain._msgList.end(), sendMsg);
			if (it == _msgChain._msgList.end())
				return false;//not exist, may cause memory leak

			handler = sendMsg->_clientHandler;
			if (handler)
				nRev = handler->onSendEvent(sendMsg->_buf, sendMsg->_ioSize, this);
		}
	}

	return nRev;
}

int    CommonTcpCommuncator::onClose (Message* closeMsg)
{
	ICommuncator::Ptr conn = closeMsg->_selfConn;
	ZQ::common::MutexGuard guard(_msgChain._msgChainMutex);
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CommonTcpCommuncator, "onClose, sock[%d]"), _msgChain._defSock);
	if (CLOSE_PENDING == closeMsg->_msgType && NULL != closeMsg->_clientHandler)
	{
		IAsyncClientHandler* handler = NULL;		
		if (NULL  == _msgChain._defARecvTrigger || NULL == _msgChain._defASendTrigger)
			return false;

		std::list<Message*>::iterator it =  std::find(_msgChain._msgList.begin(), _msgChain._msgList.end(), closeMsg);
		if (it == _msgChain._msgList.end())
			return false;//not exist, may cause memory leak
		
		int   closeSock = closeMsg->_sock;
		
		handler = closeMsg->_clientHandler;
		_msgChain._msgList.erase(it);
        closeMsg->_sock = NULL;
		closeMsg->_clientHandler = NULL;
		if (handler)
			handler->onCloseEvent(closeMsg->_buf, closeMsg->_ioSize, this);

		if (!closeSock)
		{		
#ifdef ZQ_OS_MSWIN
			::shutdown(closeSock, SD_BOTH);
			::closesocket(closeSock);
#else//  ZQ_OS_LINUX
			::shutdown(closeSock, SHUT_RDWR);
			::close(closeSock);
#endif
		}

		_mgr->remove(closeMsg->_selfConn);
		delete closeMsg;

		return true;
	}

	return false;
}
int    CommonTcpCommuncator::onError(Message* errMsg)
{
	_log(ZQ::common::Log::L_DEBUG, CLOGFMT(CommonTcpCommuncator, "onError, sock[%d]"), _msgChain._defSock);
	ICommuncator::Ptr conn = errMsg->_selfConn;
	ZQ::common::MutexGuard guard(_msgChain._msgChainMutex);
	if (ERROR_PENDING == errMsg->_msgType)
	{
		IAsyncClientHandler* handler = NULL;
		if (NULL  == _msgChain._defARecvTrigger || NULL == _msgChain._defASendTrigger)
			return false;

		std::list<Message*>::iterator it =  std::find(_msgChain._msgList.begin(), _msgChain._msgList.end(), errMsg);
		if (it == _msgChain._msgList.end())
			return false;//not exist, may cause memory leak

		int   closeSock = errMsg->_sock;

		_msgChain._msgList.erase(it);
		handler                = errMsg->_clientHandler;
		errMsg->_sock          = NULL;
		errMsg->_clientHandler = NULL;
		if (handler)
			handler->onErrorEvent(errMsg->_buf, errMsg->_ioSize, this);

		if (!closeSock)
		{
#ifdef ZQ_OS_MSWIN
			::shutdown(errMsg->_sock, SD_BOTH);
			::closesocket(errMsg->_sock);
#else// ZQ_OS_LINUX
			::shutdown(errMsg->_sock, SHUT_RDWR);
			::close(errMsg->_sock);
#endif
		}

		_mgr->remove(errMsg->_selfConn);
		delete errMsg;

		return true;
	}

	return false;
}

//data out
int		CommonTcpCommuncator::doRecvSync (char* buffer, unsigned int bufSize, int32 timeout/* = -1*/)
{
	(_log)(ZQ::common::Log::L_ERROR, CLOGFMT(CommonTcpCommuncator, "init() failure"));

	int sock = _msgChain._defSock;
    return _asyncServer->doRecvSync(sock, buffer, bufSize, timeout);
}

int		CommonTcpCommuncator::doRecvAsync(Message* recvMsg)
{
	Message* recvMsgTrigger = NULL;
	if (recvMsg)
	{//next for supporting message buffer
		recvMsgTrigger = recvMsg;
	}else{//using default send trigger
		recvMsgTrigger           = _msgChain._defARecvTrigger;
		recvMsgTrigger->_msgType = RECV_PENDING;
		recvMsgTrigger->_sock    = _msgChain._defSock;
	}

	return _asyncServer->doRecvAsync(recvMsgTrigger);
}

int		CommonTcpCommuncator::doSendSync (const char* buffer, unsigned int bufSize, int32 timeout/* = -1*/)
{
	int sock = _msgChain._defSock;
	return _asyncServer->doSendSync(sock, buffer, bufSize, timeout);
}

int		CommonTcpCommuncator::doSendAsync(Message* sendMsg)
{
	Message* sendMsgTrigger  = sendMsg;
	if (NULL != sendMsgTrigger)
	{//next for supporting message buffer

	}else{//using default send trigger
		sendMsgTrigger           = _msgChain._defASendTrigger;
		sendMsgTrigger->_msgType = SEND_PENDING;
		sendMsgTrigger->_sock    = _msgChain._defSock;
	}

	return _asyncServer->doSendAsync(sendMsgTrigger);
}

ConnectCommuncator::ConnectCommuncator(IAsyncServer* asyncServer, IMgr* mgr, Message* msgHold)
    :CommonTcpCommuncator(asyncServer, mgr, msgHold), _connHold(msgHold), _mgr(mgr), _log(*(mgr->_log))
{}

ConnectCommuncator::~ConnectCommuncator()
{
	using namespace ZQ::common;
	_log(Log::L_INFO, CLOGFMT(ConnectCommuncator, "~ConnectCommuncator() dst[%s],port[%d], sock[%d]"), _sockDst.c_str(), _port, _msgChain._defSock);

	_connHold = NULL;
}

int ConnectCommuncator::init(const char* sockDst, unsigned int port)
{
	int nRev    = connectTo(sockDst, port);
	int initRev = CommonTcpCommuncator::init(sockDst, port);

	return nRev && initRev;
}

int ConnectCommuncator::connectTo(const char* sockDst, unsigned int port)
{
	using namespace ZQ::common;
	
	if (NULL == sockDst || 65535 < port)
	{
		_log(Log::L_WARNING, CLOGFMT(ConnectCommuncator, "connectTo() error dst[NULL] or port[%d]"), port);
		return false;
	}

	_port    = port;
	_sockDst = sockDst;
	int connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (0 > connectSocket) 
	{
		_log(Log::L_WARNING, CLOGFMT(ConnectCommuncator, "connectTo() create socket error dst[%s], port[%d]"), sockDst, port);
		return false;
	}

	sockaddr_in clientService;
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(sockDst);
	clientService.sin_port = htons(port);

	int result = connect(connectSocket, (struct sockaddr *) &clientService, sizeof (clientService));
	if(0 > result) 
	{
		_log(Log::L_WARNING, CLOGFMT(ConnectCommuncator, "connectTo() connect error dst[%s], port[%d], sock[%d]"), sockDst, port, connectSocket);
		// log ;wprintf(L"connect function failed with error: %ld\n", WSAGetLastError());
#ifdef ZQ_OS_MSWIN
		::shutdown(connectSocket, SD_BOTH);
		::closesocket(connectSocket);
#else// ZQ_OS_LINUX
		::shutdown(connectSocket, SHUT_RDWR);
		::close(connectSocket);
#endif
		return false;
	}

	_msgChain._defSock = connectSocket;
	_connHold->_sock   = connectSocket;
	_log(Log::L_INFO, CLOGFMT(ConnectCommuncator, "succeed connectTo() dst[%s], port[%d], sock[%d]"), sockDst, port, connectSocket);

	return true;
}