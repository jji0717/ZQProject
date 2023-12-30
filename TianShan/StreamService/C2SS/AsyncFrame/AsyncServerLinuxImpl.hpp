#ifndef __ASYNC_SERVER_LINUX_IMPLE_HPP__
#define __ASYNC_SERVER_LINUX_IMPLE_HPP__

#ifdef ZQ_OS_LINUX

#include "IAsyncServer.hpp"
#include "common_async.hpp"
#include "NativeThread.h"
#include "IMgr.hpp"
#include <vector>

class AsyncServerLinuxWorker;

struct IoEventWrapped 
{
	int _eventFd;
};

class AsyncServerLinuxImpl : public IAsyncServer, public IoEventWrapped
{
public:
	explicit AsyncServerLinuxImpl(IMgr& mgr, uint32 maxWorkers);
	virtual  ~AsyncServerLinuxImpl();

	virtual  void    start(void);
	virtual  void    stop(void);

	virtual	 int32	 doRecvSync (int sock, char* buffer, unsigned int bufSize, int32 timeout = -1);	
	virtual	 int32	 doRecvAsync(Message* recvMsg);

	virtual	 int32	 doSendSync (int sock, const char* buffer, unsigned int bufSize, int32 timeout = -1);
	virtual	 int32	 doSendAsync(Message* sendMsg);

	virtual	 int32	 addServent   (int socketForInject, ICommuncator *key, int event = EPOLLIN);
	virtual	 int32	 activeServent(int socketForInject, ICommuncator *key, int event = EPOLLIN);
	virtual	 int32	 removeServent(int socketForRemove, ICommuncator* key);

	virtual  void    getLocalAddress(std::string& localIP, std::string& localPort)const;

	virtual  void    getRemoteAddress(std::string& remoteIP, std::string& remotePort) const;

	virtual	 uint32  getIdleTime();

private:
	friend  class  AsyncServerLinuxWorker;

	IMgr&      _mgr;
	uint32     _maxWorkers;
	std::vector<AsyncServerLinuxWorker* > _linuxWorkers;
};

class AsyncServerLinuxWorker : public ZQ::common::NativeThread
{
public:
	AsyncServerLinuxWorker(AsyncServerLinuxImpl& asyncServerLinux);
	virtual ~AsyncServerLinuxWorker();

	virtual  int32   run();
	virtual	 bool    start(void);
	virtual  void    stop(void) ;

private:
	uint32      _quit;
	AsyncServerLinuxImpl&  _asyncServerLinux; 
};

#endif//ZQ_OS_LINUX

#endif//__ASYNC_SERVER_LINUX_IMPLE_HPP__