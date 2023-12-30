#include "AsyncServerLinuxImpl.hpp"

#ifdef ZQ_OS_LINUX

#include <poll.h>
#include <arpa/inet.h>
#include <sys/socket.h>  
#include <sys/epoll.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <fcntl.h>  
#include <unistd.h>  
#include <stdio.h>  
#include <errno.h>  
#include <iostream>  

#define MAX_IO_SIZE     (5000)
#define MAX_EPOLL_WAIT  (100)

AsyncServerLinuxImpl::AsyncServerLinuxImpl(IMgr& mgr, uint32 maxWorkers)
  :_maxWorkers(maxWorkers), _mgr(mgr)
{
	unsigned long maxIoSize = MAX_IO_SIZE;
	_eventFd = ::epoll_create(maxIoSize);
	for (uint32 worker = 0 ; worker < _maxWorkers ; ++worker)
	{
		AsyncServerLinuxWorker* workerThr = new AsyncServerLinuxWorker(*this);
		assert(NULL != workerThr);
		_linuxWorkers.push_back(workerThr);
		workerThr->start();
	}
}

AsyncServerLinuxImpl::~AsyncServerLinuxImpl()
{
	::close(_eventFd);
	_eventFd = -1;
}

void    AsyncServerLinuxImpl::start(void)
{
	return ;
}

void    AsyncServerLinuxImpl::stop(void)
{
	return ;
}

int32	 AsyncServerLinuxImpl::doRecvSync(int sock, char* buffer, unsigned int bufSize, int32 timeout/* = -1*/)
{
	int    mSock   = sock;

	if( timeout <= 0 )
	{
		int iRet = ::recv( mSock , reinterpret_cast<char*>(buffer) , static_cast<int>(bufSize) , 0 );
		if( 0 >= iRet)
		{
			//MLOG(ZQ::common::Log::L_ERROR, COMMFMT(read,"failed to invoke recv and errorCode[%s]"), strerror(errno));
			return ERROR_CODE_OPERATION_FAIL;
		}
		return static_cast<int32>(iRet);
	}
	else
	{
		struct pollfd pfd;
		pfd.fd = mSock;
		pfd.events = POLLIN;
		int iRet = poll(&pfd,1,timeout);
		if(iRet == -1)
		{
			//MLOG(ZQ::common::Log::L_ERROR,COMMFMT(read,"poll return error code[%d] string[%s]"),errno,strerror(errno));
			return ERROR_CODE_OPERATION_FAIL;
		}
		else if( iRet == 0)
		{
			return ERROR_CODE_OPERATION_TIMEOUT;
		}
		else
		{
			if((pfd.revents & POLLIN) == POLLIN )
			{
				iRet = ::recv(mSock, buffer, bufSize, 0);
				if(0 >= iRet)
				{
					//MLOG(ZQ::common::Log::L_ERROR,COMMFMT(read,"failed to invoke send and errorCode[%d]"),errno);
					return ERROR_CODE_OPERATION_FAIL;
				}
				else
				{
					return iRet ;
				}
			}
			else
			{
				//MLOG(ZQ::common::Log::L_ERROR,COMMFMT(read,	"poll return successful but the socket not in the set"));
				return ERROR_CODE_OPERATION_FAIL;
			}
		}
	}

	return true;
}

int32	 AsyncServerLinuxImpl::doRecvAsync(Message* recvMsg)
{
	int iRet = recv( recvMsg->_sock , recvMsg->_ioStart + recvMsg->_ioSize, recvMsg->_bufSize - recvMsg->_ioSize , MSG_DONTWAIT);
	if( iRet == -1 )
	{
		if( errno == EAGAIN || errno == EWOULDBLOCK )
		{
			return ERROR_CODE_OPERATION_PENDING;
		}
		else
		{
			//MLOG(ZQ::common::Log::L_INFO,COMMFMT(readAsync,"failed to invoke recv and errorCode[%d] string[%s]"), errno,strerror(errno));
			return ERROR_CODE_OPERATION_FAIL;
		}
	}
	else
	{
		recvMsg->_ioSize  += iRet;
		return iRet;		
	}

	return true;
}

int32	 AsyncServerLinuxImpl::doSendSync(int sock, const char* buffer, unsigned int bufSize, int32 timeout/* = -1*/)
{
	int mSock   = sock;

	if ( timeout <= 0 )
	{
		int32 retVal = bufSize;
		size_t pos = 0;
		//int64 startTime = ZQ::common::now();
		while( bufSize > 0 )
		{
			int iRet = ::send( mSock , buffer + pos , bufSize , 0 );
			if( iRet <= 0 )
			{			
				//MLOG(ZQ::common::Log::L_ERROR,COMMFMT(write,"failed to invoke send and errorCode[%s]"),
				//	strerror(errno) );
				return ERROR_CODE_OPERATION_FAIL;
			}
			else
			{
				bufSize -= iRet;
				pos += iRet;
			}
		}
		return retVal;
	}
	else
	{
		struct pollfd pfd;
		pfd.fd = mSock;
		pfd.events = POLLOUT;
		int iRet = poll(&pfd,1,timeout);
		if(iRet == -1)
		{
			return ERROR_CODE_OPERATION_FAIL;
		}
		else if( iRet == 0)
		{
			return ERROR_CODE_OPERATION_TIMEOUT;
		}
		else
		{
			if((pfd.revents & POLLOUT) == POLLOUT )
			{
				iRet = ::send( mSock, buffer, bufSize, 0);
				if( iRet == -1 )
				{
					return ERROR_CODE_OPERATION_FAIL;
				}
				else
				{
					return iRet ;
				}
			}
			else
			{
				return ERROR_CODE_OPERATION_FAIL;
			}
		}

	}
	return true;
}

