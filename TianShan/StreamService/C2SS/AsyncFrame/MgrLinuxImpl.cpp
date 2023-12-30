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

#include "AsyncServerLinuxImpl.hpp"
#include "MgrLinuxImpl.hpp"

MgrLinuxImpl::MgrLinuxImpl(unsigned int hashSize)
:IMgr(hashSize)
{
#if defined(DEBUG)
	 _clientCount = 0;
#endif
	_maxWorker = 0;
	_asyncServer = NULL;
	for (int workNum = 0; workNum < _maxWorker; ++workNum)
	{
		MgrLinuxWorker* work = new MgrLinuxWorker(*this, *_asyncServer);
		work->start();
		_linuxWorks.push_back(work);
	}
}

MgrLinuxImpl::MgrLinuxImpl(ZQ::common::Log* log, unsigned int hashSize)
:IMgr(log, hashSize)
{
#if defined(DEBUG)
	_clientCount = 0;
#endif
	_maxWorker = 0;
	_asyncServer = NULL;
	for (int workNum = 0; workNum < _maxWorker; ++workNum)
	{
		MgrLinuxWorker* work = new MgrLinuxWorker(*this, *_asyncServer);
		work->start();
		_linuxWorks.push_back(work);
	}
}

MgrLinuxImpl::~MgrLinuxImpl()
{

}

void  MgrLinuxImpl::onConnect()
{
	return ;
}

void  MgrLinuxImpl::onCreate()
{
	return ;
}

void  MgrLinuxImpl::onClose(Message* closeMsg, ICommuncator* closeConn)
{
#ifdef DEBUG
	++_clientCount;
	if (!(_clientCount % 50))
	{
		std::cout<<"inMemoryC="<<_clientCount<<endl;
	}
#endif

	return ;
}

bool  MgrLinuxImpl::onRecvSync(int8* buffer, size_t bufSize) 
{
	return true;
}

bool  MgrLinuxImpl::onRecvAsync(Message* recvMsg) 
{
#ifdef DEBUG
	//std::cout<<recvMsg->_buf<<endl;
#endif
	return true;
}

bool  MgrLinuxImpl::onSendSync ( const int8* buffer, size_t bufSize)
{
	return true;
}

bool  MgrLinuxImpl::onSendAsync(Message* sendMsg)
{
	return true;
}

void  MgrLinuxImpl::onAccept(Message* acceptMsg)
{
#ifdef DEBUG
	++_clientCount;
	if (!(_clientCount % 200))
	{
		std::cout<<"onAccept="<<_clientCount<<endl;
	}
#endif//DEBUG

	return;
}

void  MgrLinuxImpl::onError(Message* errMsg, ICommuncator* errConn)
{
#ifdef DEBUG
	++_clientCount;
	if (!(_clientCount % 50))
	{
		std::cout<<"inMemoryE ="<<_clientCount<<endl;
	}
#endif

	return ;
}

int   MgrLinuxImpl::onStartUp()
{
	return true;
}


MgrLinuxWorker::MgrLinuxWorker(MgrLinuxImpl& mgrLinux, IAsyncServer& asyncServerLinux)
:_mgrLinux(mgrLinux), _asyncServerLinux(asyncServerLinux), _quit(false)
{
}

MgrLinuxWorker::~MgrLinuxWorker()
{

}

int32   MgrLinuxWorker::run()
{
	while (false)
	{}

	return true;
}

bool    MgrLinuxWorker::start(void)
{
	ZQ::common::NativeThread::start();
	return true;
}

void    MgrLinuxWorker::stop(void)
{
	_quit = true;
}

#endif//ZQ_OS_LINUX