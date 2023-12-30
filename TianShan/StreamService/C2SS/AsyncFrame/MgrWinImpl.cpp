#include "MgrWinImpl.hpp"
#include "AsyncServerWinImpl.hpp"
#include "IAsyncClientHandler.hpp"

#include <algorithm>

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Mswsock.lib")

MgrWinImpl::MgrWinImpl(unsigned int hashSize)
  :IMgr(hashSize)
{
#if defined(DEBUG)
	_clientCount = 0;
#endif//DEBUG

    _maxWorker = 0;
	_asyncServer = NULL;
	
	for (int workNum = 0; workNum < _maxWorker * 3; ++workNum)
	{
		MgrWinWorker* work = new MgrWinWorker(*this, *_asyncServer);
		work->start();
		_winWorks.push_back(work);
	}
}

MgrWinImpl::MgrWinImpl(ZQ::common::Log* log, unsigned int hashSize)
:IMgr(log, hashSize)
{
#if defined(DEBUG)
	_clientCount = 0;
#endif//DEBUG

	_maxWorker = 0;
	_asyncServer = NULL;

	for (int workNum = 0; workNum < _maxWorker * 3; ++workNum)
	{
		MgrWinWorker* work = new MgrWinWorker(*this, *_asyncServer);
		work->start();
		_winWorks.push_back(work);
	}
}

MgrWinImpl::~MgrWinImpl()
{

}

void  MgrWinImpl::onConnectTo()
{
    return ;
}

void  MgrWinImpl::onCreate()
{
	return ;
}

void  MgrWinImpl::onClose(Message* closeMsg, ICommuncator* closeConn)
{
#ifdef DEBUG
	InterlockedDecrement(&_clientCount);
	if (!(_clientCount % 500))
	{
		std::cout<<"inMemoryC="<<_clientCount<<endl;
	}
#endif

	return ;
}

bool  MgrWinImpl::onRecvSync(int8* buffer, size_t bufSize) 
{
	return true;
}

bool  MgrWinImpl::onRecvAsync(Message* recvMsg) 
{
#ifdef DEBUG
	//std::cout<<recvMsg->_buf<<endl;
#endif
	return true;
}

bool  MgrWinImpl::onSendSync ( const int8* buffer, size_t bufSize)
{
	return true;
}

bool  MgrWinImpl::onSendAsync(Message* sendMsg)
{
	return true;
}

void  MgrWinImpl::onAccept(Message* acceptMsg)
{
#ifdef DEBUG
	InterlockedIncrement(&_clientCount);
	if (!(_clientCount % 500))
	{
		std::cout<<"onAccept="<<_clientCount<<endl;
	}
#endif//DEBUG

	return;
}

void  MgrWinImpl::onError(Message* errMsg, ICommuncator* errConn)
{
#ifdef DEBUG
    InterlockedDecrement(&_clientCount);
	if (!(_clientCount % 500))
	{
		std::cout<<"inMemoryE ="<<_clientCount<<endl;
	}
#endif
	return ;
}

MgrWinWorker::MgrWinWorker(MgrWinImpl& mgrWin, IAsyncServer& asyncServerWin)
:_mgrWin(mgrWin), _asyncServerWin(asyncServerWin), _quit(false)
{

}

MgrWinWorker::~MgrWinWorker()
{

}

int32   MgrWinWorker::run()
{
	while (false)
	{}

	return true;
}

bool    MgrWinWorker::start(void)
{
	ZQ::common::NativeThread::start();
	return true;
}

void    MgrWinWorker::stop(void)
{
	_quit = true;
}