int32	 AsyncServerLinuxImpl::doSendAsync(Message* sendMsg)
{
	int dwFlag = MSG_DONTWAIT;
	int iRet   = ::send(sendMsg->_sock, sendMsg->_ioStart, sendMsg->_ioSize, dwFlag);
	if( iRet == -1 )
	{
		if( errno != EAGAIN && errno != EWOULDBLOCK)
		{
			return ERROR_CODE_OPERATION_FAIL;
		}
	}
	else
	{
		sendMsg->_ioSize  -= iRet;
		sendMsg->_ioStart += iRet;
	}
	
	return this->addServent(sendMsg->_sock, (ICommuncator*)sendMsg, EPOLLOUT);//just trigger send event
}

int32	 AsyncServerLinuxImpl::addServent(int socketForInject, ICommuncator *key, int ev)
{
	int eventType = ev;//EPOLLOUT | EPOLLIN;
	struct epoll_event event = {0, {0}};

	//event.data.fd  = socketForInject;
	event.data.ptr = (void*)key;
	event.events   = eventType | EPOLLHUP | EPOLLERR | EPOLLONESHOT; //reg read event once , if need another epoll_wait, re-add

	int iRet = epoll_ctl(_eventFd, EPOLL_CTL_ADD, socketForInject, &event);
	if (iRet == -1)
	{//std::cout << "epoll_ctl error: " << strerror(errno) << std::endl;
		return false;
	}

	return true;
}

int32	 AsyncServerLinuxImpl::activeServent(int socketForInject, ICommuncator *key, int ev)
{
	int eventType = ev;//EPOLLOUT | EPOLLIN;
	struct epoll_event event = {0, {0}};
	
	//event.data.fd  = socketForInject;
	event.data.ptr = (void*)key;
	event.events   = eventType | EPOLLHUP | EPOLLERR | EPOLLONESHOT;//epoll_wait once, if need another epoll_wait, re-add

	int iRet = ::epoll_ctl(_eventFd, EPOLL_CTL_MOD, socketForInject, &event);
	if (iRet == -1)
	{//std::cout << "epoll_ctl error: " << strerror(errno) << std::endl;
		return false;
	}

	return true;
}

int32	 AsyncServerLinuxImpl::removeServent(int socketForRemove, ICommuncator* key)
{
	struct epoll_event epv = {0, {0}};  

	epv.data.fd  = socketForRemove;
	epv.data.ptr = key;
	
	int32 nRev = epoll_ctl(_eventFd, EPOLL_CTL_DEL, socketForRemove, &epv); 

	return nRev;
}

void    AsyncServerLinuxImpl::getLocalAddress(std::string& localIP, std::string& localPort)const
{
	return ;
}

void    AsyncServerLinuxImpl::getRemoteAddress(std::string& remoteIP, std::string& remotePort) const
{
	return ;
}

uint32  AsyncServerLinuxImpl::getIdleTime()
{
	return true;
}

AsyncServerLinuxWorker::AsyncServerLinuxWorker(AsyncServerLinuxImpl& asyncServerLinux)
:_asyncServerLinux(asyncServerLinux)
{
	_quit = false;
}

AsyncServerLinuxWorker::~AsyncServerLinuxWorker()
{
}

int32   AsyncServerLinuxWorker::run()
{
	int fds = 0;
	struct epoll_event events[MAX_EPOLL_WAIT];

	while (!_quit)
	{
		fds = 0;
		memset(events, 0, sizeof(events));
		fds = epoll_wait(_asyncServerLinux._eventFd, events, MAX_EPOLL_WAIT, 1000);
		if(_quit)
		{
			break;
		}
		if(fds == -1)
		{
			if(errno == EINTR)
				continue;

			break;
		}

		for(int step = 0; step < fds; ++step)
		{  
			Message *msg           = (Message*)events[step].data.ptr;
			ICommuncator::Ptr conn = msg->_selfConn;
			if (!conn)
				continue;

			if (events[step].events & EPOLLERR)
			{ // error events
				msg->_msgType = ERROR_PENDING;
				_asyncServerLinux.removeServent(msg->_sock, (ICommuncator*)msg);
				conn->onError(msg);
				continue;
			}		

			int reActiveEvent = EPOLLIN;//default should re-enable EPOLLIN
			if((events[step].events & EPOLLIN))
			{
				if (ACCEPT_PENDING != msg->_msgType) // not listening sock
				{
					int32 recvSize = _asyncServerLinux.doRecvSync(msg->_sock, msg->_ioStart, msg->_bufSize - msg->_ioSize, -1);
					if(ERROR_CODE_OPERATION_FAIL == recvSize && RECV_PENDING == msg->_msgType)
					{
						msg->_msgType = CLOSE_PENDING;
						_asyncServerLinux.removeServent(msg->_sock, (ICommuncator*)msg);
						conn->onClose(msg);
						continue;// next for
					}
					msg->_ioSize  = recvSize;
					msg->_msgType = RECV_SUCCEED;
				}// default EPOLLIN
				conn->onAsyncIn(msg);
			} 

			if((events[step].events & EPOLLOUT)) // write event  
			{
				conn->onAsyncOut(msg);
			}

			_asyncServerLinux.activeServent(msg->_sock, (ICommuncator*) msg, reActiveEvent);
		} 
	}
	
    return true;
}

bool    AsyncServerLinuxWorker::start(void)
{
	ZQ::common::NativeThread::start();
	return true;
}

void    AsyncServerLinuxWorker::stop(void)
{
	_quit = true;
	return;
}
#endif//ZQ_OS_LINUX