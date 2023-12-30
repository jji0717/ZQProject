#ifndef __I_ASYNC_SERVER_HPP__
#define __I_ASYNC_SERVER_HPP__

#ifdef ZQ_OS_MSWIN
#include <WinSock2.h>
#else
#include <sys/socket.h>  
#include <sys/epoll.h>  
#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#endif

#include "common_async.hpp"
#include "ICommunicator.hpp"
#include "IAsyncClientHandler.hpp"


#define IO_BUF_SIZE    (2048)

typedef enum MessageType
{
    ACCEPT_PENDING ,
	ACCEPT_SUCCEED ,
	SEND_PENDING   ,
	SEND_SUCCEED   ,
	RECV_PENDING   ,
	RECV_SUCCEED   ,
	CLOSE_PENDING  ,
	ERROR_PENDING  ,
	COUNT_MSG
} MsgType;

struct Message
{
public:
	enum Mark
	{
		TRIGGER    , 
		COMMON     ,
		MARK_ERR
	};

public:
	Message(IAsyncClientHandler* clientHandle = NULL)
		:_msgType(COUNT_MSG), _sock(0), _bufSize(IO_BUF_SIZE),
		_listenSock(0), _ioSize(0), _ioStart(0), _clientHandler(clientHandle)
	{
		_buf = new (std::nothrow) char[IO_BUF_SIZE];
		if (NULL != _buf)
			memset(_buf, 0, IO_BUF_SIZE);
		else
			_bufSize = 0;

		_ioStart    = _buf;
		_msgSpecies = COMMON;
#ifdef ZQ_OS_MSWIN
		memset(&_overlapped, 0, sizeof(_overlapped));
		_wsaBuffer.buf = _buf;
		_wsaBuffer.len = _bufSize;
#endif
	}

	Message(MsgType msgType, IAsyncClientHandler* clientHandle = NULL)
		:_msgType(msgType), _sock(0), _bufSize(0), _buf(NULL),
		_listenSock(0), _ioSize(0), _ioStart(0), _clientHandler(clientHandle)
	{
		_msgSpecies = TRIGGER;
#ifdef ZQ_OS_MSWIN
		_buf = new char[1024];
		memset(&_overlapped, 0, sizeof(_overlapped));
		_wsaBuffer.buf = NULL;//用于在wsasend的 trigger buffer
		_wsaBuffer.len = 0;
#endif
	}

	~Message()
	{
		_ioStart = 0;
		if (_clientHandler)
		{	//delete _clientHandler;//delete by handler manager onclose
			_clientHandler = NULL;
		}
		if (NULL != _buf)
		{
			delete [] _buf;
			_buf = 0;
		}
#ifdef ZQ_OS_MSWIN
		memset(&_overlapped, 0, sizeof(_overlapped));
		memset(&_wsaBuffer, 0, sizeof(_wsaBuffer));
#endif
	}

	int resetBuffer(void)
	{
		if (_buf)
			memset(_buf, 0, _bufSize);

		_ioStart = _buf;
		_ioSize  = 0;
#ifdef ZQ_OS_MSWIN
		_wsaBuffer.len = _bufSize;
#endif//ZQ_OS_MSWIN

		return true;
	}

#ifdef ZQ_OS_MSWIN
	OVERLAPPED _overlapped;//make sure to be in the first place
	WSABUF     _wsaBuffer;
#endif
	IAsyncClientHandler*      _clientHandler;
	MsgType                   _msgType;
	int           _sock;//it is easy to merge socket.h here
	ICommuncator::Ptr         _selfConn;
	Mark          _msgSpecies;
	int           _listenSock;
	int           _bufSize;
	int           _ioSize;
	char*         _ioStart;
	char*         _buf;
};

class ICommuncator;

class IAsyncServer
{
public:
	virtual     ~IAsyncServer(){};
	virtual		void				start(void) = 0;
	virtual		void				stop(void)  = 0;

	virtual		int32				doRecvSync (int sock, char* buffer, unsigned int bufSize, int32 timeout = -1) = 0;	
	virtual		int32				doRecvAsync(Message* recvMsg) = 0;

	virtual		int32				doSendSync (int sock, const char* buffer, unsigned int bufSize, int32 timeout = -1) = 0;
	virtual		int32				doSendAsync(Message* sendMsg) = 0;

	virtual		int32				addServent (int socket, ICommuncator *key, int event = 1) = 0;
	virtual     int32               activeServent(int socket, ICommuncator *key, int event = 1) = 0;
	virtual		int32				removeServent(int socket, ICommuncator* key) = 0;

	virtual     void                getLocalAddress(std::string& localIP, std::string& localPort)const = 0;

	virtual     void                getRemoteAddress(std::string& remoteIP, std::string& remotePort)const = 0;

	virtual		uint32				getIdleTime() = 0;
};

#endif//__I_ASYNC_SERVER_HPP__