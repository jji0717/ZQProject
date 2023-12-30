#ifndef __ASYNC_SERVER_WIN_IMPL_HPP__
#define __ASYNC_SERVER_WIN_IMPL_HPP__

#include "IAsyncServer.hpp"
#include "common_async.hpp"
#include "NativeThread.h"
#include "IMgr.hpp"
#include <vector>

class AsyncServerWinWorker;

struct IoEventWrapped 
{
	OVERLAPPED  _overLapped;
	HANDLE      _ioCompletionPort;
};

class CLASSINDLL_CLASS_DECL AsyncServerWinImpl : public IAsyncServer, public IoEventWrapped
{
public:
	explicit AsyncServerWinImpl(IMgr& mgr, uint32 maxWorkers);
	virtual  ~AsyncServerWinImpl();

	virtual  void    start(void);
	virtual  void    stop(void);

	virtual	 int32	 doRecvSync (int sock, char* buffer, unsigned int bufSize, int32 timeout = -1);	
	virtual	 int32	 doRecvAsync(Message* recvMsg);

	virtual	 int32	 doSendSync (int sock, const char* buffer, unsigned int bufSize, int32 timeout = -1);
	virtual	 int32	 doSendAsync(Message* sendMsg);

	virtual	 int32	 addServent   (int socketForInject, ICommuncator *key, int event = 1);
	virtual	 int32	 activeServent(int socketForInject, ICommuncator *key, int event = 1);
	virtual	 int32	 removeServent(int socketForRemove, ICommuncator* key);

	virtual  void    getLocalAddress(std::string& localIP, std::string& localPort)const;

	virtual  void    getRemoteAddress(std::string& remoteIP, std::string& remotePort) const;

	virtual	 uint32  getIdleTime();

private:
	friend  class  AsyncServerWinWorker;

	IMgr&      _mgr;
	uint32     _maxWorkers;
	std::vector<AsyncServerWinWorker* > _winWorkers;
};


class AsyncServerWinWorker : public ZQ::common::NativeThread
{
public:
	AsyncServerWinWorker(AsyncServerWinImpl& asyncServerWin);
	virtual  int32   run();
	virtual	 bool    start(void);
	virtual  void    stop(void) ;

private:
	uint32               _quit;
	AsyncServerWinImpl&  _asyncServerWin; 
};

#endif//__ASYNC_SERVER_WIN_IMPL_HPP__